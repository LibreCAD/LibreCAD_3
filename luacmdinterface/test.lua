start = microtime()
local r = 10;
local d = 0;
local rx = 100;
local ry = 100;
local p =rx;
local q=ry;

doc=active.document()
ce=Builder(doc,em)
layer = active.proxy.layerByName("0")
local count=0
while (d< 8*math.pi) do
    local x=rx+(math.sin(d)*d)*r;
    local y=ry+(math.sin(d+(math.pi/2))*(d+(math.pi/2)) * r);
    if (d>0) then
        ce:append(Line(Coord(x,y), Coord(p,q),layer));
    end
    p=x;
    q=y;
    d=d + 0.005;
   count=count + 1
end
ce:execute()
print "done";
print (count)
print (microtime()-start);