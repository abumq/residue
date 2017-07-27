file=$1
key=$2
if [ "$RIPE" = "" ];then
    RIPE="ripe"
fi
cat $file | $RIPE -e --aes --key $key > $file.enc
