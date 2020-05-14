local Spiral = {}
Spiral.__index = Spiral

setmetatable(Spiral, {
    __index = Operations,
    __call = function (cls, ...)
        return cls.new(...)
    end,
})

function Spiral.new(id)
    local self = setmetatable({}, Spiral)

    Operations._init(self, id)

    self.n = nil
    self.R	= nil
    self.origin = nil

    self.entities = {}

    luaInterface:registerEvent("number", self)
    luaInterface:registerEvent("point", self)
    luaInterface:registerEvent("mouseMove", self)

    message("Enter the number of rotations in the spiral")

    return self
end

function Spiral:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "number" or eventName == "point") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:tempSpiral(...)
    end
end

function Spiral:newData(data)
    if(self.n == nil) then
        if(type(data) == "number") then
            self.n = data

            message("Enter the radius of the spiral")
        end
	elseif(self.R == nil) then
		if(type(data) == "number") then
			self.R = data
			message("Click on the spiral center")
		end
    elseif(self.origin == nil) then
		self.origin = Operations:getCoordinate(data)
		
		if(self.origin ~= nil) then
			self:drawSpiral()
		end
    end
end

function Spiral:point_on_circle(radius, angle)
    p = {}
    p.x = radius * math.cos(angle)
    p.y = radius * math.sin(angle)
    return p
end

function Spiral:calc(N, R, layer, metaInfo, block)
	local points = {}
	local arcCenter
    local entities = {}
	local half = true
	local kR = (R / (2 * math.pi)) / N
	
	for i = math.pi/2, (2*math.pi*N) + (math.pi/2), math.pi do
		table.insert(points, self:point_on_circle(kR*i, i))
	end
	
    for k,v in pairs(points) do
		half = not half
		if(half) then			
			arcCenter = Coordinate(math.pi * kR, 0)
			table.insert(entities, Arc(arcCenter, math.abs(v.y), 0, math.pi, false, layer, metaInfo, block))
		else		
			arcCenter = Coordinate(0, 0)
			table.insert(entities, Arc(arcCenter, math.abs(v.y), math.pi, math.pi*2, false, layer, metaInfo, block))
		end
    end

    return entities
end

function Spiral:drawSpiral()
    local layer = mainWindow:getCadMdiChild():activeLayer()
    local metaInfo = mainWindow:getCadMdiChild():metaInfoManager():metaInfo()
    local block = Block("Spiral_" .. math.random(9999999999), Coordinate(0, 0, 0)) --TODO: get proper ID

    local entities = self:calc(self.n, self.R, layer, metaInfo, block)

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

function Spiral:tempSpiral(point)
    local n = self.n or 5
	local R = self.R or 5
	local origin = self.origin or point
	
    for k, v in pairs(self.entities) do
        active_widget():tempEntities():removeEntity(v)
    end

    self.entities = self:calc(n, R, mainWindow:getCadMdiChild():activeLayer(), mainWindow:getCadMdiChild():metaInfoManager():metaInfo())

    for k, v in pairs(self.entities) do
        v = v:move(origin)
        active_widget():tempEntities():addEntity(v)
    end
end

function Spiral:close()
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

if(LC_interface == "gui") then
    local tab = toolbar:tabByName("Quick Access")
    local group = tab:addGroup("Spiral")

    local SpiralButton = create_button("Spiral")
    tab:addWidget(group, SpiralButton, 0, 0, 1, 1)
    luaInterface:luaConnect(SpiralButton, "pressed()", function()
        new_operation()
        luaInterface:setOperation(active_widget().id, Spiral(active_widget().id))
    end)
end
