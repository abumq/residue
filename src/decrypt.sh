file=$1
if [ "$RIPE" = "" ];then
    RIPE="ripe"
fi
cat $file | $RIPE -d --aes --key $2 --base64 > "${file%????}"
