LibreCAD_3 ![Build status](https://api.travis-ci.org/LibreCAD/LibreCAD_3.svg?branch=master)
==========

LibreCAD 3 development (GSoC 2014)
This is the new LibreCAD Kernel. With keeping in mind the extensibilty, modularity and the design, the LibreCAD is divided into 3 parts.  
1) The UI  
2) The kernel  
3) The CADViewer  


Compilation Instructions
==========

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
10) LibDxfRW (see compilation instruction after)

LibDxfRW compilation
----------
```
git clone https://github.com/LibreCAD/libdxfrw
cd libdxfrw
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

LibreCAD compilation
----------
 
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
========
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
========

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
========

```
apt-get install qttools5-dev qttools5-dev-tools libqt5opengl5-dev liblua5.2-dev git g++ gcc-4.8 libcairo2-dev libpango-1.0-0 libpango1.0-dev libboost-all-dev libqt5svg5 libgtest-dev libeigen3-dev libcurl4-gnutls-dev libgtk-3-dev
```

You need to compile Google Test in /usr/src/gtest/ and move the libraries in /usr/lib/

### Ubuntu 14.xx
GCC version from Ubuntu 14 doesn't support C++14. You need to install GCC 4.9.
http://askubuntu.com/a/456849

Windows
========

# Visual Studio
Install Visual Studio with:
- C++
- CMake for Windows
- Windows SDK

## Dependencies

### Boost
Download prebuilt Windows binaries for Boost from: https://sourceforge.net/projects/boost/files/boost-binaries/

### Eigen 3
Download Eigen 3 zip archive from https://eigen.tuxfamily.org 
Unzip it on your disk (`C:\local\eigen-3.3.7`)

### Qt
Download and install Qt (open source version) from https://www.qt.io/download
You must select at least one version of Qt for the MSVC compiler

### Lua
Download Windows binaries (dll15) here: https://sourceforge.net/projects/luabinaries/files/5.3.5/Windows%20Libraries/Dynamic/
Unzip it on your disk (`C:\local\lua-5.3.5`)

### GLEW
Download Windows binaries from glew.sourceforge.net
Unzip this archive on your disk (`C:\local\glew-2.1.0`)

## Libdxfrw
Clone libdxfrw repository
Open it as a folder in Visual Studio:
- Build all
- Install DXFRW

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
-DGLEW_LIBRARY=C:\local\glew-2.1.0\lib\Release\x64\glew32.lib 
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

Reading materials for feature usage
=========

If we want to support associative drawings, this is a good introduction to get started
http://www.ellenfinkelstein.com/acadblog/understanding-how-dimensions-are-associated-with-objects/



Some comment's created with
=========

http://patorjk.com/software/taag/#p=display&c=c&f=Three%20Point&t=DimAligned


List of apparently awesome frameworks
=========
https://github.com/fffaraz/awesome-cpp
https://github.com/retuxx/tinyspline
https://github.com/ebassi/graphene
http://discourse.mcneel.com/c/opennurbs

If we are short of sample DXF files to test
=========
http://cadkit.blogspot.nl/p/sample-dxf-files.html

