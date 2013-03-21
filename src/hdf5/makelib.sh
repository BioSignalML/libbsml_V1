g++ -dynamiclib  -o libbsml.dylib  -flat-namespace -Wl --exclude-libs,ALL /usr/local/lib/libhdf5*.a  /Users/dave/build/zlib-1.2.7/libz.a *.o
