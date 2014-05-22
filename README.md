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

git clone --recursive https://github.com/LibreCAD/LibreCAD_3/tree/master

qmake -r
make -j 4



Ubuntu/Mint
========

apt-get install qttools5-dev qttools5-dev-tools libqt5opengl5-dev liblua5.2-dev git g++ gcc-4.8 libcairo2-dev liblog4cxx10-dev


