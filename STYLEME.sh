#find libraries/cadkernel -type f -name *.cpp -o -name *.h -exec astyle 

astyle --options=./librecad.astylerc --recursive  "*.cpp" "*.h"

#libraries/cadkernel/*.h

#find . -regex ".*\.\(cpp\|h\)"