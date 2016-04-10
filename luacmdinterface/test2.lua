layer=Layer("10", Color(1,0,0,1));
al = AddLayer(active.document(), layer);
al:execute();

metaInfo1= MetaInfo():add(MetaColor(Color(1,0,0,1)));
metaInfo2 = MetaInfo():add(MetaColor(Color(0,1,0,1)));
l1=Line1(Coord(0,-0), Coord(0,100), layer, metaInfo1);
l2=Line1(Coord(0,0), Coord(100,0), layer, metaInfo2);
l3=Line(Coord(0,0), Coord(50,50), layer);
d=active.document();
Builder(d):append(l1):append(l2):append(l3):execute();
