#find libraries/cadkernel -type f -name *.cpp -o -name *.h -exec astyle 

astyle --exclude=nano-signal-slot  --exclude=lua-intf --options=./librecad.astylerc --recursive  "*.cpp" "*.h"

find . -name "*.orig" -exec rm {} \;

#libraries/cadkernel/*.h

#find . -regex ".*\.\(cpp\|h\)"