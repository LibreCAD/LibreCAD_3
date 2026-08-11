"""Rectangle plugin — Python port of lcUILua/plugins_disabled/rectangle/plugin.lua.

Phase 6 PR-6.1 sub-piece 3c: modernized port + Python version of the
custom-entity reference plugin.  Serves two purposes:

1. **Interactive create**: `PYRECTANGLE` command lets the user click two
   corners to define a rectangle, which lands in the document as 4 Line
   entities + one `Insert` referring to a `CustomEntityStorage` block
   with plugin name "LC Plugin" and params `width` / `height`.  On DXF
   save, the app-data codes 102/410/411/470/471 carry the plugin
   identification through to the file.

2. **On-reload reconstruction**: `lc.register_plugin("LC Plugin", ...)`
   registers a callback that fires when the document is reopened and
   `NewWaitingCustomEntityEvent` is emitted for any `Insert` whose
   `displayBlock` is a `CustomEntityStorage` with `pluginName == "LC
   Plugin"`.  The callback rebuilds the entity as a `ScriptCustomEntity`
   with the 6 script-defined behavior slots wired to this file's
   pure-function handlers.

Two limitations compared to the pre-refactor Lua original:
  * `CustomEntityBuilder.copy(insert)` — that API doesn't exist on the
    current builder.  This port sets the InsertBuilder-parent fields
    (layer / coord / doc / displayBlock) explicitly on the reconstruct
    path.
  * `lc.operation.Push()` / `lc.operation.Remove()` — Python calls
    `.new()` per PR-5.5's ctor-vs-factory pattern, matching the fixed
    `remove_operation.py` sample.

**Known blocker**: DXF reload of custom entities is currently broken by
an upstream libdxfrw bug (task_e20ba02a — dead loop in
`DRW_Entity::parseDxfGroups` never reads app-data codes 410/411/470/
471).  The plugin *registration* is correct and works today; the
*reconstruction dispatch* only fires post-DXF-fix.  Meanwhile the
interactive `PYRECTANGLE` create path fully works today.
"""

import lc

from lcUIPy.create_operations import CreateOperations


# =============================================================================
# Module-level per-Insert state for interactive drag operations.
#
# The Lua original uses tables keyed by `insert:id()`.  Python mirrors
# that with per-entity dicts.  Entries are added on `dragPointClick` and
# cleared on `dragPointRelease`.  A stale entry doesn't leak — the
# max size is bounded by the number of concurrently-dragged rectangles
# (typically 1).
# =============================================================================

_drag_storage = {}          # insert_id → list of preview line entities
_drag_selected_point = {}   # insert_id → int (0-3, corner ID)
_drag_width = {}            # insert_id → float
_drag_height = {}           # insert_id → float
_drag_base = {}             # insert_id → Coordinate (bottom-left after drag)


# =============================================================================
# Custom-entity behavior slots — pure functions used by both the
# interactive create-op and the on-reload reconstruction plugin.
#
# Every function receives the concrete `Insert` (thanks to pybind11's
# classh polymorphic downcast, verified in sub-piece 3a's
# `CustomEntityBuilderBuildReturnsScriptCustomEntity` and applied at the
# DXF-reload boundary in sub-piece 3b's exit-criterion test).
# =============================================================================

def _snap_points(insert, coord, constrain, min_dist, max_pts):
    """Return the 4 corner snap points."""
    block = insert.displayBlock()
    w = float(block.param("width"))
    h = float(block.param("height"))
    base = insert.position()
    return [
        lc.EntityCoordinate(base, 0),
        lc.EntityCoordinate(
            lc.geo.Coordinate(base.x() + w, base.y(), base.z()), 1),
        lc.EntityCoordinate(
            lc.geo.Coordinate(base.x() + w, base.y() + h, base.z()), 2),
        lc.EntityCoordinate(
            lc.geo.Coordinate(base.x(), base.y() + h, base.z()), 3),
    ]


def _nearest_point(insert, coord):
    """Nearest point on the rectangle's 4 edges.

    Delegates to the constituent Line entities (each edge is a real
    kernel Line stored under the block, per the create flow).
    """
    doc = insert.document()
    block = insert.displayBlock()
    entities = doc.entitiesByBlock(block).asVector(32767)

    best_pt = None
    best_dist = None
    for e in entities:
        # `e` is a base CADEntity; `nearestPointOnPath` is bound on
        # Snapable (a base interface) — every Line inherits it.
        try:
            candidate = e.nearestPointOnPath(coord)
        except Exception:
            continue
        d = coord.distanceTo(candidate)
        if best_dist is None or d < best_dist:
            best_pt = candidate
            best_dist = d

    if best_pt is None:
        # No lines in the block — degenerate; return zero.  Matches the
        # Lua original's fallback.
        return lc.geo.Coordinate(0, 0, 0)
    return best_pt


def _drag_points(insert):
    """Return the 4 corners as draggable handles."""
    block = insert.displayBlock()
    w = float(block.param("width"))
    h = float(block.param("height"))
    base = insert.position()
    # dict is the correct return type per sub-piece 2b's dispatch hook
    # signature.  Keys are corner IDs (0=BL, 1=BR, 2=TR, 3=TL).
    return {
        0: lc.geo.Coordinate(base.x(),     base.y(),     base.z()),
        1: lc.geo.Coordinate(base.x() + w, base.y(),     base.z()),
        2: lc.geo.Coordinate(base.x() + w, base.y() + h, base.z()),
        3: lc.geo.Coordinate(base.x(),     base.y() + h, base.z()),
    }


def _drag_point_click(insert, builder, point_id):
    """Called when the user starts dragging a corner.

    Removes the 4 line entities from the doc + the storage block; adds
    them to tempEntities for preview.  Also appends `Push`/`Remove`
    operations to the operation builder.

    Per PR-5.5's fixup lesson: `lc.operation.Push()` / `Remove()` are
    NOT valid ctor calls; use `.new()` static factories.
    """
    key = insert.id()
    _drag_selected_point[key] = point_id
    doc = insert.document()
    block = insert.displayBlock()

    # Snapshot the existing lines (kernel Line entities stored under
    # the block) so they can be restored to tempEntities during drag
    # and removed from the document via Push+Remove.
    existing = doc.entitiesByBlock(block).asVector(32767)
    _drag_storage[key] = list(existing)

    # Remove from document; preview via tempEntities.
    _remove_via_builder(builder, doc, list(existing) + [insert])


def _drag_point_release(insert, builder):
    """Called when the user releases the drag.  Commits the new
    rectangle to the document with the updated width/height/base."""
    key = insert.id()
    doc = insert.document()

    base = _drag_base.get(key)
    w = _drag_width.get(key)
    h = _drag_height.get(key)
    if base is None or w is None or h is None:
        return

    # Storage block with the new width/height.  ID key = "LC Plugin"
    # so this insert routes back to THIS plugin on DXF reload.
    block = _create_storage(base, w, h)
    lc.operation.AddBlock.new(doc, block).execute()

    # Add the 4 lines + the CustomEntity Insert.
    eb = lc.operation.EntityBuilder.new(doc)
    p2 = lc.geo.Coordinate(base.x() + w, base.y() + h, base.z())
    for line in _generate_lines(base, p2, insert.layer(), block):
        eb.appendEntity(line)
    eb.appendEntity(_create_rectangle_entity(base, insert.layer(), doc, block))
    eb.execute()

    # Cleanup per-Insert drag state.
    for d in (_drag_storage, _drag_selected_point,
              _drag_width, _drag_height, _drag_base):
        d.pop(key, None)


def _new_drag_point(insert, position):
    """Called continuously as the user drags.  Updates width/height/base
    based on which corner is being dragged."""
    key = insert.id()
    point_id = _drag_selected_point.get(key)
    if point_id is None:
        return

    block = insert.displayBlock()
    w = float(block.param("width"))
    h = float(block.param("height"))
    base = insert.position()

    # The 4 corners: 0=BL, 1=BR, 2=TR, 3=TL.  Compute the new w/h/base
    # from which corner moved to `position`.
    if point_id == 0:
        w = (base.x() + w) - position.x()
        h = (base.y() + h) - position.y()
        base = position
    elif point_id == 1:
        w = position.x() - base.x()
        h = (base.y() + h) - position.y()
        base = lc.geo.Coordinate(base.x(), position.y(), base.z())
    elif point_id == 2:
        w = position.x() - base.x()
        h = position.y() - base.y()
    else:  # point_id == 3
        w = (base.x() + w) - position.x()
        h = position.y() - base.y()
        base = lc.geo.Coordinate(position.x(), base.y(), base.z())

    _drag_width[key] = w
    _drag_height[key] = h
    _drag_base[key] = base


# =============================================================================
# Storage / entity construction helpers.
# =============================================================================

def _create_storage(pos, width, height):
    """Build a `CustomEntityStorage` under plugin name "LC Plugin" so
    both the Lua and Python versions of this plugin bind to the same
    persisted entity name."""
    return lc.meta.CustomEntityStorage(
        "LC Plugin", "Rectangle", pos,
        {"width": str(width), "height": str(height)})


def _create_rectangle_entity(position, layer, document, storage,
                             meta_info=None):
    """Build a `ScriptCustomEntity` wired to the 6 behavior functions
    defined in this module."""
    ceb = lc.builder.CustomEntityBuilder()
    ceb.setLayer(layer)
    ceb.setCoordinate(position)
    ceb.setDocument(document)
    ceb.setDisplayBlock(storage)
    ceb.setSnapFunction(_snap_points)
    ceb.setNearestPointFunction(_nearest_point)
    ceb.setDragPointsFunction(_drag_points)
    ceb.setNewDragPointFunction(_new_drag_point)
    ceb.setDragPointsClickedFunction(_drag_point_click)
    ceb.setDragPointsReleasedFunction(_drag_point_release)
    if meta_info is not None:
        # CustomEntityBuilder inherits setMetaInfo from CADEntityBuilder;
        # this method is bound but may be None at bind time — guarded.
        try:
            ceb.setMetaInfo(meta_info)
        except Exception:
            pass
    return ceb.build()


def _generate_lines(p1, p2, layer, block=None, meta_info=None):
    """Return the 4 kernel Line entities forming the rectangle from
    corner p1 to opposite corner p2."""
    # Corner ordering: BL → BR → TR → TL → BL
    corners = [
        p1,
        lc.geo.Coordinate(p2.x(), p1.y(), p1.z()),
        p2,
        lc.geo.Coordinate(p1.x(), p2.y(), p1.z()),
    ]
    lines = []
    for i in range(4):
        start = corners[i]
        end   = corners[(i + 1) % 4]
        lb = lc.builder.LineBuilder()
        lb.setStartPoint(start)
        lb.setEndPoint(end)
        lb.setLayer(layer)
        if block is not None:
            lb.setBlock(block)
        if meta_info is not None:
            try:
                lb.setMetaInfo(meta_info)
            except Exception:
                pass
        lines.append(lb.build())
    return lines


def _remove_via_builder(builder, doc, entities):
    """Remove `entities` from `doc` via the operation `builder` using
    the Push+Remove pattern."""
    # Per PR-5.5 fixup — Push and Remove are `.def_static("new", ...)`
    # only, NOT `py::init<>()`.  Calling `Push()` / `Remove()` raises
    # TypeError.  Same fix as `remove_operation.py`.
    inner = lc.operation.EntityBuilder.new(doc)
    for e in entities:
        inner.appendEntity(e)
    inner.appendOperation(lc.operation.Push.new())
    inner.appendOperation(lc.operation.Remove.new())
    # `builder` is a `lc::operation::Builder&` — the drag callback's
    # 2nd arg.  It has `.append(other)` which chains a sub-builder.
    # (`Builder::append(Builder_SPtr)` — bound.)
    builder.append(inner)


# =============================================================================
# Interactive create operation — PYRECTANGLE.
# =============================================================================

@lc.register_operation
class PyRectangleOp(CreateOperations):
    """Interactive rectangle creation via 2 corner clicks."""

    name         = "PyRectangleOp"
    command_line = "PYRECTANGLE"
    icon         = "rectangle.svg"
    description  = "Python Rectangle (custom entity)"

    def _init_default(self):
        # Instance state — the PR-5.7 fixup discipline: set state
        # directly, DO NOT call `super().__init__()` (runOperation's
        # instantiate() already ran it).
        self.p1 = None
        self.p2 = None
        self.entities = []  # temp preview lines
        self.step = "AWAIT_P1"
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyRectangle: click corner 1")

    def AWAIT_P1(self, event_name, args):
        if event_name != "point":
            return
        coord = args.get("position")
        if coord is None:
            return
        self.p1 = coord
        self.step = "AWAIT_P2"
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyRectangle: click corner 2")

    def AWAIT_P2(self, event_name, args):
        if event_name == "mouseMove":
            self._temp_rectangle(args.get("position"))
            return
        if event_name != "point":
            return
        coord = args.get("position")
        if coord is None:
            return
        self.p2 = coord
        self._store_rectangle()

    def _temp_rectangle(self, point):
        """Update the preview lines during mouse-move."""
        if self.p1 is None or point is None:
            return
        mw = self._get_main_window()
        if mw is None:
            return
        temp = mw.cadMdiChild().tempEntities()
        # Clear previous preview.
        for e in self.entities:
            temp.removeEntity(e)
        layer = mw.cadMdiChild().activeLayer()
        # New preview — 4 unblocked lines.
        self.entities = _generate_lines(self.p1, point, layer, None)
        for e in self.entities:
            temp.addEntity(e)

    def _store_rectangle(self):
        """Commit the rectangle to the document."""
        mw = self._get_main_window()
        if mw is None:
            return
        mdi = mw.cadMdiChild()
        doc = mdi.document()
        layer = mdi.activeLayer()

        # Clear preview.
        temp = mdi.tempEntities()
        for e in self.entities:
            temp.removeEntity(e)
        self.entities = []

        # Storage block.
        w = self.p2.x() - self.p1.x()
        h = self.p2.y() - self.p1.y()
        block = _create_storage(self.p1, w, h)
        lc.operation.AddBlock.new(doc, block).execute()

        # Add the 4 lines + the CustomEntity Insert.
        eb = lc.operation.EntityBuilder.new(doc)
        for line in _generate_lines(self.p1, self.p2, layer, block):
            eb.appendEntity(line)
        eb.appendEntity(_create_rectangle_entity(self.p1, layer, doc, block))
        eb.execute()

        self.close()


# =============================================================================
# On-reload reconstruction plugin — registers under "LC Plugin".
#
# Fires when NewWaitingCustomEntityEvent is emitted for an Insert whose
# displayBlock is a CustomEntityStorage with pluginName == "LC Plugin".
#
# NOTE (task_e20ba02a): this dispatch is currently BLOCKED by an upstream
# libdxfrw bug that drops all app-data on read (DRW_Entity::parseDxfGroups
# dead loop).  The plugin registration itself is correct; the callback
# will fire once the upstream bug is fixed.  See
# unittest/scripting/customentitymanager_test.cpp's
# `CustomEntityDxfRoundTrip` test comment for the full evidence trail.
# =============================================================================

def _on_new_waiting_custom_entity(insert):
    """Reconstruct a rectangle CustomEntity from a raw Insert waiting
    for its script-defined behaviors to be wired.

    The insert already has layer/coord/document/displayBlock set by
    the DXF reader (or by whatever code fired NewWaitingCustomEntityEvent).
    We build a `ScriptCustomEntity` wired to the module's 6 behavior
    functions, then append it to the document.
    """
    ceb = lc.builder.CustomEntityBuilder()
    # Copy the InsertBuilder-parent fields explicitly.  The Lua
    # original uses `ceb:copy(insert)` — that method doesn't exist
    # on the current CustomEntityBuilder (post-refactor).
    ceb.setLayer(insert.layer())
    ceb.setCoordinate(insert.position())
    ceb.setDocument(insert.document())
    ceb.setDisplayBlock(insert.displayBlock())
    ceb.setSnapFunction(_snap_points)
    ceb.setNearestPointFunction(_nearest_point)
    ceb.setDragPointsFunction(_drag_points)
    ceb.setNewDragPointFunction(_new_drag_point)
    ceb.setDragPointsClickedFunction(_drag_point_click)
    ceb.setDragPointsReleasedFunction(_drag_point_release)
    ce = ceb.build()

    eb = lc.operation.EntityBuilder.new(insert.document())
    eb.appendEntity(ce)
    eb.execute()


# Register the plugin at module import time.  The plugin registration
# hook (installed by lcUI's installEventHooks at MainWindow construction)
# routes this into the process-global LuaCustomEntityManager.  In
# headless CLI mode with no hook installed, this call is a silent
# no-op (matches the sub-piece 1 rationale for the hook slot).
lc.register_plugin("LC Plugin", _on_new_waiting_custom_entity)
