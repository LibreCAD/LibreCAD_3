"""Python port of lcUILua/actions/operations.lua.

Phase 5 PR-5.1 — the `Operations` base class for Python-defined tool
operations.  Mirrors the semantics (not syntax) of the Lua original:
distance/angle/coordinate helpers, close, pauseCreation, isPaused.

The instance's `step` attribute is the state machine tag — same
mechanism as Lua (a string like `'FIRST_POINT'` or `'PAUSE'`).
ContextMenuManager reads `step` (via ScriptObject.getAttr) to compute
the current step's transitions, and writes it (via setAttr) when a
transition menu item fires.
"""

import lc


class Operations:
    """Base class for tool operations in Python.

    Subclasses must set `self.step` (or leave it None until the first
    transition) and are expected to define step-named methods that
    receive `(self, event_name, data)`.
    """

    def __init__(self):
        # `finished` flips to True at close() so re-entrancy from
        # tail-triggered events is a no-op.
        self.finished = False
        # `step` is the state-machine tag (str).  Subclasses set it.
        self.step = None
        # `lastStep` is the resume target after pauseCreation().
        self.lastStep = None

    def getDistance(self, center, point):
        """Return the distance from `center` to `point`.

        If `point` is a Coordinate (userdata), use its distanceTo();
        otherwise treat `point` as a raw distance number and return
        it unchanged — same conditional as the Lua original.
        """
        # In Lua the discriminator was type(point) == "userdata".  In
        # Python we check for the Coordinate attr instead.
        if hasattr(point, "distanceTo"):
            return center.distanceTo(point)
        return point

    def getAngle(self, center, point):
        """Return the angle from `center` to `point`."""
        if hasattr(point, "angleTo"):
            return center.angleTo(point)
        return point

    def getCoordinate(self, coordinate):
        """Return `coordinate` if it's a Coordinate userdata, else None."""
        # Lua returned nil for non-userdata; Python analog is None.
        if hasattr(coordinate, "x"):
            return coordinate
        return None

    def close(self):
        """Hook for subclasses.  Default: no-op."""
        pass

    def pauseCreation(self, pause):
        """Pause/resume creation.

        On pause, stash the current step under `lastStep` and flip
        `step` to 'PAUSE'.  On resume, restore `step` from `lastStep`.
        """
        if pause:
            self.lastStep = self.step
            self.step = 'PAUSE'
        else:
            if self.lastStep is None:
                return
            self.step = self.lastStep

    def isPaused(self):
        return self.step == 'PAUSE'
