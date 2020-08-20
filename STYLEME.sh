#find libraries/cadkernel -type f -name *.cpp -o -name *.h -exec astyle 

astyle --exclude=build --exclude=lckernel/nano-signal-slot --exclude=lckernel/tinyspline --exclude=persistence/libopencad --exclude=third_party/kaguya --recursive  "*.cpp" "*.h"

find . -name "*.orig" -exec rm {} \;

#libraries/cadkernel/*.h

#find . -regex ".*\.\(cpp\|h\)"
