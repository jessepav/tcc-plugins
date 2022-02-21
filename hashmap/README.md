# TCC Hashmap Plugin

This is a plugin for JPSoftware's [Take Command] shell that adds a fast in-memory hashmap
data type.

## Download

You can download prebuilt binaries from these links:

* [`hashmap-x64.zip`]

## Building

1.  Use CMake -- e.g.

    ```
    cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B build\cmake-build ^
          -DTCCHM_DEBUG=0 -DUSE_PCH=1 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL
    cmake --build build\cmake-build
    ```

2.  *Assuming that you have TCC, which you probably do, considering this is a TCC plugin*<br>
    
    From the `hashmap` directory:
    
    ```
    ..\tmake.btm
    ```

## Usage

 
<!-- References -->

[Take Command]: https://jpsoft.com/products/take-command.html
[`hashmap-x64.zip`]: https://app.box.com/shared/static/ly8jqtdopf0p2q9hvw93a2zj9yonowu4.zip

<!-- :wrap=none:noTabs=true:indentSize=2:maxLineLen=92: -->
