file=$1
newfile="${file%????}"

if [ "$RIPE" = "" ];then
    RIPE="ripe"
fi


if [ "$RESIDUE_SRC_KEY" = "" ];then
    echo "RESIDUE_SRC_KEY not set"
    exit;
fi

result=$(cat $file | $RIPE -d --aes --key $RESIDUE_SRC_KEY --base64 | ripe -e --hex)

if [ -f $newfile ]; then
   orig=`cat $newfile | ripe -e --hex`
   if [ "$orig" = "$result" ];then
       echo "Ignoring for same file"
   else
       echo "$orig" > "$newfile.bk"
       echo "$result" > $newfile.hex
       cat $newfile.hex | ripe -d --hex > $newfile
       rm $newfile.hex
   fi
else
   echo "$result" > $newfile.hex
   cat $newfile.hex | ripe -d --hex > $newfile
   rm $newfile.hex
fi

