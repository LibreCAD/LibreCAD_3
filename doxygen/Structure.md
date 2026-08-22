Code Structure
---------

LibreCAD consists of various modules, which are as follows:

<div>
<h2> <a href="../../lckernel/html/index.html">lckernel</a> </h2>
It is the core module as it has no dependency and many other modules depend on it, it provides the CADEntities, Storage, Operations and similar other things that are required.
</div>

<div>
<h2> <a href="../../persistence/html/index.html">persistence</a> </h2>
This module contains file related operations like reading, writing dxf files, loading patterns for hatch, etc.
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
<h2> <a href="../../lcadpythonscript/html/index.html">lcadpythonscript</a> </h2>
Phase 5 addition — a sibling of lcadluascript exposing the same LCKernel surface to Python via pybind11.
Both scripting languages live behind a neutral scripting layer (<a href="../../lcscripting/html/index.html">lcscripting</a>),
so Lua and Python share the exact same C++-side event bus, callback dispatch, and operation registry.
See <a href="Scripting.md">Scripting.md</a> for the full architecture, the parity table, and plugin
authoring notes.
</div>

<div>
<h2> lcscripting </h2>
Neutral scripting layer (phase 4).  Contains <code>ScriptCallback</code>, <code>ScriptObject</code>, and
<code>ScriptValue</code> — the language-agnostic types every scripting adapter (Lua, Python, future runtimes)
wraps into.  The EventBus, the operation registry, and custom-entity plugin slots all live here.
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
Resources, currently only contains hatch patterns.
</div>

<div>
## scripts
Some build scripts
</div>

<div>
## third_party
Contains third party libraries
</div>
