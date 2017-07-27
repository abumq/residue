file=$1
if [ "$RIPE" = "" ];then
    RIPE="ripe"
fi


if [ "$RESIDUE_SRC_KEY" = "" ];then
    echo "RESIDUE_SRC_KEY not set"
    exit;
fi

cat $file | $RIPE -d --aes --key $RESIDUE_SRC_KEY --base64 > "${file%????}"
