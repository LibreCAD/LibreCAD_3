local drag_storage = {}
local drag_selected_point = {}
local drag_width = {}
local drag_height = {}
local drag_base = {}

function snapPoints(insert, a, b, c, d)
    local points = {}

    local block = insert:displayBlock()
    local w = tonumber(block:param("width"))
    local h = tonumber(block:param("height"))
    local base = insert:position()

    table.insert(points, EntityCoordinate(base, 0))
    table.insert(points, EntityCoordinate(Coordinate(base:x() + w, base:y(), base:z()), 1))
    table.insert(points, EntityCoordinate(Coordinate(base:x() + w, base:y() + h, base:z()), 2))
    table.insert(points, EntityCoordinate(Coordinate(base:x(), base:y() + h, base:z()), 3))

    return points
end

function nearestPointOnPath(insert, coord)
    local point
    local distance

    local d = insert:document()
    local block = insert:displayBlock()

    local entities = d:entitiesByBlock(block):asVector()
    for k, v in pairs(entities) do
        local tmp = v:nearestPointOnPath(coord)
        local tmpDistance = coord:distanceTo(tmp)

        if(point == nil or tmpDistance < distance) then
            point = tmp
            distance = tmpDistance
        end
    end

    if(point == nil) then
        return Coordinate(0, 0, 0)
    end
    return point
end

function dragPointClicked(insert, builder, point)
    drag_storage[insert:id()] = {}
    drag_selected_point[insert:id()] = point

    local d = insert:document()

    --Remove old entities
    local removeEntitiesBuilder = EntityBuilder(d)

    drag_storage[insert:id()] = d:entitiesByBlock(insert:displayBlock()):asVector()
    for k, v in pairs(drag_storage[insert:id()]) do
        removeEntitiesBuilder:appendEntity(v)
        active_widget():tempEntities():addEntity(v)
    end
    removeEntitiesBuilder:appendEntity(insert)

    removeEntitiesBuilder:appendOperation(Push())
    removeEntitiesBuilder:appendOperation(Remove())
    builder:append(removeEntitiesBuilder)

    --Remove block
    builder:append(RemoveBlock(d, insert:displayBlock()))
end

function dragPointReleased(insert, builder)
    local d = insert:document()
    local base = drag_base[insert:id()]
    local w = drag_width[insert:id()]
    local h = drag_height[insert:id()]

    for k, v in pairs(drag_storage[insert:id()]) do
        active_widget():tempEntities():removeEntity(v)
    end

    local block = createStorage(base, w, h)
    builder:append(AddBlock(d, block))

    local eb = EntityBuilder(d)
    local lines = generate_lines(base, Coordinate(base:x() + w, base:y() + h, base:z()), insert:layer(), block, insert:metaInfo())
    for k, v in pairs(lines) do
        eb:appendEntity(v)
    end

    eb:appendEntity(createInsert(base, insert:layer(), d, block, insert:metaInfo()))
    builder:append(eb)

    drag_storage[insert:id()] = nil
    drag_selected_point[insert:id()] = nil
    drag_width[insert:id()] = nil
    drag_height[insert:id()] = nil
    drag_base[insert:id()] = nil
end


function dragPoints(insert)
    local points = {}

    local block = insert:displayBlock()
    local w = tonumber(block:param("width"))
    local h = tonumber(block:param("height"))
    local base = insert:position()


    points[0] = Coordinate(base:x(), base:y(), base:z())
    points[1] = Coordinate(base:x() + w, base:y(), base:z())
    points[2] = Coordinate(base:x() + w, base:y() + h, base:z())
    points[3] = Coordinate(base:x(), base:y() + h, base:z())

    return points
end

function newDragPoints(insert, position)
    for k, v in pairs(drag_storage[insert:id()]) do
        active_widget():tempEntities():removeEntity(v)
    end

    local block = insert:displayBlock()
    local w = tonumber(block:param("width"))
    local h = tonumber(block:param("height"))
    local base = insert:position()
    local pointID = drag_selected_point[insert:id()]

    if(pointID == 0) then
        w = (base:x() + w) - position:x()
        h = (base:y() + h) - position:y()
        base = position
    elseif(pointID == 1) then
        w = position:x() - base:x()
        h = (base:y() + h) - position:y()
        base = Coordinate(base:x(), position:y(), base:z())
    elseif(pointID == 2) then
        w = position:x() - base:x()
        h = position:y() - base:y()
    else
        w = (base:x() + w) - position:x()
        h = position:y() - base:y()
        base = Coordinate(position:x(), base:y(), base:z())
    end

    drag_width[insert:id()] = w
    drag_height[insert:id()] = h
    drag_base[insert:id()] = base

    drag_storage[insert:id()] = generate_lines(base, Coordinate(base:x() + w, base:y() + h, base:z()), insert:layer(), nil, insert:metaInfo())

    for k, v in pairs(drag_storage[insert:id()]) do
        active_widget():tempEntities():addEntity(v)
    end

    return insert
end

function generate_lines(p1, p2, layer, block, metaInfo)
    local lines = {}
    local lb = LineBuilder()
    lb:setLayer(layer)

    if(block ~= nil) then
        lb:setBlock(block)
    end

    if(metaInfo ~= nil) then
        lb:setMetaInfo(metaInfo)
    end

    lb:setStart(p1)
    lb:setEnd(Coordinate(p2:x(), p1:y(), p1:z()))

    lines[1] = lb:build()

    lb:setStart(p2)
    lb:newID()
    lines[2] = lb:build()

    lb:setEnd(Coordinate(p1:x(), p2:y(), p1:z()))
    lb:newID()
    lines[3] = lb:build()

    lb:setStart(p1)
    lb:newID()
    lines[4] = lb:build()

    return lines;
end

function createStorage(pos, width, height)
    return CustomEntityStorage("LC Plugin", "Rectangle", pos, {
        width = width,
        height = height
    })
end

function createInsert(position, layer, document, storage, metaInfo)
    local ceb = CustomEntityBuilder()

    if(metaInfo ~= nil) then
        ceb:setMetaInfo(metaInfo)
    end

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

    return ceb:build()
end

local Rectangle = {}
Rectangle.__index = Rectangle

setmetatable(Rectangle, {
    __index = Operations,
    __call = function (cls, ...)
        return cls.new(...)
    end,
})

function Rectangle.new(id)
    local self = setmetatable({}, Rectangle)

    Operations._init(self, id)

    self.p1 = nil
    self.p2 = nil

    self.entities = {}

    luaInterface:registerEvent("point", self)
    luaInterface:registerEvent("mouseMove", self)

    message("Enter corner 1")

    return self
end

function Rectangle:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:tempRectangle(...)
    end
end

function Rectangle:newData(data)
    if(self.p1 == nil) then
        self.p1 = data
        message("Enter corner 2")
    elseif(self.p2 == nil) then
        self.p2 = data
        self:storeRectangle()
    end
end

function Rectangle:storeRectangle()
    local d = active_widget():document()
    local b = Builder(d, "Rectangle")

    local block = createStorage(self.p1, self.p2:x() - self.p1:x(), self.p2:y() - self.p1:y())
    b:append(AddBlock(d, block))

    local entities = generate_lines(self.p1, self.p2, mainWindow:cadMdiChild():activeLayer(), block, mainWindow:cadMdiChild():metaInfoManager():metaInfo())
    local eb = EntityBuilder(d)

    for k, v in pairs(entities) do
        eb:appendEntity(v)
    end

    eb:appendEntity(createInsert(self.p1, mainWindow:cadMdiChild():activeLayer(), d, block, mainWindow:cadMdiChild():metaInfoManager():metaInfo()))
    b:append(eb)

    b:execute()

    self:close()
end

function Rectangle:tempRectangle(point)
    if(self.p1 == nil) then
        return
    end

    for k, v in pairs(self.entities) do
        active_widget():tempEntities():removeEntity(v)
    end

    self.entities = generate_lines(self.p1, point, mainWindow:cadMdiChild():activeLayer(), nil, mainWindow:cadMdiChild():metaInfoManager():metaInfo())

    for k, v in pairs(self.entities) do
        active_widget():tempEntities():addEntity(v)
    end
end

function Rectangle:close()
    if(self.finished == false) then
        self.finished = true

        for k, v in pairs(self.entities) do
            active_widget():tempEntities():removeEntity(v)
        end

        luaInterface:deleteEvent("point", self)
        luaInterface:deleteEvent("mouseMove", self)

        luaInterface:triggerEvent("operationFinished", nil)
    end
end

if(LC_interface == "gui") then
    local tab = toolbar:tabByName("Quick Access")
    local group = tab:addGroup("Rectangle")

    local RectangleButton = create_button("Rectangle")

    tab:addWidget(group, RectangleButton, 0, 0, 1, 1)
    luaInterface:luaConnect(RectangleButton, "pressed()", function()
        new_operation()
        luaInterface:setOperation(active_widget().id, Rectangle(active_widget().id))
    end)
end

local function onNewWaitingCustomEntity(insert)
    local ceb = CustomEntityBuilder()
    ceb:copy(insert)
    ceb:setSnapFunction(snapPoints)
    ceb:setNearestPointFunction(nearestPointOnPath)
    ceb:setDragPointsFunction(dragPoints)
    ceb:setNewDragPointFunction(newDragPoints)
    ceb:setDragPointsClickedFunction(dragPointClicked)
    ceb:setDragPointsReleasedFunction(dragPointReleased)
    local ce = ceb:build()

    local b = EntityBuilder(insert:document())
    b:appendEntity(ce)
    b:execute()
end
LuaCustomEntityManager.getInstance():registerPlugin("LC Plugin", onNewWaitingCustomEntity)
