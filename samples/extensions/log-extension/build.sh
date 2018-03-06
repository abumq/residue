g++ -dynamiclib -std=c++11 -flat_namespace -I../../../src/ simple.cc ../../../src/extensions/extension.cc ../../../src/extensions/log-extension.cc -o simple.so -DRESIDUE_EXTENSION_LIB
