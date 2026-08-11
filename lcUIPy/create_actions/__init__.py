# lcUIPy.create_actions — Python ports of lcUILua/createActions/*.lua.
#
# Phase 5 PR-5.7 populates:
#   * point_operations.py — Python port of pointoperations.lua as a
#                            disabled-by-default proof port + regression
#                            fixture.  Uses `PYPOINT` as command_line
#                            so it coexists with the Lua `POINT` op.
#
# The .lua originals stay authoritative until phase 6 flips the
# default.  These duplicates ship as living tests that PR-5.1's
# CreateOperations base + PR-5.6's gui.* bindings can actually drive
# an interactive create-op end-to-end.
