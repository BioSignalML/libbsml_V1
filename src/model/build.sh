APP=test
#
SOURCES="object.cpp recording.cpp signal.cpp clock.cpp units.cpp datetime.cpp rdf.cpp utility.cpp"
LIBRARIES="-lrdf -lboost_date_time"
#
OPTIONS="-g2"
COMPILER="c++ --std=c++11 --stdlib=libc++"
#
BUILD="$COMPILER $OPTIONS $SOURCES $APP.cpp $LIBRARIES -o $APP"
echo $BUILD
$BUILD
