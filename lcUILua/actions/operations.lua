require 'actions.event'
require 'actions.lineoperations'
require 'actions.circleoperations'
require 'actions.arcoperations'
require 'actions.ellipseoperations'
require 'actions.dimalignedoperations'
require 'actions.dimangularoperations'
require 'actions.dimdiametricoperations'
require 'actions.dimlinearoperations'
require 'actions.dimradialoperations'
require 'actions.splineoperations'
require 'actions.lwpolylineoperations'

require 'actions.moveoperation'
require 'actions.rotateoperation'
require 'actions.copyoperation'
require 'actions.scaleoperation'
require 'actions.removeoperation'

Operations = {}
Operations.__index = Operations

setmetatable(Operations, {
	__call = function (o, ...)
		local self = setmetatable({}, o)
		self:_init(...)
		return self
	end,
})

function Operations:_init(id)
	self.id = id
	self.finished = false
end

function Operations:forMe()
	return active_widget().id == self.id
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
