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

// required interface
psl_GetEventManifest GetEventManifest;
psl_GetManifest GetManifest;


static psl_Function map_zero;
static psl_Function map_add;
static psl_Function map_reload;

const psl_DataType map_zero_args [] = {psl_string};
const psl_FunctionDescription map_zero_fn = {
    "map.extension.zero",          // SandScript name
    psl_Flag_Pure,                 // flags. Currently only Pure is available.
    map_zero,                      // function to call
    psl_integer,                   // return type
    NELEM(map_zero_args),          // length of arguments list
    map_zero_args                  // the list of argument types above
};

const psl_DataType map_add_args [] = {psl_string,psl_string};
const psl_FunctionDescription map_add_fn = {
    "map.extension.add",           // SandScript name
    psl_Flag_Pure,                 // flags. Currently only Pure is available.
    map_add,                       // function to call
    psl_integer,                   // return type
    NELEM(map_add_args),           // length of arguments list
    map_add_args                   // the list of argument types above
};

const psl_DataType map_reload_args [] = {psl_string, psl_integer};
const psl_FunctionDescription map_reload_fn = {
    "map.extension.reload",        // SandScript name
    psl_Flag_Pure,                 // flags. Currently only Pure is available.
    map_reload,                    // function to call
    psl_integer,                   // return type
    NELEM(map_reload_args),        // length of arguments list
    map_reload_args                // the list of argument types above
};

bool
map_zero(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    return false;
}

bool
map_add(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    return false;
}

bool
map_reload(psl_Value *ResultLocation, const psl_Value *const *Arguments)
{
    return false;
}


static const psl_FunctionDescription* functions [] = {
     &map_zero_fn,
     &map_add_fn,
     &map_reload_fn
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
