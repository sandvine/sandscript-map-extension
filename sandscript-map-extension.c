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

// required interface
psl_GetEventManifest GetEventManifest;
psl_GetManifest GetManifest;


static psl_Function map_zero;
static psl_Function map_add;
static psl_Function map_reload;
static psl_Function map_instance;

const psl_DataType map_zero_args [] = {psl_string};
const psl_FunctionDescription map_zero_fn = {
    "extension.map.zero",          // SandScript name
    psl_Flag_Pure,                 // flags. Currently only Pure is available.
    map_zero,                      // function to call
    psl_integer,                   // return type
    NELEM(map_zero_args),          // length of arguments list
    map_zero_args                  // the list of argument types above
};

const psl_DataType map_add_args [] = {psl_string,psl_string};
const psl_FunctionDescription map_add_fn = {
    "extension.map.add",           // SandScript name
    psl_Flag_Pure,                 // flags. Currently only Pure is available.
    map_add,                       // function to call
    psl_integer,                   // return type
    NELEM(map_add_args),           // length of arguments list
    map_add_args                   // the list of argument types above
};

const psl_DataType map_reload_args [] = {psl_string, psl_integer};
const psl_FunctionDescription map_reload_fn = {
    "extension.map.reload",        // SandScript name
    psl_Flag_Pure,                 // flags. Currently only Pure is available.
    map_reload,                    // function to call
    psl_integer,                   // return type
    NELEM(map_reload_args),        // length of arguments list
    map_reload_args                // the list of argument types above
};

const psl_DataType map_instance_args [] = {};
const psl_FunctionDescription map_instance_fn = {
    "extension.map.instance",      // SandScript name
    psl_Flag_Pure,                 // flags. Currently only Pure is available.
    map_instance,                  // function to call
    psl_integer,                   // return type
    NELEM(map_instance_args),      // length of arguments list
    map_instance_args              // the list of argument types above
};

char *
SandScriptStringToCString(const psl_stringRef *arg)
{
    char *p = 0;
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
bool
map_zero(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    char *path;
    ResultLocation->i = 0;

    if (!Arguments[0])
        return false;
    path =_getPath(&Arguments[0]->s);

    if (truncate(path, 0) < 0)
        ResultLocation->i = -1;
    free(path);
    return true;
}

//
// Add a line to the given file
bool
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
        free(path);
        free(val);
        ResultLocation->i = 0;
        fclose(fp);
    }
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
bool
map_reload(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    // We don't want to copy the memory space. Could use vfork or clone
    // vfork does not copy pagetables.

    pid_t pid = vfork();
    int status;
    if (pid == 0)
    {
        char *argv[2];
        argv[0] = "pdbClient";
        argv[1] = "-c";
        argv[2] = "devices/policyMaps/1/config/reloading true";
        execve("/usr/local/sandvine/bin/pdbClient.bin",argv,environ);
    }
    waitpid(pid, &status, 0);
    ResultLocation->i = status;

    return true;
}

//
// This is a bit of a work-around. Get the instance-id by processing
// /proc/<mypid>/cmdline and look for -instance #
// If not present return 0. If present, return the #
bool
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


static const psl_FunctionDescription* functions [] = {
     &map_zero_fn,
     &map_add_fn,
     &map_reload_fn,
     &map_instance_fn
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
