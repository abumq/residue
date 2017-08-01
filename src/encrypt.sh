file=$1
key=$2

if [ "$RESIDUE_SRC_KEY" = "" ];then
    echo "RESIDUE_SRC_KEY not set"
    exit;
fi

RESIDUE_SRC_IV=f1002847d4c7c8a714a765f3fef232eb

cat $file | $RIPE -e --aes --key $RESIDUE_SRC_KEY --iv $RESIDUE_SRC_IV > $file.enc
