<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# known_clients ↴
## public_key
[String] Path to RSA public key file for associated client ID. This key must be present and readable at the time of starting the server.

You can use `$RESIDUE_HOME` environment variable in this file path.

All the initial key exchange is done securely using this public key.

## Generating The Key
You can generate the key using OpenSSL for either previously created private key or generate new keypair altogether. We recommend 2048-bit or 3072-bit key. Any less may fail to encrypt the server information because of it's size and any more may be slow. 2048-bit key is secure enough until 2030 and beyond that we will need 3072-bit keys [ref1](https://csrc.nist.gov/publications/detail/sp/800-57-part-1/rev-4/final)

```
openssl genrsa -out private.pem 3072
```

If you wish to generated an encrypted key, we recommend AES-256

```
openssl genrsa -aes256 -out private.pem 3072
```

### Extract Public Key
```
openssl rsa -in private.pem -outform PEM -pubout -out public.pem
```

You must upload this `public.pem` to residue server and provide path to readable uploaded file as `public_key`

## `RESIDUE_HOME`
`RESIDUE_HOME` is environment variable that is resolved at runtime if provided with in the value. E.g, if `RESIDUE_HOME` is exported to `/opt/residue/config` and you have public key `$RESIDUE_HOME/public/mykey.pem` the final file that will be read will be `/opt/residue/config/public/mykey.pem`
