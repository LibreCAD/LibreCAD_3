-- Rectangle plugin — modernized (Phase 6 PR-6.1 sub-piece 3c + fixup).
--
-- Companion to lcUIPy/plugins/rectangle/plugin.py (same "LC Plugin" name
-- so the two versions produce interoperable DXFs).
--
-- Coordinator's PR-6.1 sub-piece 3c review flagged 3 systemic issues in
-- the initial rewrite of this file:
--   1. Bare-global class references (Coordinate, EntityBuilder,
--      CustomEntityBuilder, LineBuilder, Push, Remove, etc.) don't
--      exist in the current Lua bridge — everything is namespaced under
--      lc.*.  luac -p passed the OLD file, but that's meaningless: Lua
--      resolves globals at CALL time, so a completely broken file still
--      parses clean.  This rewrite uses the correct namespaced forms
--      (lc.geo.Coordinate, lc.builder.CustomEntityBuilder, etc.).
--   2. `toolbar` as a bare global doesn't exist — reachable only via
--      `mainWindow:toolbar()`.  Fixed here.
--   3. `CustomEntityBuilder` had ZERO Lua binding until sub-piece 3c
--      fixup (this same commit) — added in lcadluascript/bridge/
--      lc_builder.cpp.  Uses the sub-piece 2b free helpers
--      (setSnapFunctionLua etc.) which preserve the LUA_TFUNCTION
--      guard from sub-piece 2a fixup.
--
-- Also: this file is at `plugins_disabled/rectangle/` — the plugin
-- loader points at `plugins/` per lcUILua/CMakeLists.txt, so this file
-- doesn't auto-load.  Enabling requires renaming the parent directory
-- or moving this file to `plugins/rectangle/`.  Kept disabled for now
-- pending the libdxfrw reload blocker (task_e20ba02a).

-- =============================================================================
-- Module-level per-Insert state for interactive drag operations.
-- =============================================================================

local drag_storage = {}
local drag_selected_point = {}
local drag_width = {}
local drag_height = {}
local drag_base = {}

-- =============================================================================
-- Custom-entity behavior slots — pure functions used by both the
-- interactive create-op and the on-reload reconstruction plugin.
-- =============================================================================

local function snapPoints(insert, coord, constrain, min_dist, max_pts)
    local block = insert:displayBlock()
    local w = tonumber(block:param("width"))
    local h = tonumber(block:param("height"))
    local base = insert:position()

    return {
        lc.EntityCoordinate(base, 0),
        lc.EntityCoordinate(lc.geo.Coordinate(base:x() + w, base:y(), base:z()), 1),
        lc.EntityCoordinate(lc.geo.Coordinate(base:x() + w, base:y() + h, base:z()), 2),
        lc.EntityCoordinate(lc.geo.Coordinate(base:x(), base:y() + h, base:z()), 3),
    }
end

local function nearestPointOnPath(insert, coord)
    local d = insert:document()
    local block = insert:displayBlock()
    local entities = d:entitiesByBlock(block):asVector()

    local best_pt = nil
    local best_dist = nil
    for _, e in pairs(entities) do
        local candidate = e:nearestPointOnPath(coord)
        local dist = coord:distanceTo(candidate)
        if best_pt == nil or dist < best_dist then
            best_pt = candidate
            best_dist = dist
        end
    end

    if best_pt == nil then
        return lc.geo.Coordinate(0, 0, 0)
    end
    return best_pt
end

local function dragPoints(insert)
    local block = insert:displayBlock()
    local w = tonumber(block:param("width"))
    local h = tonumber(block:param("height"))
    local base = insert:position()

    return {
        [0] = lc.geo.Coordinate(base:x(),     base:y(),     base:z()),
        [1] = lc.geo.Coordinate(base:x() + w, base:y(),     base:z()),
        [2] = lc.geo.Coordinate(base:x() + w, base:y() + h, base:z()),
        [3] = lc.geo.Coordinate(base:x(),     base:y() + h, base:z()),
    }
end

-- Forward declarations of the create helpers used inside the drag
-- callbacks.  Lua's local-scope rules require this ordering.
local createStorage
local createInsert
local generate_lines

local function dragPointClicked(insert, builder, point_id)
    local id = insert:id()
    drag_selected_point[id] = point_id
    local doc = insert:document()

    -- Snapshot the existing kernel lines stored under the block.
    local existing = doc:entitiesByBlock(insert:displayBlock()):asVector()
    drag_storage[id] = existing

    -- Preview: add the snapshotted lines to tempEntities so the user
    -- sees the outline during the drag.  Sub-piece 3c fixup — the
    -- previous rewrite had DROPPED this preview code (regression from
    -- the pre-refactor version).  Restored here.
    local temp = mainWindow:cadMdiChild():tempEntities()
    for _, e in pairs(existing) do
        temp:addEntity(e)
    end

    -- Remove existing entities from the document.  Push+Remove chain
    -- inside an inner EntityBuilder, then appended to `builder`.
    -- lc.operation.Push and Remove use `.new()` static factories per
    -- PR-5.5's fixup pattern (identical to Python side).
    local inner = lc.operation.EntityBuilder.new(doc)
    for _, e in pairs(existing) do
        inner:appendEntity(e)
    end
    inner:appendEntity(insert)
    inner:appendOperation(lc.operation.Push.new())
    inner:appendOperation(lc.operation.Remove.new())
    builder:append(inner)

    -- Also remove the block itself.
    builder:append(lc.operation.RemoveBlock.new(doc, insert:displayBlock()))
end

local function dragPointReleased(insert, builder)
    local id = insert:id()
    local doc = insert:document()
    local base = drag_base[id]
    local w = drag_width[id]
    local h = drag_height[id]

    if base == nil or w == nil or h == nil then
        return
    end

    -- Clear preview lines from tempEntities.
    if drag_storage[id] ~= nil then
        local temp = mainWindow:cadMdiChild():tempEntities()
        for _, e in pairs(drag_storage[id]) do
            temp:removeEntity(e)
        end
    end

    -- New block with the updated width/height.
    local block = createStorage(base, w, h)
    builder:append(lc.operation.AddBlock.new(doc, block))

    -- Add the 4 lines + the CustomEntity Insert.
    local eb = lc.operation.EntityBuilder.new(doc)
    local p2 = lc.geo.Coordinate(base:x() + w, base:y() + h, base:z())
    for _, line in pairs(generate_lines(base, p2, insert:layer(), block, insert:metaInfo())) do
        eb:appendEntity(line)
    end
    eb:appendEntity(createInsert(base, insert:layer(), doc, block, insert:metaInfo()))
    builder:append(eb)

    -- Clear per-Insert drag state.
    drag_storage[id] = nil
    drag_selected_point[id] = nil
    drag_width[id] = nil
    drag_height[id] = nil
    drag_base[id] = nil
end

local function newDragPoints(insert, position)
    local id = insert:id()
    local point_id = drag_selected_point[id]
    if point_id == nil then
        return
    end

    local doc = insert:document()

    -- Sub-piece 3c fixup — remove OLD preview lines from tempEntities
    -- before generating new ones for the current drag position.  The
    -- pre-fix rewrite silently dropped this preview code (regression);
    -- restored here.
    local temp = mainWindow:cadMdiChild():tempEntities()
    if drag_storage[id] ~= nil then
        for _, e in pairs(drag_storage[id]) do
            temp:removeEntity(e)
        end
    end

    local block = insert:displayBlock()
    local w = tonumber(block:param("width"))
    local h = tonumber(block:param("height"))
    local base = insert:position()

    if point_id == 0 then
        w = (base:x() + w) - position:x()
        h = (base:y() + h) - position:y()
        base = position
    elseif point_id == 1 then
        w = position:x() - base:x()
        h = (base:y() + h) - position:y()
        base = lc.geo.Coordinate(base:x(), position:y(), base:z())
    elseif point_id == 2 then
        w = position:x() - base:x()
        h = position:y() - base:y()
    else -- point_id == 3
        w = (base:x() + w) - position:x()
        h = position:y() - base:y()
        base = lc.geo.Coordinate(position:x(), base:y(), base:z())
    end

    drag_width[id] = w
    drag_height[id] = h
    drag_base[id] = base

    -- Generate new preview lines from the updated dimensions +
    -- register them for both the next drag update AND the temp
    -- entities.
    local p2 = lc.geo.Coordinate(base:x() + w, base:y() + h, base:z())
    drag_storage[id] = generate_lines(base, p2, insert:layer(), nil, insert:metaInfo())
    for _, e in pairs(drag_storage[id]) do
        temp:addEntity(e)
    end
end

-- =============================================================================
-- Storage / entity construction helpers.
-- =============================================================================

createStorage = function(pos, width, height)
    return lc.meta.CustomEntityStorage("LC Plugin", "Rectangle", pos, {
        width  = tostring(width),
        height = tostring(height),
    })
end

createInsert = function(position, layer, document, storage, metaInfo)
    local ceb = lc.builder.CustomEntityBuilder()
    ceb:setLayer(layer)
    ceb:setCoordinate(position)
    ceb:setDocument(document)
    ceb:setDisplayBlock(storage)
    ceb:setSnapFunction(snapPoints)
    ceb:setNearestPointFunction(nearestPointOnPath)
    ceb:setDragPointsFunction(dragPoints)
    ceb:setNewDragPointFunction(newDragPoints)
    ceb:setDragPointsClickedFunction(dragPointClicked)
    ceb:setDragPointsReleasedFunction(dragPointReleased)
    if metaInfo ~= nil then
        ceb:setMetaInfo(metaInfo)
    end
    return ceb:build()
end

generate_lines = function(p1, p2, layer, block, metaInfo)
    local lines = {}
    local corners = {
        p1,
        lc.geo.Coordinate(p2:x(), p1:y(), p1:z()),
        p2,
        lc.geo.Coordinate(p1:x(), p2:y(), p1:z()),
    }
    for i = 1, 4 do
        local start = corners[i]
        local endc  = corners[(i % 4) + 1]
        local lb = lc.builder.LineBuilder()
        lb:setStart(start)
        lb:setEnd(endc)
        lb:setLayer(layer)
        if block ~= nil then
            lb:setBlock(block)
        end
        if metaInfo ~= nil then
            lb:setMetaInfo(metaInfo)
        end
        table.insert(lines, lb:build())
    end
    return lines
end

-- =============================================================================
-- Interactive create operation — RECTANGLE.
--
-- Uses the CreateOperations base class shape (per createActions/
-- pointoperations.lua's pattern).  Two clicks: p1, p2.
-- =============================================================================

Rectangle = {}
Rectangle.__index = Rectangle

setmetatable(Rectangle, {
    __index = Operations,
    __call = function(o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function Rectangle:_init()
    Operations._init(self)

    self.p1 = nil
    self.p2 = nil
    self.entities = {}

    luaInterface:registerEvent("point",     self)
    luaInterface:registerEvent("mouseMove", self)

    message("Rectangle: click corner 1")
end

function Rectangle:onEvent(eventName, ...)
    if Operations.forMe and Operations.forMe(self) == false then
        return
    end

    if eventName == "point" then
        self:newData(...)
    elseif eventName == "mouseMove" then
        self:tempRectangle(...)
    end
end

function Rectangle:newData(data)
    if self.p1 == nil then
        self.p1 = data
        message("Rectangle: click corner 2")
    elseif self.p2 == nil then
        self.p2 = data
        self:storeRectangle()
    end
end

function Rectangle:tempRectangle(point)
    if self.p1 == nil or point == nil then
        return
    end

    -- Clear previous preview.
    local temp = mainWindow:cadMdiChild():tempEntities()
    for _, e in pairs(self.entities) do
        temp:removeEntity(e)
    end

    local layer = mainWindow:cadMdiChild():activeLayer()
    self.entities = generate_lines(self.p1, point, layer, nil, nil)
    for _, e in pairs(self.entities) do
        temp:addEntity(e)
    end
end

function Rectangle:storeRectangle()
    local mdi = mainWindow:cadMdiChild()
    local doc = mdi:document()
    local layer = mdi:activeLayer()
    local metaInfo = mdi:metaInfoManager():metaInfo()

    -- Clear preview.
    local temp = mdi:tempEntities()
    for _, e in pairs(self.entities) do
        temp:removeEntity(e)
    end
    self.entities = {}

    -- Storage block.
    local w = self.p2:x() - self.p1:x()
    local h = self.p2:y() - self.p1:y()
    local block = createStorage(self.p1, w, h)
    lc.operation.AddBlock.new(doc, block):execute()

    -- Add the 4 lines + the CustomEntity Insert.
    local eb = lc.operation.EntityBuilder.new(doc)
    for _, line in pairs(generate_lines(self.p1, self.p2, layer, block, metaInfo)) do
        eb:appendEntity(line)
    end
    eb:appendEntity(createInsert(self.p1, layer, doc, block, metaInfo))
    eb:execute()

    self:close()
end

function Rectangle:close()
    if self.finished then
        return
    end
    self.finished = true

    local temp = mainWindow:cadMdiChild():tempEntities()
    for _, e in pairs(self.entities) do
        temp:removeEntity(e)
    end

    luaInterface:deleteEvent("point",     self)
    luaInterface:deleteEvent("mouseMove", self)
    luaInterface:triggerEvent("operationFinished", nil)
end

-- =============================================================================
-- GUI wiring (Quick Access toolbar button).
--
-- Sub-piece 3c fixup — was using bare `toolbar` global which doesn't
-- exist; only reachable via `mainWindow:toolbar()`.  Also uses the
-- modern `Toolbar:addButton(name, icon, group, cb, tooltip)` signature
-- instead of the removed `luaConnect` + `create_button` combo.
-- =============================================================================

if LC_interface == "gui" then
    mainWindow:toolbar():addButton("Rectangle", "", "Quick Access",
        function()
            mainWindow:runOperationByName("Rectangle")
        end,
        "Rectangle (custom entity)")
end

-- =============================================================================
-- On-reload reconstruction plugin — registers under "LC Plugin".
--
-- Fires when NewWaitingCustomEntityEvent is emitted for an Insert whose
-- displayBlock is a CustomEntityStorage with pluginName == "LC Plugin".
--
-- Sub-piece 3c fixup — CRITICAL: `ceb:setID(insert:id())` REPLACES the
-- placeholder Insert already sitting in the document; without this, the
-- placeholder stays AND the rebuilt entity is added alongside it,
-- duplicating every custom entity on every reload.  Same fix applied
-- to the Python companion.
--
-- NOTE (task_e20ba02a): the DXF reload dispatch is currently BLOCKED by
-- an upstream libdxfrw bug (DRW_Entity::parseDxfGroups dead loop drops
-- all app-data on read).  The plugin registration itself is correct
-- today; the callback fires only after the upstream fix.
-- =============================================================================

local function onNewWaitingCustomEntity(insert)
    local ceb = lc.builder.CustomEntityBuilder()
    -- Set the parent InsertBuilder fields explicitly.  The pre-refactor
    -- version used `ceb:copy(insert)` which never existed on the C++
    -- side.
    ceb:setLayer(insert:layer())
    ceb:setCoordinate(insert:position())
    ceb:setDocument(insert:document())
    ceb:setDisplayBlock(insert:displayBlock())
    -- Sub-piece 3c fixup — CRITICAL replace-not-duplicate semantic.
    -- See docstring above.
    ceb:setID(insert:id())
    ceb:setSnapFunction(snapPoints)
    ceb:setNearestPointFunction(nearestPointOnPath)
    ceb:setDragPointsFunction(dragPoints)
    ceb:setNewDragPointFunction(newDragPoints)
    ceb:setDragPointsClickedFunction(dragPointClicked)
    ceb:setDragPointsReleasedFunction(dragPointReleased)
    local ce = ceb:build()

    local eb = lc.operation.EntityBuilder.new(insert:document())
    eb:appendEntity(ce)
    eb:execute()
end

-- Global `registerPlugin` — bound in lcadluascript/lclua.cpp.  Same
-- behavior as `LuaCustomEntityManager.getInstance():registerPlugin(...)`
-- but without the singleton-access boilerplate.
registerPlugin("LC Plugin", onNewWaitingCustomEntity)
