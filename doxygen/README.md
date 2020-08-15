LibreCAD consists of various modules, which are as follows:

## LCKernel
It is the core module as it has no dependency and many other modules depend on it, it provides the CADEntities, Storage , Operations and similar other things that are required.

## Persistence
This module containes file related operation like reading, writing dxf files, loading patterns for hatch, etc.

## LCViewerNoQt
As the name says it contains viewer(main canvas) implement. And OpenGL painter too.

## LCADLuaScript
It provides the functions from the LCKernel to lua, since lcKernel should not have any dependency.

## LCUI
It contains gui related operations and widgets.

## LuaCMDInterface
Command line tool to render output directly using lua

## Unittest
Unittests

## Doxygen
Files to build this documentation

## Res
Resources , currently only contains hatch patterns.

## Scripts
Some build scripts

## Third_Party
Contains third party libraries
