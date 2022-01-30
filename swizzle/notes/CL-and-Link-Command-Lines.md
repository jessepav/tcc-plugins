# Compiler and Linker Command Lines

## Release

```
cl.exe
/permissive- /Yu"pch.h" /ifcOutput "build\x64\Release\" /GS- /W3 /wd"5105" /Gy- /Zc:wchar_t /Zi /Gm-
/Ox /sdl- /Fd"build\x64\Release\vc142.pdb" /Zc:inline /fp:precise
/D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt
/WX- /Zc:forScope /std:c11 /Gd /Oi /MD /FC /Fa"build\x64\Release\" /nologo
/Fo"build\x64\Release\" /Fp"build\x64\Release\TCCSwizzle.pch" /diagnostics:column
```

```
link.exe
/OUT:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\out\x64\Release\TCCSwizzle.dll" /MANIFEST
/NXCOMPAT /PDB:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\out\x64\Release\TCCSwizzle.pdb"
/DYNAMICBASE "TakeCmd.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "advapi32.lib"
/IMPLIB:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\out\x64\Release\TCCSwizzle.lib" /DLL
/MACHINE:X64 /OPT:REF /INCREMENTAL:NO
/PGD:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\out\x64\Release\TCCSwizzle.pgd"
/SUBSYSTEM:WINDOWS /MANIFESTUAC:NO /ManifestFile:"build\x64\Release\TCCSwizzle.dll.intermediate.manifest"
/LTCGOUT:"build\x64\Release\TCCSwizzle.iobj" /OPT:ICF /ERRORREPORT:PROMPT
/ILK:"build\x64\Release\TCCSwizzle.ilk" /NOLOGO
/LIBPATH:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\lib" /TLBID:1 
```

## Debug

```
cl.exe
/JMC /permissive- /Yu"pch.h" /ifcOutput "build\x64\Debug\" /GS- /W3 /wd"5105" /Gy- /Zc:wchar_t /Zi /Gm-
/Od /sdl- /Fd"build\x64\Debug\vc142.pdb" /Zc:inline /fp:precise
/D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt
/WX- /Zc:forScope /RTC1 /std:c11 /Gd /MDd /FC /Fa"build\x64\Debug\" /nologo
/Fo"build\x64\Debug\" /Fp"build\x64\Debug\TCCSwizzle.pch" /diagnostics:column 
```

```
link.exe
/OUT:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\out\x64\Debug\TCCSwizzle.dll" /MANIFEST
/NXCOMPAT /PDB:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\out\x64\Debug\TCCSwizzle.pdb"
/DYNAMICBASE "TakeCmd.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "advapi32.lib"
/IMPLIB:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\out\x64\Debug\TCCSwizzle.lib" /DEBUG /DLL
/MACHINE:X64 /INCREMENTAL
/PGD:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\out\x64\Debug\TCCSwizzle.pgd"
/SUBSYSTEM:WINDOWS /MANIFESTUAC:NO /ManifestFile:"build\x64\Debug\TCCSwizzle.dll.intermediate.manifest"
/LTCGOUT:"build\x64\Debug\TCCSwizzle.iobj" /ERRORREPORT:PROMPT /ILK:"build\x64\Debug\TCCSwizzle.ilk"
/NOLOGO /LIBPATH:"D:\Relocations\Code\Projects\tcc-plugins\swizzle\lib" /TLBID:1 
```
