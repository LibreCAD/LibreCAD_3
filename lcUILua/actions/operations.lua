local event = require 'actions.event'

local operations = {}

function operations.create_line()
	print("Draw a line")
	print("Click on first point")	
	event.register('point', operations.get_line_point1, nil)
end

function operations.get_line_point1(point)
	event.delete('point')
	event.register('point', operations.get_line_point2, point[1], point[2])
	print("Click on second point")
end

function operations.get_line_point2(point1, point2)
	d = mdiArea:activeSubWindow():widget():document()
	b = Builder(d)
	layer = d:layerByName("0")
	
	l=Line(Coord(point1[1],point1[2]), Coord(point2[1],point2[2]), layer);
    b:append(l)
    b:execute()
    event.delete('point')
end

function operations.click(x,y)
	event.trigger('point', x, y)
end

return operations