file=$1

if [ "$RESIDUE_SRC_KEY" = "" ];then
    echo "RESIDUE_SRC_KEY not set"
    exit;
fi

FILE_CHECKSUM=`shasum $file`
CURR_CHECKSUM=`cat $file.chk`

if [ "$FILE_CHECKSUM" != "$CURR_CHECKSUM" ];then
    cat $file | $RIPE -e --aes --key $RESIDUE_SRC_KEY > $file.enc
    echo "$FILE_CHECKSUM" > $file.chk
fi
