#
# This file is sample for three-way hand-shake that gets us handshake key and then we send acknowledgement to get real key and iv for future use
# 

echo "Connecting..."
# Connect to the server and retrieve encrypted symmetric key. process-handshake saves acknowledge request and the encryption key
printf "{\"type\":1,\"rsa_public_key\":\"`cat client-256-public.pem | ripe -e --base64`\"}\r\n\r\n" | nc `cat host` 8777 | ripe -d --rsa --clean --in-key client-256-private.pem --base64 | php process-connect-response.php

if [ $? -eq 1 ]; then
    exit
fi

echo "Saving connection params..."
cat connect.enc | ripe -d --key `cat tmp.key` --iv `cat iv.key` > connection.json

echo "Cleaning..."
rm iv.key tmp.key connect.base64.enc connect.enc
