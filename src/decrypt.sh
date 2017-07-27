file=$1
newfile="${file%????}"

if [ "$RIPE" = "" ];then
    RIPE="ripe"
fi


if [ "$RESIDUE_SRC_KEY" = "" ];then
    echo "RESIDUE_SRC_KEY not set"
    exit;
fi

result=$(cat $file | $RIPE -d --aes --key $RESIDUE_SRC_KEY --base64 | $RIPE -e --hex)

if [ -f $newfile ]; then
   orig=`cat $newfile | $RIPE -e --hex`
   if [ "$orig" != "$result" ];then
       echo "$file contains local changes, ignoring..."
       exit;
       #echo "$orig" > "$newfile.bk"
       #echo "$result" > $newfile.hex
       #cat $newfile.hex | $RIPE -d --hex > $newfile
       #rm $newfile.hex
   fi
else
   echo "Updating $file..."
   echo "$result" > $newfile.hex
   cat $newfile.hex | $RIPE -d --hex > $newfile
   rm $newfile.hex
fi

