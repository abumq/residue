<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# server_rsa_secret
[String] If private key is encrypted, this is the secret (passphrase) to decrypt it. **THIS SHOULD BE HEX ENCODED**

For example, if you used

```
openssl genrsa -aes256 -out server-priv.pem 8192
```

It will ask for password, let's say you provide `abc123` (don't do it, it's insecure). `server_rsa_secret` will be

```
616263313233
```

You can either use ripe to get this HEX encoded string

```
echo abc123 | ripe -e --hex
```

or [mine](https://github.com/abumq/mine#installation-cli-tool)

```
echo abc123 | mine -e --hex
```

or 

```
echo abc123 | hexdump
```

If you use hexdump, make sure you do not include last byte (it's a null-character)

## See Also
 * [`server_rsa_private_key`](/docs/CONFIGURATION.md#server_rsa_private_key)

[← Configurations](/docs/CONFIGURATION.md)

