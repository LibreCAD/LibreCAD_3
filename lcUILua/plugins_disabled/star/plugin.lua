local Star = {}
Star.__index = Star

setmetatable(Star, {
    __index = Operations,
    __call = function (cls, ...)
        return cls.new(...)
    end,
})

function Star.new(id)
    local self = setmetatable({}, Star)

    Operations._init(self, id)

    self.n = nil
    self.innerR	= nil
    self.outerR = nil
    self.origin = nil

    self.entities = {}

    luaInterface:registerEvent("number", self)
    luaInterface:registerEvent("point", self)
    luaInterface:registerEvent("mouseMove", self)

    message("Enter the number of star points")

    return self
end

function Star:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "number" or eventName == "point") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:tempStar(...)
    end
end

function Star:newData(data)
    if(self.n == nil) then
        if(type(data) == "number") then
            self.n = data

            message("Click on the star center")
        end
    elseif(self.origin == nil) then
		self.origin = Operations:getCoordinate(data)
		message("Click on the outer radius")
    elseif(self.outerR == nil) then
		self.outerR = Operations:getDistance(self.origin, Operations:getCoordinate(data))
		message("Click on the inner radius")
    elseif(self.innerR == nil) then
        self.innerR = Operations:getDistance(self.origin, Operations:getCoordinate(data))
		
		if(self.innerR ~= nil) then
			self:drawStar()
		end
    end
end

function Star:point_on_circle(radius, angle)
	angle = (angle * math.pi) / 180
    p = {}
    p.x = radius * math.cos(angle)
    p.y = radius * math.sin(angle)
    return p
end

function Star:calc(N, innerR, outerR, layer, metaInfo, block)
	local points = {}
    local entities = {}
	for i=1,N do
		table.insert(points, self:point_on_circle(outerR, ((i * 360) / N)))
		table.insert(points, self:point_on_circle(innerR, ((i * 360) / N) + (180 / N)))
	end
	
    local first = points[1]
	local fKeep = points[1]
    for k,v in pairs(points) do
        table.insert(entities, Line(Coord(first.x, first.y), Coord(v.x, v.y), layer, metaInfo, block))
        first=v
    end
    table.insert(entities, Line(Coord(first.x,first.y), Coord(fKeep.x,fKeep.y), layer, metaInfo, block))

    return entities
end

function Star:drawStar()
    local layer = mainWindow:getCadMdiChild():activeLayer()
    local metaInfo = mainWindow:getCadMdiChild():metaInfoManager():metaInfo()
    local block = Block("Star_" .. math.random(9999999999), Coordinate(0, 0, 0)) --TODO: get proper ID
    local entities = self:calc(self.n, self.innerR, self.outerR, layer, metaInfo, block)

    local b = Builder(active_widget():document(), "Star")
    b:append(AddBlock(active_widget():document(), block))

    local eb = EntityBuilder(active_widget():document())
    for k, v in pairs(entities) do
        eb:appendEntity(v)
    end

    local insertBuilder = InsertBuilder()
    insertBuilder:setLayer(layer)
    insertBuilder:setDisplayBlock(block)
    insertBuilder:setCoordinate(self.origin)
    insertBuilder:setDocument(active_widget():document())
    eb:appendEntity(insertBuilder:build())

    b:append(eb)
    b:execute()

    self:close()
end

function Star:tempStar(point)
    local origin
    local n
    local innerR
    local outerR

    if(self.origin == nil) then
        origin = point
        n = self.n or 5
        outerR = 10
        innerR = 5
    else
        origin = self.origin
        n = self.n or 5
        if(self.outerR == nil) then
            outerR = Operations:getDistance(origin, point)
            innerR = outerR / 2
        else
            outerR = self.outerR
            innerR = Operations:getDistance(origin, point)
        end
    end

	
    for k, v in pairs(self.entities) do
        active_widget():tempEntities():removeEntity(v)
    end

    self.entities = self:calc(n, innerR, outerR, mainWindow:getCadMdiChild():activeLayer(), mainWindow:getCadMdiChild():metaInfoManager():metaInfo())

    for k, v in pairs(self.entities) do
        v = v:move(origin)
        active_widget():tempEntities():addEntity(v)
    end
end

function Star:close()
    if(self.finished == false) then
        self.finished = true

        for k, v in pairs(self.entities) do
            active_widget():tempEntities():removeEntity(v)
        end

        luaInterface:deleteEvent("number", self)
        luaInterface:deleteEvent("point", self)
        luaInterface:deleteEvent("mouseMove", self)

        luaInterface:triggerEvent("operationFinished")
    end
end

local tab = toolbar:tabByName("Quick Access")
local group = tab:addGroup("Star")

if(LC_interface == "gui") then
    local StarButton = create_button("Star")
    tab:addWidget(group, StarButton, 0, 0, 1, 1)
    luaInterface:luaConnect(StarButton, "pressed()", function()
        new_operation()
        luaInterface:setOperation(active_widget().id, Star(active_widget().id))
    end)
end
