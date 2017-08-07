file=$1
newfile="${file%????}"

if [ "$RESIDUE_SRC_KEY" = "" ];then
    echo "RESIDUE_SRC_KEY not set"
    exit;
fi

result=$(cat $file | $RIPE -d --aes --key $RESIDUE_SRC_KEY --base64 | $RIPE -e --hex)

if [ -f $newfile ]; then
   orig=`cat $newfile | $RIPE -e --hex`
   if [ "$orig" != "$result" ];then
       echo "Updating $file... (orig file backedup)"
       echo "$orig" > "$newfile.bk.hex"
       echo "$result" > $newfile.hex
       cat $newfile.hex | $RIPE -d --hex > $newfile
       shasum $newfile > $newfile.chk
       cat $newfile.bk.hex | $RIPE -d --hex > $newfile.bk
       rm $newfile.hex
       rm $newfile.bk.hex
   fi
else
   echo "Updating $file..."
   echo "$result" > $newfile.hex
   cat $newfile.hex | $RIPE -d --hex > $newfile
   shasum $newfile > $newfile.chk
   rm $newfile.hex
fi

