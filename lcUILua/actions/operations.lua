require 'actions.event'
require 'actions.lineoperations'

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
end

function Operations:forMe()
	return active_widget().id == self.id
end