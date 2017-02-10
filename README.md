sandscript-map-extension
========================

A simple extension to allow using map-files from within
SandScript.

It is recommended to do this only on one instance
9e.g. instance 0).


For the map names, a file name is used If the file path is
absolute [starts with a /] it is used as-is.
If the path is just a file name then it is placed
in /usr/local/sandvine/etc/

    integer inst
    set inst = extension.map.instance()

    PolicyGroup expr(inst = 0) all {
        set ign = extension.map.zero("foo.map")
        set ign = extension.map.add("foo.map","This is a line")
        set ign = extension.map.reload("foo.map",-1)
    }

Note: on the extension.map.reload(), the first parameter
is the map to reload, the second is the instance (or -1 for
all instances) to reload on. **NEITHER IS CURRENTLY IMPLEMENTED**
Calling this method will reload all maps on all instances.


