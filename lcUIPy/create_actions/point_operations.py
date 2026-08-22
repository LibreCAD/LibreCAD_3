"""Python port of lcUILua/createActions/pointoperations.lua.

Phase 5 PR-5.7 — proof port + living regression test for the
CreateOperations base (PR-5.1) + lcgui bindings (PR-5.6).  Ships as a
DISABLED-by-default duplicate of the Lua PointOperations under a
distinct command name (`PYPOINT`) so the two versions coexist without
CLI collision.  The Lua version remains authoritative until phase 6.

State machine (matches Lua verbatim):
  * `enterPoint` — the sole step.  On mouseMove, update the builder's
    coordinate to preview the point at the current cursor.  On point
    (click), commit the entity and close.

The `@lc.register_operation` decorator + `command_line = 'PYPOINT'`
class attributes cause OperationLoader (phase 5 PR-5.2) to wire this
into the CLI (as PYPOINT), the toolbar (icon `point.svg`), and the
menu (via `menu_actions.default = "actionPoint"` — but see the Lua
convention note: the .lua version reuses `actionPoint` which the QT
resource has for the real POINT command; we mirror that here so the
menu item appears if the Lua counterpart is disabled).
"""

import lc

from ..create_operations import CreateOperations


@lc.register_operation
class PyPointOperations(CreateOperations):
    """Python-defined point-creation operation.  Coexists with Lua's
    PointOperations via the `PYPOINT` command name."""

    # OperationLoader reads these class attributes to wire the op into
    # the CLI/menu/toolbar (same schema as the Lua-side ops).
    name         = 'PyPointOperations'
    command_line = 'PYPOINT'
    icon         = 'point.svg'
    description  = 'Python Point'
    # Menu wiring — reuse the Lua-side actionPoint slot so this op
    # appears in the same menu location.  Disabled in production via
    # the Lua version taking precedence when both are loaded; both can
    # be enabled during dev by running each via its own CLI name.
    menu_actions = {'default': 'actionPoint'}

    def _init_default(self):
        """Entry point OperationLoader calls when the CLI command runs
        without a specific init-suffix (the default init method).

        PR-5.7 fixup — critical bug: previously this called
        `super().__init__(builder=..., step=...)`, which
        DOUBLE-REGISTERED every event listener.  `MainWindow::runOperation`
        constructs the instance via `instantiate()` FIRST — that already
        runs `CreateOperations.__init__()` (which calls
        `registerEvents()`).  Then `_init_default` fires — the second
        `super().__init__()` call re-runs `registerEvents()` on the
        SAME EventBus.  Result: the first click after PYPOINT fires
        onEvent twice, creating 2 duplicate Points; `close()`'s
        single-match `unregisterEvents()` then leaves ONE stale
        registration permanently in the shared EventBus, corrupting
        every subsequent click app-wide until process restart.

        Fix (mirrors the gear-plugin `_init_default` pattern at
        `lcUIPy/plugins/gear/plugin.py:114-125`): set instance state
        directly.  DO NOT call `super().__init__()` — the base class's
        constructor already ran at `instantiate()` time.
        """
        # The base constructor ran with defaults (builder=None,
        # step=None) — we need to attach the real builder + starting
        # step now.  Instance attrs override the defaults.
        self.builder = lc.builder.PointBuilder()
        self.step = 'enterPoint'

        # Show the CLI prompt and print the "add a new point" hint.
        mw = self._get_main_window()
        if mw is not None:
            mw.cliCommand().commandActive(True)
            mw.cliCommand().write('<b>PYPOINT</b> Add a new point')

    # -----------------------------------------------------------------
    # Step handler — matches the Lua original's `enterPoint` shape:
    #   * mouseMove: update the builder's coordinate to preview.
    #   * point (click): commit the entity via createEntity().
    # -----------------------------------------------------------------
    def enterPoint(self, event_name, data):
        if event_name == 'mouseMove':
            # `data` is a Python dict (materialized by py_lc_event.cpp's
            # toPyLocked from ScriptValue::Map).  `data["position"]`
            # holds the current mouse position as a Coordinate.
            position = data.get('position')
            if position is not None:
                self.builder.setCoordinate(position)
        elif event_name == 'point':
            position = data.get('position')
            if position is not None:
                self.builder.setCoordinate(position)
                self.createEntity()
