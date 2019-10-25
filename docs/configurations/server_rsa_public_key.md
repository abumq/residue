<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# server_rsa_public_key
[String] Corresponding public key for `server_rsa_private_key`. You can use `$RESIDUE_HOME` environment variable in this file path.

Once you have generated the private key, you can extract public key from it using following OpenSSL commands:

```
openssl rsa -in server-priv.pem -outform PEM -pubout -out server.pem
```

## See Also
 * [`server_rsa_private_key`](/docs/CONFIGURATION.md#server_rsa_private_key)
 * [`RESIDUE_HOME`](https://github.com/amrayn/residue/blob/develop/docs/INSTALL.md#residue_home)

[← Configurations](/docs/CONFIGURATION.md)

