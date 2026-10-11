"""Python port of lcUILua/createActions/createOperations.lua.

Phase 5 PR-5.1 — the `CreateOperations` base class for interactive
entity-creation operations (point, line, arc, ...).  Extends
`Operations` with the register/unregister-events lifecycle and the
step-dispatch onEvent handler.

Event API used:
  * `lc.event.register(name, self)`   — attach a listener; the object
    shape means the underlying dispatch fires `self.onEvent(event,
    args)` at trigger time.  The name choices match Lua verbatim
    (point / mouseMove / number / text) so a Python subclass can
    coexist with Lua listeners on the same events.
  * `lc.event.deregister(name, self)` — same shape; PythonCallbackImpl's
    is-or-equal equality (phase 4 design decision, verified by
    unittest/scripting/scriptobject_test.cpp::InstantiateInvokesConstructor
    et al.) means passing `self` again works even though attribute
    access can create fresh py::objects.
"""

import lc

from .operations import Operations

# `lcgui` is the lcUI-side embedded Python module (see
# lcUI/python/pyguibridge.cpp).  It exposes `currentMainWindow()` which
# is the only supported way to reach the owning MainWindow from an
# event-driven callback path: PythonCallbackImpl::invokeEvent fires
# our `onEvent` fresh from C++ (Qt slot → EventBus → invoke), so
# `sys._getframe()` walking finds no Python caller with `mainWindow`
# in scope. C++ enters nested owner scopes around construction, teardown,
# script execution, and EventBus dispatch; outside those scopes this returns
# None rather than guessing from insertion order.
try:
    import lcgui as _gui
except ImportError:
    # Headless CLI mode may not have lcgui loaded (built without
    # WITH_PYTHONSCRIPT for the lcUI library, or a script running
    # via lcpythonscript alone).  CreateOperations is still
    # importable; the mw-dependent methods just no-op.
    _gui = None


class CreateOperations(Operations):
    """Interactive create-op base.

    Subclasses set `builder` (a callable that constructs an
    lc.builder.* subclass instance) and `step` (the starting step
    tag).  The dispatch loop calls `self[step](event, data)` on every
    incoming event so subclasses only need to define step-named
    methods.
    """

    # `mainWindow` is injected into the module namespace by
    # OperationLoader / ScriptDock — we read it lazily via
    # `_get_main_window()` so tests can construct CreateOperations
    # instances without a live UI.
    def __init__(self, builder=None, step=None):
        super().__init__()
        self.prevEntity = None
        self.entity = None
        self.builder = builder() if builder is not None else None
        self.step = step
        self._event_registrations = []
        # Register on the CURRENT MainWindow's EventBus via the phase-5
        # hooks.  If no MainWindow exists (headless CLI) the hooks
        # silently no-op — Python operations can still be *loaded* in
        # CLI mode; they just won't react to interactive events.
        self.registerEvents()

    # -----------------------------------------------------------------
    # Event lifecycle.
    # -----------------------------------------------------------------
    def registerEvents(self):
        for name in ('point', 'mouseMove', 'number', 'text'):
            registration = lc.event.register(name, self)
            if registration is not None:
                self._event_registrations.append((name, registration))

    def unregisterEvents(self):
        # Exact tokens avoid deleting another equal callback if this object
        # registered more than once or a fresh bound method/object compares
        # equal to a sibling registration.
        for name, registration in reversed(self._event_registrations):
            lc.event.deregister(name, registration)
        self._event_registrations.clear()

    # -----------------------------------------------------------------
    # onEvent — the object-shape dispatch entry point.  PythonCallbackImpl
    # routes here for object-with-onEvent registrations.
    # -----------------------------------------------------------------
    def onEvent(self, event_name, data):
        if self.step == 'PAUSE':
            return
        if self.step is None:
            return
        # Step methods are named after the step tag: `self.FIRST_POINT`,
        # `self.SECOND_POINT`, etc.  Same convention as the Lua base.
        step_fn = getattr(self, self.step, None)
        if step_fn is not None:
            step_fn(event_name, data)
        if not self.finished:
            self.refreshTempEntity()

    # -----------------------------------------------------------------
    # Entity builder machinery.  Phase 5 PR-5.1 fixup: reach the
    # MainWindow via lcgui.currentMainWindow() rather than by walking
    # the Python call stack.  The frame walk failed for event-driven
    # dispatch — PythonCallbackImpl::invokeEvent calls onEvent fresh
    # from C++ (Qt slot → EventBus → invoke), so no Python caller
    # frame contains `mainWindow`. currentMainWindow() is supplied by
    # a nested C++ execution scope during operation construction,
    # callbacks, and ScriptDock runs.
    # -----------------------------------------------------------------
    def _get_main_window(self):
        if _gui is None:
            return None
        return _gui.currentMainWindow()

    def createEntity(self):
        mw = self._get_main_window()
        if mw is None:
            return
        # Phase 5 PR-5.5 fixup — `lc.operation.EntityBuilder(doc)` is
        # NOT a bound constructor; py_lc_operation.cpp:160-168 exposes
        # only `def_static("new", ...)`.  Calling it as a ctor raised
        # `TypeError: No constructor defined!` on every real use.
        # Confirmed correct call form via
        # `unittest/python/persistence_test.cpp:58-65`.
        b = lc.operation.EntityBuilder.new(mw.cadMdiChild().document())
        b.appendEntity(self.build())
        b.execute()
        self.manualClose()

    def build(self):
        mw = self._get_main_window()
        if mw is None or self.builder is None:
            return None
        mdi = mw.cadMdiChild()
        self.builder.setLayer(mdi.activeLayer())
        # metaInfoManager / activeViewport are lcgui-side (not bound in
        # the phase-3.2 minimum surface); phase 5 PR-5.6 adds them.
        # Guard so this method is call-safe today.
        if hasattr(mdi, 'metaInfoManager'):
            self.builder.setMetaInfo(mdi.metaInfoManager().metaInfo())
        if hasattr(mdi, 'activeViewport'):
            self.builder.setBlock(mdi.activeViewport())
        return self.builder.build()

    def refreshTempEntity(self):
        mw = self._get_main_window()
        if mw is None:
            return
        tempEntities = mw.cadMdiChild().tempEntities()
        if self.prevEntity is not None:
            tempEntities.removeEntity(self.prevEntity)
        self.entity = self.build()
        if self.entity is not None:
            tempEntities.addEntity(self.entity)
        self.prevEntity = self.entity

    def removeTempEntity(self):
        mw = self._get_main_window()
        if mw is None:
            return
        if self.prevEntity is not None:
            mw.cadMdiChild().tempEntities().removeEntity(self.prevEntity)

    # -----------------------------------------------------------------
    # close / manualClose — the same double-fire order as Lua.
    # -----------------------------------------------------------------
    def close(self):
        if self.finished:
            return
        # Phase 5 PR-5.1 fixup — luaInterface() and cliCommand() are
        # bound on the Python MainWindow class now, so the double-fire
        # `operationFinished` + CLI reset actually runs.  Previously
        # the AttributeError was swallowed by PythonObjectImpl's
        # blanket catch, silently skipping removeTempEntity() /
        # cleanUp() / unregisterEvents() and leaking listeners on
        # every operation completion.
        mw = self._get_main_window()
        # Fire `operationFinished` BEFORE tearing down our own
        # listeners — matches the Lua double-fire ordering (verified
        # in phase 4's PRESERVE list).
        if mw is not None:
            mw.luaInterface().triggerEvent('operationFinished')
        self.removeTempEntity()
        if hasattr(self, 'cleanUp'):
            self.cleanUp()
        self.unregisterEvents()
        if mw is not None:
            cli = mw.cliCommand()
            cli.returnText(False)
            cli.commandActive(False)
        self.finished = True

    def manualClose(self):
        """Override in subclasses that need custom teardown; default =
        close()."""
        self.close()
