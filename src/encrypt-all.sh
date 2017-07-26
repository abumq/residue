if [ "$1" == "" ];then
     echo "No key provided!"
     exit;
fi

find . -name "*.cc" -exec sh encrypt.sh {} $1 \;
find . -name "*.h" -exec sh encrypt.sh {} $1 \;
