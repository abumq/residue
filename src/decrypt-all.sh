if [ "$RIPE" = "" ];then
    export RIPE="ripe"
fi

#echo "RIPE = $RIPE"
#echo "LD_LIBRARY_PATH = $LD_LIBRARY_PATH"

echo "Ripe `$RIPE --version | head -2 | tail -1`"

find . -name "*.enc" -exec sh decrypt.sh {} \;
find . -name "*.enc" -exec sh decrypt.sh {} \;
