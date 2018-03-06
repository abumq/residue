
# Usage: sh build.sh log-extension

darwin=1

if [ "$darwin" = "1" ];then

    # Create libresidue-extension

    g++ -dynamiclib -flat_namespace -std=c++11 -I../../src/ -I../../deps/ ../../src/extensions/extension.cc ../../src/extensions/log-extension.cc ../../src/extensions/pre-archive-extension.cc ../../src/extensions/post-archive-extension.cc ../../src/core/json-doc.cc ../../deps/gason/gason.cc -o libresidue-extension.dylib -DRESIDUE_EXTENSION_LIB

    # Compile extension
    g++ -dynamiclib -flat_namespace -std=c++11 -I../../src/ -I../../deps/ -L./ $1/simple.cc -lresidue-extension -o $1/simple.dylib -DRESIDUE_EXTENSION_LIB

else

    # Create libresidue-extension
    g++  -fPIC -shared -std=c++11 -I../../src/ -I../../deps/ extensions/extension.cc extensions/log-extension.cc extensions/pre-archive-extension.cc extensions/post-archive-extension.cc core/json-doc.cc gason/gason.cc -o libresidue-extension.so -DRESIDUE_EXTENSION_LIB

    # Compile extension
    g++ -fPIC -shared -std=c++11 -I../../src/ -I../../deps/ -L./ $1/simple.cc -lresidue-extension -o $1/simple.so -DRESIDUE_EXTENSION_LIB
fi