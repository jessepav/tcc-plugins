cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B build\cmake-build ^
      -DTCCHM_DEBUG=0 -DUSE_PCH=1 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL
cmake --build build\cmake-build
