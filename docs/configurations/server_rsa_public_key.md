<p align=center>
   ﷽
</p>

[🏠 Configurations](/docs/CONFIGURATION.md)

# server_rsa_public_key
[String] Corresponding public key for `server_rsa_private_key`. You can use `$RESIDUE_HOME` environment variable in this file path.

Once you have generated the private key, you can extract public key from it using following OpenSSL commands:

```
openssl rsa -in private.pem -outform PEM -pubout -out public.pem
```

## `RESIDUE_HOME`
`RESIDUE_HOME` is environment variable that is resolved at runtime if provided with, in the value. E.g, if `RESIDUE_HOME` is exported to `/opt/residue/config` and you have public key `$RESIDUE_HOME/server.pem` the final file that will be read will be `/opt/residue/config/server.pem`

## See Also
 * [`server_rsa_private_key`](/docs/CONFIGURATION.md#server_rsa_private_key)

[🏠 Configurations](/docs/CONFIGURATION.md)

