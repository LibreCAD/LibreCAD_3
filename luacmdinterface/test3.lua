-- Demo to drawing and rotations
layer=Layer("10", Color(1,0,0,1));
al = AddLayer(active.document(), layer);
al:execute();

d=active.document()
b = Builder(d)
function render (b_, layer_, text, x, y, a, b)
    local c = 0.0;
    for i=0,360,45 do 
	c = math.pi/180 * i;
	b_:append(Text(Coord(x, y), text, 20, c, "standard",0,a,b, layer_));
    end
    b_:append(Circle(Coord(x, y), 5, layer_));
end

render(b, layer, "Top Left", -150, 150, 0, 3);
render(b, layer, "Top Center", 0, 150, 1, 3);
render(b, layer, "Top Right", 150, 150, 2, 3);
render(b, layer, "Middle Left", -150, 0, 0, 2);
render(b, layer, "Middle Center", 0, 0, 1, 2);
render(b, layer, "Middle Right", 150, 0, 2, 2);
render(b, layer, "Bottom Left", -150, -150, 0, 1);
render(b, layer, "Bottom Center", 0, -150, 1, 1);
render(b, layer, "Bottom Right", 150, -150, 2, 1);

-- DRaw suqare (for autoscale)
b:append(Line(Coord(-400,400), Coord(400,400), layer));
b:append(Line(Coord(400,400), Coord(400,-400), layer));
b:append(Line(Coord(400,-400), Coord(-400,-400), layer));
b:append(Line(Coord(-400,-400), Coord(-400,400), layer));


-- Draw center
metaInfo1= MetaInfo():add(MetaColor(Color(1,0,0,1)));
metaInfo2 = MetaInfo():add(MetaColor(Color(0,1,0,1)));
b:append(Line1(Coord(0,0), Coord(0,180), layer, metaInfo1));
b:append(Line1(Coord(0,0), Coord(180,00), layer, metaInfo2));

-- DRaw the thing
b:execute();
