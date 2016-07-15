local Gear = {}
Gear.__index = Gear

setmetatable(Gear, {
    __call = function (cls, ...)
        return cls.new(...)
    end,
})

function Gear.new()
    local self = setmetatable({}, Gear)
    return self
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
function Gear:calc(builder, N, phi, Pc)
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
        builder:append(Line(Coord(first.x,first.y), Coord(v.x,v.y),layer))
        first=v
    end
    builder:append(Line(Coord(first.x,first.y), Coord(fKeep.x,fKeep.y),layer))

end

local tab = toolbar:tabByName("Quick Access")
local group = tab:addGroup("Gear")

local gearButton = create_button("Gear")
tab:addButton(group, gearButton, 0, 0, 1, 1)
luaInterface:luaConnect(gearButton, "pressed()", function()
    layer = active_layer()
    local gear=Gear()

    b=Builder(active_widget():document())

    gear:calc(b, 20,math.rad(10),math.rad(10))
    gear:calc(b, 10,math.rad(10),math.rad(10))
    b:push()
    b:scale(Coord(0,0),Coord(10,10))
    b:execute()
end)