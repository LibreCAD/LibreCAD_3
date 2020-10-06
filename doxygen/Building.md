Building from Source
----------
[TOC]

Required libraries
----------

1) cairo 
2) Pango
3) Qt 5
4) git
5) Google test
6) Eigen 3
7) Lua >= 5.2
8) Curl
9) Boost
10) LibDxfRW (building instructions follow)

LibDxfRW
--------
```
git clone https://github.com/LibreCAD/libdxfrw
cd libdxfrw
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
make
sudo make install
```

LibreCAD
--------
 
```
git clone --recursive https://github.com/LibreCAD/LibreCAD_3.git

git submodule init

git submodule update --recursive --remote

mkdir build
cd build
cmake .. (for a release you would use cmake -DCMAKE_BUILD_TYPE=Release ..)
make -j 4
./lcUI/librecad
```

OSX MacPorts
============
If you want to fool around with macports use these build instructions

```
mkdir build
cd build
/opt/local/bin/cmake -DCMAKE_CXX_COMPILER=/opt/local/bin/g++ -DCMAKE_C_COMPILER=/opt/local/bin/gcc ..
make -j 4
./lcUI/librecad
```

For a release build you would do:
```
/opt/local/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/opt/local/bin/g++ -DCMAKE_C_COMPILER=/opt/local/bin/gcc ..
```


OSX X-Code
==========

If you like X-Code as your editor you can ask cmake to generate the build file for you.
mkdir xcode
cd build
cmake -G Xcode ..

Then open the project in x-code and build.
Under some conditions you might want to set a additional search path if libraries are not found, for example if you compile **libdxfrw** yourself.

![X-Code add headers](http://skitch.rvantwisk.nl/~rvt/blog/LC.xcodeproj-20150115-143036.jpg)


When using ports I had to use:

```
cd /opt/local/include
sudo ln -sf eigen3/Eigen Eigen
sudo ln -sf eigen3/unsupported unsupported
```

any cmake guru care to comment on this oddity? Is there a way to prevent making that symlink?

Ubuntu/Mint
===========

```
apt-get install qttools5-dev qttools5-dev-tools libqt5opengl5-dev liblua5.2-dev git g++ gcc-4.8 libcairo2-dev libpango-1.0-0 libpango1.0-dev libboost-all-dev libqt5svg5 libgtest-dev libeigen3-dev libcurl4-gnutls-dev libgtk-3-dev
```

You need to compile Google Test in /usr/src/gtest/ and move the libraries in /usr/lib/

### Ubuntu 14.xx
GCC version from Ubuntu 14 doesn't support C++14. You need to install GCC 4.9.
http://askubuntu.com/a/456849

Windows
=======

**Note:** This process will install Visual Studio then use the folders generated to install Conan and the dependencies requiring manual installation (libdxfrw and Qt - see https://github.com/conan-io/wishlist/issues/124). Change `C:\Source\Repos` as required.

All command lines are run in a terminal. Navigate to the folder in Explorer, on the address bar enter `cmd`, run command(s) then close the terminal by entering `exit`.

## Visual Studio
Install [Visual Studio](https://visualstudio.microsoft.com/downloads/) Community edition with:
- C++
- CMake for Windows
- Windows SDK

## libdxfrw
Start Visual Studio, Clone or checkout code, Repository: https://github.com/LibreCAD/libdxfrw, Local path: `C:\Source\Repos`, Clone

- Build > Build all
- Build > Install DXFRW

Note the folder where it was installed (eg: `C:\Source\Repos\libdxfrw\out\install\x64-Debug`)

## Clone Librecad_3 repository
**Note: Librecad_3 MUST be cloned recursively to pick up linked repositories.**

Navigate to the Repos folder as used for libdxfrw [Fairly sure git is installed with Visual Studio].: 
```
git clone --recursive https://github.com/LibreCAD/LibreCAD_3.git
```

## Conan
Conan is a package manager providing C++ dependencies compatible with Windows and Visual Studio which simplifies the compilation process.

Download and install [Conan](https://conan.io/downloads.html).

## Dependencies
Navigate to the `LibreCAD_3` repository folder and add the bincrafters repository:
```
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan 
```

Download and install the dependencies. Navigate to the `LibreCAD_3` folder, create a `conan` folder and navigate to it:
```
conan install ..
```

## Qt
Download and install Qt (open source version) from https://www.qt.io/download. You must select at least one version of Qt for the MSVC compiler and the correct architecture.

## Configuration
### Set Computer's Environment
Control Panel > System and Security > System > Advanced system settings  
Advanced > Environment Variables

Set QT Environment
```
set QTDIR=C:\Qt\5.14.1\msvc2017_64
set QT_QPA_PLATFORM_PLUGIN_PATH=%QTDIR%\plugins\platforms\
```

Add to the PATH environment variable folders for Qt bin, libdxfrw and LibreCAD_3 lib:
```
C:\Qt\5.14.1\msvc2017_64\bin
C:\source\repos\libdxfrw\out\build\x64-Debug
C:\source\repos\LibreCAD_3\out\build\x64-Debug\lib
```

### Visual Studio CMake

Open project in Visual Studio and configure CMake settings under Project > CMake settings

Configuration name: ```x64-Debug```

Configuration type: ```RelWithDebInfo``` (Debug won't work as Conan libraries are compiled in release mode.)

CMake command arguments, adapting to your configuration:
```
-DLIBDXFRW_PATH=C:\Source\Repos\libdxfrw\out\install\x64-Debug 
-DCMAKE_PREFIX_PATH=C:\Qt\5.14.1\msvc2017_64\lib\cmake
```

When CMake detects Conan it should set GLEW_ROOT.  
Run in Visual Studio by clicking green play button.  

### OpenGL
Some old graphics cards need OpenGL configuration:

Find `opengl32sw.dll`, copy to `C:\source\repos\LibreCAD_3\out\build\x64-Debug\bin` and set Environment:
```
set QT_OPENGL=desktop
```

### Uninstall
The following is required but be careful not to remove if required for other software on your computer:
1. Librecad_3 repository (`C:\source\repos\LibreCAD_3`)
1. libdxfrw repository and install (`C:\source\repos\libdxfrw`)
1. conan (`C:\Users\User\.conan`)
1. python may have been installed to install conan
1. qt
1. Visual Studio
1. environment variables

# Manual installation
## Dependencies 

### Boost
Download prebuilt Windows binaries for Boost from: https://sourceforge.net/projects/boost/files/boost-binaries/

### Eigen 3
Download Eigen 3 zip archive from https://gitlab.com/libeigen/eigen 
Unzip it on your disk (`C:\local\eigen-3.3.7`)

### Qt
Download and install Qt (open source version) from https://www.qt.io/download
You must select at least one version of Qt for the MSVC compiler and the correct architecture.

Add to the PATH environment variable the Qt `bin` folder (`C:\Qt\5.14.1\msvc2017_64\bin`).

### Lua
Download Windows binaries (dll15) here: https://sourceforge.net/projects/luabinaries/files/5.3.5/Windows%20Libraries/Dynamic/
Unzip it on your disk (`C:\local\lua-5.3.5`)

### GLEW
Download Windows binaries from glew.sourceforge.net
Unzip this archive on your disk (`C:\local\glew-2.1.0`)

The `GLEW_ROOT` environment variable (in Windows) should be set to where the archive was extracted, in this case `C:\local\glew-2.1.0`

## Libdxfrw
Clone libdxfrw repository:
```
git clone https://github.com/LibreCAD/libdxfrw
```
Open it as a folder in Visual Studio:
- Build all
- Install DXFRW

Note the folder where it was installed (ex: `C:\Users\...\libdxfrw\out\install\x64-Debug`)

## LibreCAD
Open project as folder in Visual Studio

In `Project > CMake Settings`, set the following CMake options, after adapting the paths to your environment:

```
-DBoost_ADDITIONAL_VERSIONS=1.71.0 
-DBoost_COMPILER=-vc142 
-DBOOST_LIBRARYDIR=C:\local\boost_1_71_0\lib64-msvc-14.2 
-DEIGEN3_ROOT=C:\local\eigen-3.3.7 
-DLIBDXFRW_PATH=C:\Users\ferag\Workspace\libdxfrw\out\install\x64-Debug 
-DWITH_LIBOPENCAD=OFF
-DGLEW_INCLUDE_DIR=C:\local\glew-2.1.0\include 
-DFREETYPE_LIBRARY=C:\local\freetype-2.10.1\win64\freetype.lib 
-DCMAKE_PREFIX_PATH=C:\Qt\5.13.1\msvc2017_64\lib\cmake 
-DCMAKE_LIBRARY_PATH=C:\Qt\5.13.1\msvc2017_64\lib 
-DWITH_LUACMDINTERFACE=FALSE 
-DWITH_RENDERING_UNITTESTS=OFF 
-DWITH_CAIRO=OFF 
-DOPENGL_INCLUDE_DIR=C:\local\glm-0.9.9.6 
-DWITH_UNITTESTS=OFF
```
`-DBoost` options were added because the latest version of Boost wasn't supported by CMake. They may not be necessary anymore.

Add the folders containing the .dll files in your `PATH` environment variable
