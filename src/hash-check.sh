file=$1

if [ ! -f "$file.chk" ];then
    echo "Checksum not available for $file"
    exit;
fi

FILE_CHECKSUM=`shasum $file`
CURR_CHECKSUM=`cat $file.chk`

if [ "$FILE_CHECKSUM" != "$CURR_CHECKSUM" ];then
    echo "$file Changed"
fi
