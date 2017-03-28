sandscript-map-extension
========================

A simple extension to allow using map-files from within
SandScript.

It is recommended to do this only on one instance
(e.g. instance 0).

For the map names, a file name is used If the file path is
absolute [starts with a /] it is used as-is.
If the path is just a file name then it is placed
in /usr/local/sandvine/etc/

    integer "inst"
    set inst = extension.map.instance()

    PolicyGroup expr(inst = 0) all {
        set ign = extension.map.zero("foo.map")
        set ign = extension.map.add("foo.map","line1")
        set ign = extension.map.add("foo.map","line2")
        set ign = extension.map.add("foo.map","line3")
        set ign = extension.map.remove("foo.map","line2")
        set ign = extension.map.reload("foo.map",-1)
    }

The above code will result in 'foo.map' containing:

    line1
    line3

Note: on the extension.map.reload(), the first parameter
is the map to reload, the second is the instance (or -1 for
all instances) to reload on. **NEITHER IS CURRENTLY IMPLEMENTED**
Calling this method will reload all maps on all instances.

You can enable some tracing with

    extension.map.debug(int level)

0 will disable all. 1-N will enable more tracing (to syslog)
