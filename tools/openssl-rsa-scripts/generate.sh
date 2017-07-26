openssl genrsa -out residue-private-key.pem 2048
openssl rsa -in residue-private-key.pem -pubout -out residue-public-key.pem
