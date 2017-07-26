file=$1
cat $file | ripe -d --aes --key $2 --base64 > "${file%????}"
