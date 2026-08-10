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
        # Register on the CURRENT MainWindow's EventBus via the phase-5
        # hooks.  If no MainWindow exists (headless CLI) the hooks
        # silently no-op — Python operations can still be *loaded* in
        # CLI mode; they just won't react to interactive events.
        self.registerEvents()

    # -----------------------------------------------------------------
    # Event lifecycle.
    # -----------------------------------------------------------------
    def registerEvents(self):
        lc.event.register('point',     self)
        lc.event.register('mouseMove', self)
        lc.event.register('number',    self)
        lc.event.register('text',      self)

    def unregisterEvents(self):
        lc.event.deregister('mouseMove', self)
        lc.event.deregister('number',    self)
        lc.event.deregister('point',     self)
        lc.event.deregister('text',      self)

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
    # Entity builder machinery.  Reads the active MainWindow lazily via
    # the module-level `mainWindow` injected by OperationLoader.
    # -----------------------------------------------------------------
    def _get_main_window(self):
        # `mainWindow` lives in the caller's global namespace (injected
        # by ScriptDock / OperationLoader).  Use the module globals to
        # find it — same shape as Lua's `mainWindow` global lookup.
        import sys
        # ScriptDock injects `mainWindow` into the exec namespace, not
        # this module's own namespace.  Walk back to find it.
        frame = sys._getframe(1)
        while frame is not None:
            if 'mainWindow' in frame.f_globals:
                return frame.f_globals['mainWindow']
            if 'mainWindow' in frame.f_locals:
                return frame.f_locals['mainWindow']
            frame = frame.f_back
        return None

    def createEntity(self):
        mw = self._get_main_window()
        if mw is None:
            return
        b = lc.operation.EntityBuilder(mw.cadMdiChild().document())
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
        # Fire `operationFinished` before we tear down our own
        # listeners — matches the Lua double-fire ordering (verified
        # in phase 4's PRESERVE list).
        mw = self._get_main_window()
        if mw is not None:
            mw.luaInterface().triggerEvent('operationFinished')
        self.removeTempEntity()
        if hasattr(self, 'cleanUp'):
            self.cleanUp()
        self.unregisterEvents()
        if mw is not None and hasattr(mw, 'cliCommand'):
            cli = mw.cliCommand()
            if hasattr(cli, 'returnText'):
                cli.returnText(False)
            if hasattr(cli, 'commandActive'):
                cli.commandActive(False)
        self.finished = True

    def manualClose(self):
        """Override in subclasses that need custom teardown; default =
        close()."""
        self.close()
