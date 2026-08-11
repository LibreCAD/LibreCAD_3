LibreCAD_3
==========

![Build status](https://api.travis-ci.org/LibreCAD/LibreCAD_3.svg?branch=master) [![Coverage Status](https://coveralls.io/repos/github/LibreCAD/LibreCAD_3/badge.svg?branch=master)](https://coveralls.io/github/LibreCAD/LibreCAD_3?branch=master) [![CodeFactor](https://www.codefactor.io/repository/github/librecad/librecad_3/badge)](https://www.codefactor.io/repository/github/librecad/librecad_3)

Compilation Instructions
==========
See https://github.com/LibreCAD/LibreCAD_3/wiki/Building

Releases and Releases and Continuous Building 
----------
See https://github.com/LibreCAD/LibreCAD_3/releases

Scripting
----------
LibreCAD_3 embeds both Lua and Python simultaneously.  User scripts,
plugins, and custom entity classes can be written in either language;
the two share the same underlying event bus, operation registry, and
kernel API.  See [doxygen/Scripting.md](doxygen/Scripting.md) for the
architecture, the parity table, and plugin authoring notes.  Python
scripting is enabled by default at build time (`WITH_PYTHONSCRIPT=ON`);
build with `-DWITH_PYTHONSCRIPT=OFF` for a Lua-only build.


Reading materials for feature usage
=========

If we want to support associative drawings, this is a good introduction to get started:

[_Understanding how dimensions are associated with objects_](https://web.archive.org/web/20170318040619/http://www.ellenfinkelstein.com/acadblog/understanding-how-dimensions-are-associated-with-objects/)


List of apparently awesome frameworks
=========
* https://github.com/fffaraz/awesome-cpp
* https://github.com/retuxx/tinyspline
* https://github.com/ebassi/graphene
* http://discourse.mcneel.com/c/opennurbs

If we are short of sample DXF files to test
=========
http://cadkit.blogspot.nl/p/sample-dxf-files.html

