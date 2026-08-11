"""Python port of lcUILua/actions/removeoperation.lua.

Phase 5 PR-5.7 — proof port + living regression test for the
Operations base + selection() + lc.operation.EntityBuilder + Push/Remove
operation appending.  Ships as a DISABLED-by-default duplicate of the
Lua RemoveOperation under a distinct command name (`PYREMOVE`) so the
two versions coexist without CLI collision.  The Lua version remains
authoritative until phase 6.

Not a CreateOperations subclass — this op has no interactive step;
it just reads the current selection, appends a Remove operation, and
closes.  Mirrors the Lua original's plain-Operations shape.
"""

import lc

from ..operations import Operations

# lcgui — same optional-import pattern as create_operations.py.
try:
    import lcgui as _gui
except ImportError:
    _gui = None


@lc.register_operation
class PyRemoveOperation(Operations):
    """Python-defined remove-selected-entities operation.  Coexists
    with Lua's RemoveOperation via the `PYREMOVE` command name."""

    name         = 'PyRemoveOperation'
    command_line = 'PYREMOVE'
    icon         = 'delete.svg'
    description  = 'Python Remove'

    def _init_default(self):
        """Entry point OperationLoader calls when PYREMOVE is invoked.
        No interactive step machine — read the selection immediately
        and run the removal."""
        super().__init__()

        mw = self._get_main_window()
        if mw is None:
            return
        # Snapshot the current selection.  If nothing selected, do
        # nothing (mirrors the Lua original which also silently no-ops
        # on empty selection).
        self.selection = mw.cadMdiChild().selection()
        self.remove()

    # -----------------------------------------------------------------
    # Same helper as CreateOperations — reach the MainWindow via lcgui.
    # -----------------------------------------------------------------
    def _get_main_window(self):
        if _gui is None:
            return None
        return _gui.currentMainWindow()

    # -----------------------------------------------------------------
    # remove — build the EntityBuilder, append each selected entity,
    # append Push + Remove operation ops, execute, message, close.
    # Mirrors the Lua original line-for-line.
    # -----------------------------------------------------------------
    def remove(self):
        mw = self._get_main_window()
        if mw is None:
            return

        # Phase 5 PR-5.5 fixup — EntityBuilder is constructed via
        # `.new(doc)` (def_static), not a py::init<>.  Confirmed
        # against py_lc_operation.cpp:160-168.
        b = lc.operation.EntityBuilder.new(mw.cadMdiChild().document())

        for entity in self.selection:
            b.appendEntity(entity)

        # Push + Remove operation ops chained onto the builder — same
        # shape as the Lua original.  lc.operation.Push and
        # lc.operation.Remove are bound with `.def_static("new", ...)`
        # ONLY — they have no `py::init<>()`.  PR-5.7 fixup: was
        # `lc.operation.Push()` / `lc.operation.Remove()` — those raise
        # `TypeError: No constructor defined!` on every PYREMOVE
        # invocation.  Same bug-class as PR-5.5's gear-plugin
        # `EntityBuilder(doc)` mistake, which was fixed by switching to
        # `.new(doc)` (py_lc_operation.cpp:82-84 + :92-94 confirm both
        # are `.def_static("new", ...)`).
        b.appendOperation(lc.operation.Push.new())
        b.appendOperation(lc.operation.Remove.new())
        b.execute()

        mw.cliCommand().write(
            '{} items removed'.format(len(self.selection)))

        self.close()

    # -----------------------------------------------------------------
    # close — same double-fire operationFinished ordering as Lua.
    # -----------------------------------------------------------------
    def close(self):
        if self.finished:
            return
        mw = self._get_main_window()
        if mw is not None:
            mw.luaInterface().triggerEvent('operationFinished')
        self.finished = True
