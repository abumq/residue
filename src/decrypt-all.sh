if [ "$RIPE" = "" ];then
    export RIPE="ripe"
fi

echo "RIPE = $RIPE"
echo "LD_LIBRARY_PATH = $LD_LIBRARY_PATH"

$RIPE --version

find . -name "*.enc" -exec sh decrypt.sh {} \;
find . -name "*.enc" -exec sh decrypt.sh {} \;
