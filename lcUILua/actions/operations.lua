local event = require 'actions.event'

local operations = {}

function operations.create_line()
	operations.line_vars = {}
	operations.line_vars['lastPoint'] = nil
	
	print("Draw a line")
	print("Click on first point")	
	event.register('point', operations.get_line_point)
end

function operations.get_line_point(point)
	if(operations.line_vars['lastPoint'] ~= nil) then
		lp = operations.line_vars['lastPoint']
		d = mdiArea:activeSubWindow():widget():document()
		b = Builder(d)
		layer = d:layerByName("0")
		l=Line(Coord(lp[1], lp[2]), Coord(point[1], point[2]), layer);
		
		b:append(l)
		b:execute()
		
		event.delete('point', operations.get_line_point)
	else
		operations.line_vars['lastPoint'] = point
		print("Click on second point")
	end
end

function operations.click(x,y)
	event.trigger('point', x, y)
end

return operations