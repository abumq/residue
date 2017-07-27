if [ "$1" == "" ];then
    echo "No key provided!"
    exit;
fi
find . -name "*.enc" -exec sh decrypt.sh {} $1 \;
find . -name "*.enc" -exec sh decrypt.sh {} $1 \;
