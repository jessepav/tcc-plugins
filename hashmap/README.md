# TCC Hashmap Plugin

This is a plugin for JPSoftware's [Take Command] TCC shell that adds a fast in-memory hashmap
data type.

Compared to UnQLite, it

* Is not transactional
* Is not thread-safe
* Doesn't support cursors
* Doesn't support data sets larger than memory
* Only supports textual values

but

* is faster and lighter

  > For storing and persisting entries, it is *much, much* faster: in a basic performance
  > test on my old laptop, storing 100 Unicode string entries and saving them to disk took
  > 41 ms using Hashmap and 6731 ms using UnQLite. (This probably involves the @UNQ commands
  > flushing to disk after every call, but for the user who doesn't need ACID guarantees,
  > the result is painful slowness.)
  >
  > To run the test yourself, `SET testPerf=3 & .\test\basictest.btm`
  
* provides more convenient syntax for inserting and retrieving entries with commas and
  quotation marks in their keys or values.

* allows for multiple in-memory maps

Basically, it's just a typical hashmap, as found in standard programming languages, along
with a method to persist them to disk.

--------------

* [Download](#download)
* [Building](#building)
* [Usage](#usage)
  * [Note on Argument Parsing & Key/Value Delimiters](#note-on-argument-parsing--keyvalue-delimiters)
  * [Variable Functions](#variable-functions)
    * [@hashnew](#hashnew)
    * [@hashfree](#hashfree)
    * [@hashdelim](#hashdelim)
    * [@hashget](#hashget)
    * [@hashput](#hashput)
    * [@hashdel](#hashdel)
    * [@hashclear](#hashclear)
    * [@hashcount](#hashcount)
    * [@ishashhandle](#ishashhandle)
  * [Commands](#commands)
    * [hashentries](#hashentries)
    * [hashfile](#hashfile)
    * [hashfreeall](#hashfreeall)
* [Examples](#examples)
* [Licensing](#licensing)


## Download

You can download prebuilt binaries from these links:

* [`hashmap-x64.zip`]

## Building

You'll need Visual Studio (at least the build tools) to build the plugin. I used VS2019, but
any version that has basic C99 support should work (though you'll need to edit the build
config to remove the `/std:c11` flag).

For build systems you have two options:

1.  Use CMake -- e.g.

    ```
    cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B build\cmake-build ^
          -DTCCHM_DEBUG=0 -DUSE_PCH=1 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL
    cmake --build build\cmake-build
    ```
    
    This will generate `hashmap.dll` in `build\cmake-build`.

2.  *Assuming that you have TCC, which you probably do, considering this is a TCC plugin*<br>
    
    From the `hashmap` directory (where `project.btm` is found):
    
    ```
    ..\tmake.btm
    ```
    
    This will generate `hashmap.dll` in `build\tmake`.

## Usage

Variable Functions:

* [@hashnew](#hashnew)
* [@hashfree](#hashfree)
* [@hashdelim](#hashdelim)
* [@hashget](#hashget)
* [@hashput](#hashput)
* [@hashdel](#hashdel)
* [@hashclear](#hashclear)
* [@hashcount](#hashcount)
* [@ishashhandle](#ishashhandle)

Commands:

* [hashentries](#hashentries)
* [hashfile](#hashfile)
* [hashfreeall](#hashfreeall)

### Note on Argument Parsing & Key/Value Delimiters

Because I wanted keys and values to be able to easily use the widest range of characters,
this plugin does not use TCC's standard parsing functions, and thus the escape character
(`^`) and double quotes have no special meaning in arguments to the variable functions.

Because commas are routinely found in filenames, the `@hashget` and `@hashput` functions
separate the key and value using a delimiter, which is by default `/` but which may be
changed by passing an extra argument to `@hashdelim`. You can choose any delimiter string –
multi-character if desired – that is not found in your keys, and then not worry about escaping
commas or double-quoting strings. (If you've ever tried to use the [`@wild`] function to
test a file string that contains a comma, you'll see why this may be useful.)

### Variable Functions

#### @hashnew

Usage: `%@hashnew[[<capacity>]]`

Creates a new hashmap and returns a handle, which is used in all the other functions and
commands. If `<capacity>` is given, it will be used as the initial capacity of the hashmap,
instead of the default of 16 buckets.

Example:

```
set handle=%@hashnew[]
set res=%@hashdelim[%handle,=]
set res=%@hashput[%handle,foo=bar]
echo %@hashget[%handle,foo]
echo %@hashget[%handle,baz=(default value, since baz isn't found)]
set res=%@hashfree[%handle]
```

#### @hashfree

Usage: `%@hashfree[<handle>]`

Free a hashmap created by `@hashnew`.

#### @hashdelim

Usage: `%@hashdelim[<handle>[,<new delimiter>]]`

Return the delimiter used by the map, or, if `<new delimiter>` is given, sets the delimiter
to the supplied string.

Example:

```
echo %@hashdelim[%handle]
echo %@hashdelim[%handle,==]
```

#### @hashget

Usage: `%@hashget[<handle>,<key>[<delimiter><default_val>]]`

Retrieve a value from a hashmap. Normally, if `<key>` is not found in the map, a blank
string will be returned, but if the `<delimiter>` and `<default_val>` are given, then
`<default_val>` will be returned if the key isn't found. In this way you can test for the
existence of a key by specifying a default value that surely is not the value of the key.

Example: `echo %@hashget[%handle,foo/(default value if not found)]`

#### @hashput

Usage: `%@hashput[<handle>,<key><delimiter><value>]`

Put an entry into the map. If an existing entry for the key exists, its value will be
returned, otherwise an empty string.

Example: `set oldValue=%@hashput[%handle,Little Bobby/Red Floaty]`

#### @hashdel

Usage: `%@hashdel[<handle>,<key>]`

Delete an entry from the map. If an existing entry for the key exists, its value will be
returned, otherwise an empty string.

#### @hashclear

Usage: `%@hashclear[<handle>]`

Delete all entries from the map.

#### @hashcount

Usage: `%@hashcount[<handle>]`

Return the number of entries in the map.

#### @ishashhandle

Usage: `%@isHashHandle[<handle>]`

Return 1 if `<handle>` is a valid hashmap handle, or 0 otherwise.

This is useful if you want to have maps nested inside other maps, for instance:

```
set val=%@hashget[%handle1,%key]
IFF %@isHashHandle[%val]==1 THEN
    set handle2=%val
    :: More processing with the second hashmap
ELSE
    :: %val is a simple value
ENDIFF
```

### Commands

#### hashentries

```
hashentries [/K | /V] <handle>

    /K = only print the keys
    /V = only print the values
```

Print all entries in a map to standard output. By default it will print the key followed by
the delimiter and value, but using `/K` or `/V` only the key or value will be printed (without
any delimiter).

#### hashfile

```
hashfile <handle> < /R | /M | /W > <filename>

   /R = read hash entries from file, discarding any current entries
   /M = read hash entries from file, merging them with any current entries
   /W = write hash entries to file
```

Saves or loads the contents of a map to/from disk. Using this command will be much faster
than iterating through the keys using external TCC code.

#### hashfreeall

```
Usage: hashfreeall [/V]

   /V = verbose; print number of hashmaps freed
```

Frees all outstanding hashmaps. This is useful if you don't want to keep track of all your
allocated hashmaps in a script, to free memory at the end and avoid leaks.

## Examples

See the various `.btm` scripts in the [test] directory.

## Licensing

The MIT License (MIT)

Copyright (c) 2022 Jesse Pavel

This software uses [`hashmap.c`], Copyright (c) 2020 Joshua J Baker. 

---------

```
The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

<!-- References -->

[Take Command]: https://jpsoft.com/products/take-command.html
[`hashmap-x64.zip`]: https://app.box.com/shared/static/ly8jqtdopf0p2q9hvw93a2zj9yonowu4.zip
[`hashmap.c`]: https://github.com/tidwall/hashmap.c
[`@wild`]: https://jpsoft.com/help/f_wild.htm
[test]: https://github.com/jessepav/tcc-plugins/tree/master/hashmap/test

<!-- :wrap=none:noTabs=true:indentSize=2:maxLineLen=92: -->
