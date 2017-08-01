if [ "$RIPE" = "" ];then
    export RIPE="ripe"
fi
echo "RIPE = $RIPE"
echo "LD_LIBRARY_PATH = $LD_LIBRARY_PATH"
find . -name "*.cc" -exec sh encrypt.sh {} \;
find . -name "*.h" -exec sh encrypt.sh {} \;
