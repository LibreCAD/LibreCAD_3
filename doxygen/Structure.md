Code Structure
---------

LibreCAD consists of various modules, which are as follows:

<div>
<h2> <a href="../../lckernel/html/index.html">lckernel</a> </h2>
It is the core module as it has no dependency and many other modules depend on it, it provides the CADEntities, Storage , Operations and similar other things that are required.
</div>

<div>
<h2> <a href="../../persistence/html/index.html">persistence</a> </h2>
This module containes file related operation like reading, writing dxf files, loading patterns for hatch, etc.
</div>

<div>
<h2> <a href="../../lcviewernoqt/html/index.html">lcviewernoqt</a> </h2>
As the name says it contains viewer(main canvas) implement. And OpenGL painter too.
</div>

<div>
<h2> <a href="../../lcadluascript/html/index.html">lcadluascript</a> </h2>
It provides the functions from the LCKernel to lua, since lcKernel should not have any dependency.
</div>

<div>
<h2> <a href="../../lcUI/html/index.html">lcUI</a> </h2>
It contains gui related operations and widgets.
</div>

<div>
<h2> <a href="../../luacmdinterface/html/index.html">luacmdinterface</a> </h2>
Command line tool to render output directly using lua
</div>

<div>
<h2> <a href="../../unittest/html/index.html">unittest</a> </h2>
Unittests
</div>


Other folders are as follows:
<div>
## doxygen
Files to build this documentation
</div>

<div>
## res
Resources , currently only contains hatch patterns.
</div>

<div>
## scripts
Some build scripts
</div>

<div>
## third_party
Contains third party libraries
</div>
