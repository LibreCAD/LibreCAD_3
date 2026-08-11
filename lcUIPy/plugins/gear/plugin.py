"""Gear plugin — Python port of lcUILua/plugins_disabled/gear/plugin.lua.

Phase 5 PR-5.5 sample: proves the plugin.py load path works end-to-end
and demonstrates the CreateOperations subclass shape.

Deliberate scope reduction vs the original Lua sample:
  * The Lua sample uses `Block`/`InsertBuilder`/`AddBlock` to draw the
    gear as a scaled block-insert.  Those builders are NOT bound in the
    phase-3.2 lcgui surface (phase 5 PR-5.6 will expand it).  This port
    creates the individual `Line` entities directly via
    `lc.operation.EntityBuilder`, which IS bound — enough to prove the
    plugin fires and the geometry math works.
  * The Lua GUI hook (create_button + luaConnect) is replaced by
    `@lc.register_operation`, which routes through OperationLoader's
    Python second source (PR-5.2) so the operation appears in the CLI +
    context menu without touching widget-specific bindings.
  * No `tempGear` mouse-preview — needs Block/InsertBuilder too.

Structure:
  * Import lc + lcgui at module scope so `LC_interface` (set to "gui"
    by the plugin loader) can be used to gate GUI-specific setup.
  * Register the `PyGearOp` class via `@lc.register_operation` so the
    C++ OperationLoader picks it up alongside Lua ops.
  * Command line: `PYGEAR` (distinct from the Lua sample's `GEAR` so
    they can coexist in a mixed setup).
"""

import math

import lc

from lcUIPy.create_operations import CreateOperations


def _involute_intersect_angle(rb, r):
    return math.sqrt(r * r - rb * rb) / rb - math.acos(rb / r)


def _point_on_circle(radius, angle):
    return (radius * math.cos(angle), radius * math.sin(angle))


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

    Step machine: TEETH → PHI → PC → ORIGIN → SCALE_POINT → DONE.
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
        # First prompt to the CLI.
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyGear: enter number of teeth")

    # onEvent is inherited from CreateOperations; step-name methods
    # below implement the actual state machine.
    def AWAIT_TEETH(self, event_name, args):
        if event_name != "number":
            return
        # args is a Map with `number` key.
        n = args.asMap()["number"].asInt() if hasattr(args, "asMap") else args
        if isinstance(n, int) and n > 2:
            self.n = n
            self.step = "AWAIT_PHI"
            mw = self._get_main_window()
            if mw is not None:
                mw.message("PyGear: enter pressure angle (degrees)")

    def AWAIT_PHI(self, event_name, args):
        if event_name != "number":
            return
        v = args.asMap()["number"].asDouble() if hasattr(args, "asMap") else args
        self.phi = float(v)
        self.step = "AWAIT_PC"
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyGear: enter circular pitch")

    def AWAIT_PC(self, event_name, args):
        if event_name != "number":
            return
        v = args.asMap()["number"].asDouble() if hasattr(args, "asMap") else args
        self.pc = float(v)
        self.step = "AWAIT_ORIGIN"
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyGear: click center point")

    def AWAIT_ORIGIN(self, event_name, args):
        if event_name != "point":
            return
        # args carries `position` in a Map with widget + position keys.
        coord = args.asMap()["position"].asCoordinate() \
            if hasattr(args, "asMap") else args
        self.origin = coord
        self.step = "AWAIT_SCALE"
        mw = self._get_main_window()
        if mw is not None:
            mw.message("PyGear: click external point")

    def AWAIT_SCALE(self, event_name, args):
        if event_name != "point":
            return
        coord = args.asMap()["position"].asCoordinate() \
            if hasattr(args, "asMap") else args
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

        # Scale + translate to match the user's origin + scale point.
        # Origin is the center; scale_point sets the outer radius.
        ox, oy = self.origin.x(), self.origin.y()
        # distance from origin to scale_point sets the gear's outer scale
        dx = self.scale_point.x() - ox
        dy = self.scale_point.y() - oy
        outer_scale = math.hypot(dx, dy)
        # The pre-scale gear's outer radius is `ro`; recompute to
        # normalize.  For simplicity use n * pc / (2 * pi) + addendum
        # from the calc (identical to _calc_gear_points).  Simpler:
        # compute the max radius from the actual points.
        max_r = max(math.hypot(x, y) for (x, y) in points)
        if max_r == 0:
            return
        norm = outer_scale / max_r

        eb = lc.operation.EntityBuilder(doc)
        # Iterate consecutive pairs, closing back to the first vertex.
        n_pts = len(points)
        for i in range(n_pts):
            x1, y1 = points[i]
            x2, y2 = points[(i + 1) % n_pts]
            start = lc.geo.Coordinate(x1 * norm + ox, y1 * norm + oy, 0)
            end = lc.geo.Coordinate(x2 * norm + ox, y2 * norm + oy, 0)
            eb.appendEntity(lc.entity.Line(start, end, layer))
        eb.execute()
        self.close()


# Plugin-level marker: prove the loader ran this file.  The Python
# plugin loader sets LC_interface in the exec namespace before running
# the file; a plugin that wants to gate GUI-only setup checks it.
if globals().get("LC_interface") == "gui":
    # No dostring codegen or luaConnect needed — @lc.register_operation
    # above already handled toolbar/CLI wiring via the OperationLoader
    # second source.
    pass
