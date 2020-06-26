Operations = {}
Operations.__index = Operations

setmetatable(Operations, {
	__call = function (o, ...)
		local self = setmetatable({}, o)
		self:_init(...)
		return self
	end,
})

function Operations:_init()
	self.finished = false
end

function Operations:getDistance(center, point)
	if(type(point) == "userdata") then
		return center:distanceTo(point)
	else
		return point
	end
end

function Operations:getAngle(center, point)
	if(type(point) == "userdata") then
		return center:angleTo(point)
	else
		return point
	end
end

function Operations:getCoordinate(coordinate)
	if(type(coordinate) == "userdata") then
		return coordinate
	end

	return nil
end

function Operations:close()
end
