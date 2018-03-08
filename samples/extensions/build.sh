
# Usage: sh build.sh log-extension


## Determine type of operating system (darwin or linux)
DAR=`uname -a | grep 'Darwin' | wc -c | grep -o '[0-9]'`

if [ "$DAR" = "0" ];then
    # Linux
    g++ -fPIC -shared -std=c++11 -I/usr/local/include/residue/ -L../../build/ -lresidue-extension $1/*.cc -o $1/simple.so
else
    ## Darwin (macOS)
    g++ -dynamiclib -flat_namespace -std=c++11 -I/usr/local/include/residue/ -L../../build/ -lresidue-extension $1/*.cc -o $1/simple.dylib
fi

