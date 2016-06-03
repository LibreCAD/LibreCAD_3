require 'actions.event'

Operations = {}

function Operations:new(id)
	local o = {}
	Operations.__index = self
	setmetatable(o, self)

	self.id = id
	event.register("command", self)

	return o
end

function Operations:onEvent(eventName, ...)
	--If current window is focused
	if(mdiArea:activeSubWindow():widget().id == self.id) then
		if(eventName == "point") then
			self:get_line_point(...)
		elseif(eventName == "command") then
			if(... == "LINE") then
				self:create_line()
			end
		end
	end
end

function Operations:create_line()
	self.line_vars = {}
	self.line_vars['lastPoint'] = nil
	
	message("Draw a line")
	message("Click on first point")
	event.register('point', self)
end

function Operations:get_line_point(point)
	if(self.line_vars['lastPoint'] ~= nil) then
		local lp = self.line_vars['lastPoint']
		local d = mdiArea:activeSubWindow():widget():document()
		local b = Builder(d)
		local layer = d:layerByName("0")
		local l=Line(Coord(lp[1], lp[2]), Coord(point[1], point[2]), layer);
		
		b:append(l)
		b:execute()
		
		event.delete('point', self)
	else
		self.line_vars['lastPoint'] = point
		message("Click on second point")
	end
end