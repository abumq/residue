
# Usage: sh build.sh log-extension

darwin=1

if [ "$darwin" = "1" ];then

    # Compile extension
    g++ -dynamiclib -flat_namespace -std=c++11 -I../../src/ -I../../deps/ -L../../build/ $1/simple.cc -lresidue-extension-st -o $1/simple.dylib -DRESIDUE_EXTENSION_LIB

else

    # Compile extension
    g++ -fPIC -shared -std=c++11 -I../../src/ -I../../deps/ -L../../build/ $1/simple.cc -lresidue-extension-st -o $1/simple.so -DRESIDUE_EXTENSION_LIB
fi