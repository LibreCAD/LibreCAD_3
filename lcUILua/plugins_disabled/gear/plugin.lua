local Gear = {}
Gear.__index = Gear

setmetatable(Gear, {
    __index = Operations,
    __call = function (cls, ...)
        return cls.new(...)
    end,
})

function Gear.new(id)
    local self = setmetatable({}, Gear)

    Operations._init(self, id)

    self.n = nil
    self.phi = nil
    self.pc = nil
    self.origin = nil
    self.scalePoint = nil

    self.entities = {}

    luaInterface:registerEvent("number", self)
    luaInterface:registerEvent("point", self)
    luaInterface:registerEvent("mouseMove", self)

    message("Enter number of teeth")

    return self
end

function Gear:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "number" or eventName == "point") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:tempGear(...)
    end
end

function Gear:newData(data)
    if(self.n == nil) then
        if(type(data) == "number") then
            self.n = data

            message("Enter pressure angle")
        end
    elseif(self.phi == nil) then
        if(type(data) == "number") then
            self.phi = data

            message("Enter circilar pitch")
        end
    elseif(self.pc == nil) then
        if(type(data) == "number") then
            self.pc = data

            message("Give center point")
        end
    elseif(self.origin == nil) then
        self.origin = Operations:getCoordinate(data)

        message("Give external point")
    elseif(self.scalePoint == nil) then
        self.scalePoint = Operations:getCoordinate(data)

        if(self.scalePoint ~= nil) then
            self:drawGear()
        end
    end
end

function Gear:involute_intersect_angle(Rb, R)
    return (math.sqrt(R*R - Rb*Rb) / (Rb)) - (math.acos(Rb / R))
end

function Gear:point_on_circle(radius, angle)
    p = {}
    p.x= radius * math.cos(angle)
    p.y= radius * math.sin(angle)
    return p
end

--ce CreateEntities object
--phi = pressure angle
--PC = Circular Pitch
--teeth = no of teeth
function Gear:calc(N, phi, Pc, layer, metaInfo, block)
    local entities = {}

    -- Pitch Circle
    local D = N * Pc / math.pi
    local R = D / 2.0

    -- Diametrial pitch
    local Pd = N / D

    -- Base Circle
    local Db = D * math.cos(phi)
    local Rb = Db / 2.0

    -- Addendum
    local a = 1.0 / Pd

    -- Outside Circle
    local Ro = R + a
    local Do = 2 * Ro

    -- Tooth thickness
    local T = (math.pi * D) / (2 * N)

    -- undercut?
    local U = (2 / (math.sin(phi) * (math.sin(phi))))
    local needs_undercut = N < U
    -- sys.stderr.write("N:%s R:%s Rb:%s\n" % (N,R,Rb))

    -- Clearance
    local c = 0.0
    -- Dedendum
    local b = a + c

    -- Root Circle
    local Rr = R - b
    local Dr = 2 * Rr

    local two_pi = 2 * math.pi
    local half_thick_angle = two_pi / (4 * N)
    local pitch_to_base_angle = self:involute_intersect_angle(Rb, R)
    local pitch_to_outer_angle = self:involute_intersect_angle(Rb, Ro) -- pitch_to_base_angle

    local points ={}
    for x=1,N do
        local c = x * two_pi / N;

        -- angles
        local pitch1 = c - half_thick_angle
        local base1 = pitch1 - pitch_to_base_angle
        local outer1 = pitch1 + pitch_to_outer_angle

        local pitch2 = c + half_thick_angle
        local base2 = pitch2 + pitch_to_base_angle
        local outer2 = pitch2 - pitch_to_outer_angle

        -- points
        local b1 = self:point_on_circle(Rb, base1)
        local p1 = self:point_on_circle(R, pitch1)
        local o1 = self:point_on_circle(Ro, outer1)
        local o2 = self:point_on_circle(Ro, outer2)
        local p2 = self:point_on_circle(R, pitch2)
        local b2 = self:point_on_circle(Rb, base2)

        if Rr >= Rb then
            local pitch_to_root_angle = pitch_to_base_angle - self:involute_intersect_angle(Rb, Rr)
            local root1 = pitch1 - pitch_to_root_angle
            local root2 = pitch2 + pitch_to_root_angle
            local r1 = self:point_on_circle(Rr, root1)
            local r2 = self:point_on_circle(Rr, root2)

            table.insert(points, r1)
            table.insert(points, p1)
            table.insert(points, o1)
            table.insert(points, o2)
            table.insert(points, p2)
            table.insert(points, r2)

        else
            local r1 = self:point_on_circle(Rr, base1)
            local r2 = self:point_on_circle(Rr, base2)
            table.insert(points, r1)
            table.insert(points, b1)
            table.insert(points, p1)
            table.insert(points, o1)
            table.insert(points, o2)
            table.insert(points, p2)
            table.insert(points, b2)
            table.insert(points, r2)
        end
    end
    local first = points[1]
    local fKeep = first
    table.remove(points, 1)
    for k,v in pairs(points) do
        table.insert(entities, Line(Coord(first.x,first.y), Coord(v.x,v.y), layer, metaInfo, block))
        first=v
    end
    table.insert(entities, Line(Coord(first.x,first.y), Coord(fKeep.x,fKeep.y), layer, metaInfo, block))

    return entities
end

function Gear:drawGear()
    local layer = mainWindow:cadMdiChild():activeLayer()
    local metaInfo = mainWindow:cadMdiChild():metaInfoManager():metaInfo()
    local block = Block("Gear_" .. math.random(9999999999), Coordinate(0, 0, 0)) --TODO: get proper ID

    local entities = Gear:calc(self.n, math.rad(self.phi), math.rad(self.pc), layer, metaInfo, block)

    local b = Builder(active_widget():document(), "Gear")
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

    eb:appendOperation(Push())

    local distance = self.scalePoint:distanceTo(self.origin)
    eb:appendOperation(Scale(Coordinate(0, 0, 0), Coord(distance, distance)))

    eb:processStack()
    eb:appendEntity(insertBuilder:build())
    b:append(eb)

    b:execute()

    self:close()
end

function Gear:tempGear(point)
    local n = self.n or 10
    local phi = self.phi or 10
    local pc = self.pc or 10
    local origin = self.origin
    local scalePoint = self.scalePoint

    if(origin == nil) then
        origin = point
        scalePoint = Coord(10, 10)
    elseif(scalePoint == nil) then
        local distance = point:distanceTo(origin)
        scalePoint = Coord(distance, distance)
    end

    for k, v in pairs(self.entities) do
        active_widget():tempEntities():removeEntity(v)
    end

    self.entities = self:calc(n, math.rad(phi), math.rad(pc), mainWindow:cadMdiChild():activeLayer(), mainWindow:cadMdiChild():metaInfoManager():metaInfo())

    for k, v in pairs(self.entities) do
        v = v:move(origin):scale(origin, scalePoint)
        active_widget():tempEntities():addEntity(v)
    end
end

function Gear:close()
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
    local group = tab:addGroup("Gear")

    local gearButton = create_button("Gear")
    tab:addWidget(group, gearButton, 0, 0, 1, 1)
    luaInterface:luaConnect(gearButton, "pressed()", function()
        new_operation()
        luaInterface:setOperation(active_widget().id, Gear(active_widget().id))
    end)
end
