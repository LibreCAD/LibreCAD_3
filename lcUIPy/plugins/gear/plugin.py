"""Python port of the Lua gear sample's interactive drawing path.

The operation creates its outline as block-local Lines, inserts the block
at the chosen origin, previews the outline while the cursor moves, and
removes that preview when the operation finishes. The Python command is
named `PYGEAR` so it can coexist with the Lua sample's `GEAR`; its
Lua-specific toolbar hook is intentionally not duplicated here.
"""

import math
import uuid

import lc

from lcUIPy.create_operations import CreateOperations


def _involute_intersect_angle(rb, r):
    return math.sqrt(r * r - rb * rb) / rb - math.acos(rb / r)


def _point_on_circle(radius, angle):
    return (radius * math.cos(angle), radius * math.sin(angle))


def _make_lines(points, layer, origin, scale, meta_info=None, block=None):
    """Build a closed outline, optionally in a block's local coordinates."""
    ox, oy = origin.x(), origin.y()
    lines = []
    for index, (x1, y1) in enumerate(points):
        x2, y2 = points[(index + 1) % len(points)]
        start = lc.geo.Coordinate(x1 * scale + ox, y1 * scale + oy, 0)
        end = lc.geo.Coordinate(x2 * scale + ox, y2 * scale + oy, 0)
        builder = lc.builder.LineBuilder()
        builder.setStartPoint(start)
        builder.setEndPoint(end)
        builder.setLayer(layer)
        if meta_info is not None:
            builder.setMetaInfo(meta_info)
        if block is not None:
            builder.setBlock(block)
        lines.append(builder.build())
    return lines


def _calc_gear_points(n, phi, pc):
    """Return the outline vertices for a gear with n teeth, pressure
    angle phi (radians), and circular pitch pc (radians)."""
    d = n * pc / math.pi
    r = d / 2.0
    pd = n / d
    db = d * math.cos(phi)
    rb = db / 2.0
    a = 1.0 / pd
    ro = r + a
    b = a  # clearance 0 → dedendum = addendum
    rr = r - b
    two_pi = 2 * math.pi
    half_thick_angle = two_pi / (4 * n)
    pitch_to_base_angle = _involute_intersect_angle(rb, r)
    pitch_to_outer_angle = _involute_intersect_angle(rb, ro)

    points = []
    for x in range(1, n + 1):
        c = x * two_pi / n
        pitch1 = c - half_thick_angle
        base1 = pitch1 - pitch_to_base_angle
        outer1 = pitch1 + pitch_to_outer_angle
        pitch2 = c + half_thick_angle
        base2 = pitch2 + pitch_to_base_angle
        outer2 = pitch2 - pitch_to_outer_angle

        b1 = _point_on_circle(rb, base1)
        p1 = _point_on_circle(r, pitch1)
        o1 = _point_on_circle(ro, outer1)
        o2 = _point_on_circle(ro, outer2)
        p2 = _point_on_circle(r, pitch2)
        b2 = _point_on_circle(rb, base2)

        if rr >= rb:
            pitch_to_root_angle = pitch_to_base_angle - _involute_intersect_angle(rb, rr)
            root1 = pitch1 - pitch_to_root_angle
            root2 = pitch2 + pitch_to_root_angle
            r1 = _point_on_circle(rr, root1)
            r2 = _point_on_circle(rr, root2)
            points.extend([r1, p1, o1, o2, p2, r2])
        else:
            r1 = _point_on_circle(rr, base1)
            r2 = _point_on_circle(rr, base2)
            points.extend([r1, b1, p1, o1, o2, p2, b2, r2])

    return points


@lc.register_operation
class PyGearOp(CreateOperations):
    """Interactive gear operation — Python port of the Lua sample.

    Step machine: AWAIT_TEETH → AWAIT_PHI → AWAIT_PC → AWAIT_ORIGIN →
    AWAIT_SCALE.  The AWAIT_SCALE handler calls `draw()` which in turn
    calls `close()` — matches the Lua sample's terminal state (there
    is no distinct DONE step; the operation just closes).
    Values flow in through `onEvent(name, args)`; `_get_main_window()`
    resolves the active MainWindow via `lcgui.currentMainWindow()`.
    """

    name = "PyGearOp"
    command_line = "PYGEAR"
    group = "Creation"
    description = "Gear (Python sample)"

    # Sub-plan explicitly says NOT to bake this into resource.qrc —
    # the icon file-path fallback added in PR-5.4 resolves plugin
    # icons via <plugin_dir>/icons/ when qrc misses.  We leave the
    # icon unset here since the sample doesn't ship an icon.

    def _init_default(self):
        # Instance state.
        self.n = None
        self.phi = None
        self.pc = None
        self.origin = None
        self.scale_point = None
        self.step = "AWAIT_TEETH"
        self._preview_entities = []
        self._preview_count = 0
        # First prompt to the CLI.
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyGear: enter number of teeth")

    def onEvent(self, event_name, args):
        if event_name == "mouseMove":
            position = args.get("position")
            if position is not None and not self.finished:
                self.tempGear(position)
            return
        super().onEvent(event_name, args)

    def refreshTempEntity(self):
        # This operation's preview is a set of Lines, refreshed on mouseMove,
        # rather than CreateOperations' single builder-produced entity.
        return

    def _remove_preview(self):
        mw = self._get_main_window()
        if mw is not None and self._preview_entities:
            temp_entities = mw.cadMdiChild().tempEntities()
            for entity in self._preview_entities:
                temp_entities.removeEntity(entity)
        self._preview_entities = []
        self._preview_count = 0

    def tempGear(self, cursor_position):
        mw = self._get_main_window()
        if mw is None:
            return

        origin = self.origin
        scale_point = self.scale_point
        if origin is None:
            origin = cursor_position
            scale_point = lc.geo.Coordinate(10, 10, 0)
        elif scale_point is None:
            distance = cursor_position.distanceTo(origin)
            scale_point = lc.geo.Coordinate(distance, distance, 0)

        self._remove_preview()
        scale = scale_point.distanceTo(origin)
        if scale == 0:
            return

        points = _calc_gear_points(
            self.n or 10,
            math.radians(self.phi or 10),
            math.radians(self.pc or 10),
        )
        mdi = mw.cadMdiChild()
        lines = _make_lines(
            points,
            mdi.activeLayer(),
            origin,
            scale,
            mdi.metaInfoManager().metaInfo(),
        )
        temp_entities = mdi.tempEntities()
        for entity in lines:
            temp_entities.addEntity(entity)
        self._preview_entities = lines
        self._preview_count = len(lines)

    def cleanUp(self):
        self._remove_preview()

    # Step-name methods below implement the actual state machine.
    #
    # Phase 5 PR-5.5 fixup — `args` is a real Python DICT here (not a
    # ScriptValue with `.asMap()`).  MainWindow's trigger* slots build
    # `ScriptValue(Map)` payloads via `makeMap` and the
    # PythonCallbackImpl's `toPyLocked` MapKind case materializes those
    # into plain `py::dict` before onEvent fires (verified via
    # mainwindow.cpp:588-626 for the trigger* payload shapes: `number`,
    # `text`, `position`, `key`, `widget` are the top-level keys).
    def AWAIT_TEETH(self, event_name, args):
        if event_name != "number":
            return
        n = args.get("number")
        # Phase 5 PR-5.5 fixup round 2 — accept int OR float.  The
        # real number-event pipeline (CliCommand::enterNumber(double)
        # → MainWindow::triggerNumberEntered(double) →
        # ScriptValue(Kind::Double) → toPyLocked's K::Double case) ALWAYS
        # delivers `py::float_` — no code path produces a py::int at
        # the number-event key.  Coerce to int for the teeth count
        # (fractional teeth make no sense).
        if not isinstance(n, (int, float)):
            return
        teeth = int(n)
        if teeth > 2:
            self.n = teeth
            self.step = "AWAIT_PHI"
            mw = self._get_main_window()
            if mw is not None:
                mw.message("PyGear: enter pressure angle (degrees)")

    def AWAIT_PHI(self, event_name, args):
        if event_name != "number":
            return
        v = args.get("number")
        if v is None:
            return
        self.phi = float(v)
        self.step = "AWAIT_PC"
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyGear: enter circular pitch")

    def AWAIT_PC(self, event_name, args):
        if event_name != "number":
            return
        v = args.get("number")
        if v is None:
            return
        self.pc = float(v)
        self.step = "AWAIT_ORIGIN"
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyGear: click center point")

    def AWAIT_ORIGIN(self, event_name, args):
        if event_name != "point":
            return
        coord = args.get("position")
        if coord is None:
            return
        self.origin = coord
        self.step = "AWAIT_SCALE"
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyGear: click external point")

    def AWAIT_SCALE(self, event_name, args):
        if event_name != "point":
            return
        coord = args.get("position")
        if coord is None:
            return
        self.scale_point = coord
        self.draw()
        # draw() calls close() itself.

    def draw(self):
        mw = self._get_main_window()
        if mw is None:
            return
        mdi = mw.cadMdiChild()
        layer = mdi.activeLayer()
        doc = mdi.document()

        points = _calc_gear_points(self.n, math.radians(self.phi),
                                   math.radians(self.pc))
        if not points:
            return

        # Phase 5 PR-5.5 fixup — the origin-to-scale_point distance is
        # applied as a MULTIPLICATIVE scale on top of the gear geometry
        # computed at true circular-pitch units, matching Lua's exact
        # semantics (verified against plugins_disabled/gear/plugin.lua:
        # `Scale(Coordinate(0,0,0), Coord(distance, distance))` after
        # translate-to-origin).  The pre-fixup code normalized the
        # geometry to hit the scale_point exactly as the outer radius
        # — different final size for the same input.
        ox, oy = self.origin.x(), self.origin.y()
        dx = self.scale_point.x() - ox
        dy = self.scale_point.y() - oy
        scale = math.hypot(dx, dy)
        if scale == 0:
            return

        block = lc.meta.Block(
            "Gear_" + uuid.uuid4().hex,
            lc.geo.Coordinate(0, 0, 0),
        )
        eb = lc.operation.EntityBuilder.new(doc)
        meta_info = mdi.metaInfoManager().metaInfo()
        for entity in _make_lines(
                points, layer, lc.geo.Coordinate(0, 0, 0), 1.0,
                meta_info, block):
            eb.appendEntity(entity)
        eb.appendOperation(lc.operation.Push.new())
        eb.appendOperation(lc.operation.Scale.new(
            lc.geo.Coordinate(0, 0, 0),
            lc.geo.Coordinate(scale, scale, 1),
        ))
        eb.processStack()

        insert_builder = lc.builder.InsertBuilder()
        insert_builder.setLayer(layer)
        insert_builder.setMetaInfo(meta_info)
        insert_builder.setDisplayBlock(block)
        insert_builder.setCoordinate(self.origin)
        insert_builder.setDocument(doc)
        eb.appendEntity(insert_builder.build())

        transaction = lc.operation.Builder(doc, "Gear")
        transaction.append(lc.operation.AddBlock.new(doc, block))
        transaction.append(eb)
        transaction.execute()
        self.close()


# Plugin-level marker: prove the loader ran this file.  The Python
# plugin loader sets LC_interface in the exec namespace before running
# the file; a plugin that wants to gate GUI-only setup checks it.
if globals().get("LC_interface") == "gui":
    # No dostring codegen or luaConnect needed — @lc.register_operation
    # above already handled toolbar/CLI wiring via the OperationLoader
    # second source.
    pass
