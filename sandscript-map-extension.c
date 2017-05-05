/*
   Copyright 2016 Sandvine Incorporated ULC

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "sandscript-map-extension.h"
#include <sys/wait.h>
#include <fcntl.h>
#include <stdarg.h>

// required interface
psl_GetEventManifest GetEventManifest;
psl_GetManifest GetManifest;

static int _map_debug;

static void
MAP_DEBUG(int level, const char * fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    if (level >= _map_debug)
        vsyslog(LOG_INFO, fmt, args);
}

static psl_Function map_zero;
static psl_Function map_add;
static psl_Function map_remove;
static psl_Function map_reload;
static psl_Function map_instance;
static psl_Function map_debug;

static const psl_DataType map_zero_args [] = {psl_string};
static const psl_FunctionDescription map_zero_fn = {
    "extension.map.zero",          // SandScript name
    0,                             // flags. Currently only Pure is available.
    map_zero,                      // function to call
    psl_integer,                   // return type
    NELEM(map_zero_args),          // length of arguments list
    map_zero_args                  // the list of argument types above
};

static const psl_DataType map_add_args [] = {psl_string,psl_string};
static const psl_FunctionDescription map_add_fn = {
    "extension.map.add",           // SandScript name
    0,                             // flags. Currently only Pure is available.
    map_add,                       // function to call
    psl_integer,                   // return type
    NELEM(map_add_args),           // length of arguments list
    map_add_args                   // the list of argument types above
};

static const psl_DataType map_remove_args [] = {psl_string,psl_string};
static const psl_FunctionDescription map_remove_fn = {
    "extension.map.remove",        // SandScript name
    0,                             // flags. Currently only Pure is available.
    map_remove,                    // function to call
    psl_integer,                   // return type
    NELEM(map_remove_args),        // length of arguments list
    map_remove_args                // the list of argument types above
};

static const psl_DataType map_reload_args [] = {psl_string, psl_integer};
static const psl_FunctionDescription map_reload_fn = {
    "extension.map.reload",        // SandScript name
    0,                             // flags. Currently only Pure is available.
    map_reload,                    // function to call
    psl_integer,                   // return type
    NELEM(map_reload_args),        // length of arguments list
    map_reload_args                // the list of argument types above
};

static const psl_DataType map_instance_args [] = {};
static const psl_FunctionDescription map_instance_fn = {
    "extension.map.instance",      // SandScript name
    psl_Flag_Pure,                 // flags. Currently only Pure is available.
    map_instance,                  // function to call
    psl_integer,                   // return type
    NELEM(map_instance_args),      // length of arguments list
    map_instance_args              // the list of argument types above
};

static const psl_DataType map_debug_args [] = {psl_integer};
static const psl_FunctionDescription map_debug_fn = {
    "extension.map.debug",         // SandScript name
    0,                             // flags. Currently only Pure is available.
    map_debug,                     // function to call
    psl_integer,                   // return type
    NELEM(map_debug_args),         // length of arguments list
    map_debug_args                 // the list of argument types above
};

static char *
SandScriptStringToCString(const psl_stringRef *arg)
{
    char *p = 0;
    if (!arg || !arg->begin)
        return p;
    if (arg->length >= 0)
    {
        p = malloc(arg->length+1);
        if (p)
        {
            memcpy(p, arg->begin, arg->length);
            p[arg->length] = 0;
        }
    }
    return p;
}
static char *
_getPath(const psl_stringRef *arg)
{
    char *path;
    // If abs path, use as-is
    if (arg->begin[0] == '/')
    {
        path = SandScriptStringToCString(arg);
    }
    else
    {
#define PREFIX "/usr/local/sandvine/etc/"
        path = malloc(strlen(PREFIX) + arg->length + 1);
        strcpy(path, PREFIX);
        strncat(path, arg->begin, arg->length);
        path[strlen(PREFIX) + arg->length] = 0;
    }
    return path;
}

//
// Take the given file and zero it
static bool
map_zero(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    char *path;
    ResultLocation->i = 0;

    if (!Arguments[0] || !Arguments[0]->s.begin)
        return false;
    path =_getPath(&Arguments[0]->s);

    if (truncate(path, 0) < 0)
        ResultLocation->i = -1;

    MAP_DEBUG(1, "map: truncate(%d) <%s>", ResultLocation->i, path);

    free(path);
    return true;
}

//
// Add a line to the given file
static bool
map_add(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    char *path;
    char *val;
    FILE *fp;
    ResultLocation->i = -1;

    if (!Arguments[0] || !Arguments[1])
        return false;
    path =_getPath(&Arguments[0]->s);
    val = SandScriptStringToCString(&Arguments[1]->s);

    fp = fopen(path, "a");
    if (fp)
    {
        fprintf(fp, "%s\n", val);
        ResultLocation->i = 0;
        fclose(fp);
    }
    MAP_DEBUG(2, "map: add(%d) <%s> to <%s>", ResultLocation->i, val, path);
    free(path);
    free(val);
    return true;
}

//
// Remove a line from the given file
static bool
map_remove(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    char *path;
    char *val;
    FILE *fp;
    FILE *fp1;
    char *pathn;
    char *line;
    size_t ls = 1024;
    ssize_t rs;
    ResultLocation->i = -1;

    if (!Arguments[0] || !Arguments[1])
        return false;
    path =_getPath(&Arguments[0]->s);
    if (!path)
        return false;
    pathn = malloc(strlen(path) + 5);
    if (pathn)
    {
        sprintf(pathn, "%s.bak", path);
        val = SandScriptStringToCString(&Arguments[1]->s);

        fp = fopen(path, "r");

        if (fp)
        {
            fp1 = fopen(pathn, "w");
            if (fp1)
            {
                line = malloc(ls);
                do
                {
                    rs =  getline(&line, &ls, fp);
                    if (rs > 0)
                    {
                        strtok(line, "\n");
                        // If match, skip
                        if (strcmp(line, val))
                            fprintf(fp1, "%s\n", line);
                    }
                }
                while (rs > 0);
                fclose(fp1);
                ResultLocation->i = rename(pathn, path);

                free(line);
            }
            fclose(fp);
        }
        MAP_DEBUG(2, "map: remove(%d) <%s> from <%s>", ResultLocation->i, val, path);
        free(pathn);
        free(val);
    }
    free(path);
    return true;
}

//
// Reload the given map.
// Takes two parameters:
//  1. The map to reload
//  2. The instance to reload (-1 for all)
//
// NB: *** *** *** These parameters are ignored, it reloads all maps
//
static bool
map_reload(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    // We don't want to copy the memory space. Could use vfork or clone
    // vfork does not copy pagetables.

    pid_t pid = vfork();
    int status;
    if (pid == 0)
    {
        char *argv[4];
        argv[0] = "pdbClient";
        argv[1] = "-c";
        argv[2] = "set devices/policyMaps/1/config/reloading true";
        argv[3] = 0;
        execve("/usr/local/sandvine/bin/pdbClient.bin",argv,environ);
    }
    waitpid(pid, &status, 0);
    ResultLocation->i = status;
    MAP_DEBUG(2, "map: reload (%d)", ResultLocation->i);

    return true;
}

//
// This is a bit of a work-around. Get the instance-id by processing
// /proc/<mypid>/cmdline and look for -instance #
// If not present return 0. If present, return the #
static bool
map_instance(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{

    int fd;
    int n;
    int i;
    char line[256];
    char fname[256];
    pid_t pid = getpid();
    ResultLocation->i = 0;
    sprintf(fname, "/proc/%u/cmdline",pid);
    fd = open(fname, O_RDONLY, 0);
    if (fd > 0)
    {
        n = read(fd, line, sizeof(line)-1);
        if (n > 0)
        {
            char *p;
            for (i = 0; i < n; i++)
            {
                if (line[i] == 0)
                    line[i] = ' ';
            }

            line[n+1] = 0;
#define INSTSTR "-instance "
            p = strstr(line, INSTSTR);
            if (p)
            {
                ResultLocation->i = atoi(p + sizeof(INSTSTR)-1);
            }
        }
        close(fd);
    }
    return true;
}

//
// Set the debug level to the integer passed. 0 to disable
static bool
map_debug(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    ResultLocation->i = 0;

    if (!Arguments[0])
        return false;
    _map_debug = Arguments[0]->i;
    return true;
}

static const psl_FunctionDescription* functions [] = {
     &map_zero_fn,
     &map_add_fn,
     &map_remove_fn,
     &map_reload_fn,
     &map_instance_fn,
     &map_debug_fn
};

static psl_Manifest manifest = {
    .version = 1,
    .numFunctionDescriptions = NELEM(functions),
    .functionDescriptions = functions
};

const psl_Manifest* GetManifest()
{

    syslog(LOG_INFO, "Sandvine SandScript Map Extension %s "
                     "compiled on %s [hash: %s]",
                     SANDSCRIPT_MAP_EXTENSION_VERSION,
		     COMPILE_DATE, SANDSCRIPT_MAP_EXTENSION_GITCOMMIT);

    fprintf(stderr, "Sandvine SandScript Map Extension %s "
                     "compiled on %s [hash: %s]",
                     SANDSCRIPT_MAP_EXTENSION_VERSION,
		     COMPILE_DATE, SANDSCRIPT_MAP_EXTENSION_GITCOMMIT);

    return &manifest;
}

static psl_EventManifest eventManifest =
{
    .version = 1,
    .numDescriptions = 0,
    .numFields = 0,
    .eventDescriptions = 0,
    .eventFields = 0,
};

const psl_EventManifest*
GetEventManifest()
{
    return &eventManifest;
}
