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
5) liblog4cxx
6) Google test
7) Eigen 3
8) Lua >= 5.2
9) Curl
10) Boost
11) LibDxfRW (see compilation instruction after)

LibDxfRW compilation
----------
```
git clone https://github.com/rvt/libdxfrw
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
apt-get install qttools5-dev qttools5-dev-tools libqt5opengl5-dev liblua5.2-dev git g++ gcc-4.8 libcairo2-dev liblog4cxx10-dev libpango-1.0-0 libpango1.0-dev libboost-all-dev libqt5svg5 libgtest-dev libeigen3-dev libcurl4-gnutls-dev libgtk-3-dev
```

You need to compile Google Test in /usr/src/gtest/ and move the libraries in /usr/lib/

### Ubuntu 14.xx
GCC version from Ubuntu 14 doesn't support C++14. You need to install GCC 4.9.
http://askubuntu.com/a/456849

Windows
========

### Bash for Windows 10
This is the only way to get LibreCAD running on Windows at the moment.
1) Follow the instructions for Ubuntu 14.04.

2) Install Xming or any other X server for Windows

3) Enter `export DISPLAY=:0` before running LibreCAD

### MSYS2

```
pacman -S mingw-w64-x86_64-cairo mingw-w64-x86_64-pango mingw-w64-x86_64-lua mingw-w64-x86_64-eigen3 mingw-w64-x86_64-apr mingw-w64-x86_64-apr-util mingw-w64-x86_64-qt5 mingw-w64-x86_64-gtest
```

#### Log4CXX:

Use lastest code from repo : git://git.apache.org/log4cxx.git
https://issues.apache.org/jira/browse/LOGCXX-463

Put the source in /third_party/apache-log4cxx

```
./autogen.sh
./configure
make
make install
```

#### LibreCAD:

```
mkdir build
cd build
cmake -DWINDOWS_MSYS2=ON ..
```

Create symlinks for .so files with ```ln -s ../lc*/lib*.so```

Replace paths in build/lcUI/path.lua with Windows-style paths ```C:\msys64\…```

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

