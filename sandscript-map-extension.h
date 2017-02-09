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

#if !defined(_SANDSCRIPT_MAP_EXTENSION_H)
#define _SANDSCRIPT_MAP_EXTENSION_H
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <stdio.h>

#ifndef SANDSCRIPT_MAP_EXTENSION_VERSION
#define SANDSCRIPT_MAP_EXTENSION_VERSION "UNKNOWN"
#endif

#define NELEM(x) (sizeof(x) / sizeof(x[0]))

#include "sandscript-library-interface/sharedLibManifest.h"
#include "sandscript-library-interface/sharedLibEvents.h"

#endif /* _SANDSCRIPT_MAP_EXTENSION_H */
