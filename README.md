# ezcompile

- Compile or convert simple `.vcxproj` (Visual C++ project) files without MSBuild.
- Stay within the comforts of Visual Studio without others needing it! Which *also* means...
- Use on other platforms: If the code written for cross-platform, no CMake, Visual Studio, or even ezcompile (!) will be needed.

This is made possible by [unfinished, barebones] emulation of MSBuild, completely from scratch.

#### Usage
```
ezcompile [-script] [-cpp] <vcxproj file> [-sln <sln file>]
````

Flag | Description
-----|------------
-script | Instead of compiling, output an `ezcompile.cpp` file and bash script to clang it
-cpp | Compile using C++ driver (ex: uses `clang++` instead of `clang`)
-sln | Provide the `.sln` file for `.vcxproj` files that require it

#### Build from scratch
1) Own clang
2) Double-click `clang.sh` in `script` folder

#### Build with ezcompile
1) Own clang and ezcompile
1) `ezcompile ezcompile.vcxproj`

`Yeah.. I like clang`

(More default compiler support will be added)
