LibreCAD_3
==========

LibreCAD 3 development (GSoC 2014)
This is the new LibreCAD Kernel. With keeping in mind the extensibilty, modularity and the design, the LibreCAD is divided into 3 parts.  
1) The UI  
2) The kernel  
3) The CADViewer  


Compilation Instructions
==========

Install cairo  
Install Qt 5  
Install git  
Install liblog4cxx  
Install Google test  
 
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

MacPorts/brew might not have **libdxfrw** in it so you need to compile it yourself.

```
git clone https://github.com/rvt/libdxfrw
cd libdxfrw
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

OSX X-Code
========

If you like X-Code as your editor you can ask cmake to generate the build file for you.  
mkdir xcode  
cd build  
cmake -G Xcode ..  

Then open the project in x-code and buikd.
Under some conditions you might want to set a additional search path if libraries are not found, for example if you compile **libdxfrw** yourself.

![X-Code add headers]
(http://skitch.rvantwisk.nl/~rvt/blog/LC.xcodeproj-20150115-143036.jpg)


Ubuntu/Mint
========

```
apt-get install qttools5-dev qttools5-dev-tools libqt5opengl5-dev liblua5.2-dev git g++ gcc-4.8 libcairo2-dev liblog4cxx10-dev libpango-1.0-0 libpango1.0-dev libboost-all-dev
```



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

