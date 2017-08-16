#!/bin/bash

##########################
###
### SCM SCRIPT FOR RESIDUE
###
##########################

if [ "$RIPE" = "" ];then
    export RIPE="ripe"
fi

if [ "$RESIDUE_SRC_KEY" = "" ];then
    echo "RESIDUE_SRC_KEY not set"
    exit;
fi


function decr {
    file=$1
    newfile="${file%????}"

    result=$(cat $file | $RIPE -d --aes --key $RESIDUE_SRC_KEY --base64 | $RIPE -e --hex)

    if [ -f $newfile ]; then
       orig=`cat $newfile | $RIPE -e --hex`
       if [ "$orig" != "$result" ];then
           echo "Updating $file... (orig file backedup)"
           echo "$orig" > $newfile.bk.hex
           echo "$result" > $newfile.hex
           cat $newfile.hex | $RIPE -d --hex > $newfile
           cat $newfile.bk.hex | $RIPE -d --hex > $newfile.bk
           rm $newfile.hex
           rm $newfile.bk.hex
       fi
    else
       echo "Updating $file..."
       echo "$result" > $newfile.hex
       cat $newfile.hex | $RIPE -d --hex > $newfile
       rm $newfile.hex
    fi
}

function encr {
    file=$1

    FILE_CHECKSUM=`shasum $file | head -n1 | awk '{print $1;}'`
    CURR_CHECKSUM=`cat $file.chk`

    if [ "$FILE_CHECKSUM" != "$CURR_CHECKSUM" ];then
        echo "Change detected: $file"
        cat $file | $RIPE -e --aes --key $RESIDUE_SRC_KEY > $file.enc
        echo "$FILE_CHECKSUM" > $file.chk
    fi
}

function hash_check {
    file=$1

    if [ ! -f "$file.chk" ];then
        echo "Checksum not available for $file"
        exit;
    fi

    FILE_CHECKSUM=`shasum $file | head -n1 | awk '{print $1;}'`
    CURR_CHECKSUM=`cat $file.chk`

    if [ "$FILE_CHECKSUM" != "$CURR_CHECKSUM" ];then
        echo "$file Changed"
    fi
    
}

function decr_all {
    echo "Ripe `$RIPE --version | head -2 | tail -1`"
    find src -name "*.enc" -exec bash -c 'decr "$0"' {} \;
}

function encr_all {
    echo "Ripe `$RIPE --version | head -2 | tail -1`"
    find src -name "*.cc" -exec bash -c 'encr "$0"' {} \;
    find src -name "*.h" -exec bash -c 'encr "$0"' {} \;
}

function hash_check_all {
    find src -name "*.cc" -exec bash -c 'hash_check "$0"' {} \;
    find src -name "*.h" -exec bash -c 'hash_check "$0"' {} \;
}

function clean_all {
    find src -name "*.cc" -exec rm -f {} \;
    find src -name "*.h" -exec rm -f {} \;
}

export -f decr
export -f encr
export -f hash_check

case "$1" in
        d)
            decr_all
            ;;
        df)
            decr $2
            ;;
         
        e)
            encr_all
            ;;
        ef)
            encr $2
            ;;
        c)
            hash_check_all
            ;;
        cf)
            hash_check $2
            ;;
         clean)
            clean_all
            ;;
         status)
             git status
             echo "Checking source..."
             $0 c
             ;;
        *)
             git $@
esac
