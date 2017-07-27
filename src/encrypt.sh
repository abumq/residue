file=$1
key=$2
if [ "$RIPE" = "" ];then
    RIPE="ripe"
fi

if [ "$RESIDUE_SRC_KEY" = "" ];then
    echo "RESIDUE_SRC_KEY not set"
    exit;
fi

cat $file | $RIPE -e --aes --key $RESIDUE_SRC_KEY > $file.enc
