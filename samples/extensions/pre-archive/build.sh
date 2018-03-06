darwin=1
if [ "$darwin" = "1" ];then
    g++ -dynamiclib -flat_namespace -std=c++11 -I../../../src/ ../../../src/extensions/extension.cc ../../../src/extensions/pre-archive-extension.cc simple.cc -o simple-prearchive.dylib -DRESIDUE_EXTENSION_LIB
else
    g++ -fPIC -shared -std=c++11 -I../../../src/ ../../../src/extensions/extension.cc ../../../src/extensions/pre-archive-extension.cc simple.cc -o simple-prearchive.so -DRESIDUE_EXTENSION_LIB
fi
