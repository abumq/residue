<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# server_rsa_private_key
[Optional, String] RSA private key (PEM format file path). You can use `$RESIDUE_HOME` environment variable in this file path. If provided, it is used to read initial requests for extra security.

## Generating The Key
You can generate the key using OpenSSL for either previously created private key or generate new keypair altogether. We recommend 8192-bit because the information that is transferred can be bit. 8192-bit key can encrypt 1013 bytes to data.

```
openssl genrsa -out server-priv.pem 8192
```

If you wish to generated an encrypted key, we recommend AES-256

```
openssl genrsa -aes256 -out server-priv.pem 8192
```

Alternatively, you can use following [Ripe](https://github.com/muflihun/ripe#readme) command

```
ripe -g --rsa --length 8192 --out-public server-priv.pem --out-private server-priv.pem
```

## Note
You should have big enough key to cover for unknown clients. Remember, unknown clients will need to send their public key in initial request, which makes request quite bigger.

## See Also
 * [`server_rsa_public_key`](/docs/CONFIGURATION.md#server_rsa_public_key)
 * [`server_rsa_secret`](/docs/CONFIGURATION.md#server_rsa_secret)
 * [`RESIDUE_HOME`](https://github.com/muflihun/residue/blob/develop/docs/INSTALL.md#residue_home)

[← Configurations](/docs/CONFIGURATION.md)

