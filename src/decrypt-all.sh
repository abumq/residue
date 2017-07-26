if [ "$1" == "" ];then
    echo "No key provided!"
    exit;
fi
find . -name "*.enc.cc" -exec sh decrypt.sh {} $1 \;
find . -name "*.enc.h" -exec sh decrypt.sh {} $1 \;
