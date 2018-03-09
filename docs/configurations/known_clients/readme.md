<p align=center>
   ﷽
</p>

[🏠 Configurations](/docs/CONFIGURATION.md)

# known_clients
[Array] Known clients are managed clients that server already knows some information about. This speeds up connection, makes it secure and most of all helps to personalize your log files. Known clients can be rest assured that no one else can have access to their logs or their keys etc.

Setting up known clients can be very handy in many situations and is highly recommended. For example, consider following known client object

```
{
   "client_id": "muflihun.com-web-client",
   "public_key": "$RESIDUE_HOME/public/muflihun.com-web-client.pem",
   "key_size": 128,
   "loggers": [
       "muflihun.com-general", 
       "muflihun.com-security", 
       "muflihun.com-business"
   ],
   "default_logger": "muflihun.com-general",
   "user": "abumusamq"
}
```

This is a complete known client (i.e, all the necessary as well as optional fields are filled up).

1. Server will know that it's expecting a connection from client with ID `muflihun.com-web-client`
2. When the connection comes it simply has to be `connect me to muflihun.com-web-client`
3. Server will then [make connection](/docs/configurations/connect_port.md#responsibilities) and encrypt this connection using previously provided `public_key` i.e, `muflihun.com-web-client.pem`
4. Server will generate 128-bit key
5. Any logs associated to this client will automatically have ownership assigned to `abumusamq` with `RW-R-----` permissions (subject to `file_mode` config)
6. Listed loggers are expected loggers (more loggers may be allowed but if you use more loggers we recommend you to add it to this list) - if you use [resitail](https://github.com/muflihun/resitail) it will pick up loggers from this list
7. Any unknown loggers associated to this client will have configurations from `default_logger`'s configuration

## See Also
 * [`connect_port`](/docs/CONFIGURATION.md#connect_port)
 * [`default_key_size`](/docs/CONFIGURATION.md#default_key_size)
 * [`known_loggers`](/docs/CONFIGURATION.md#known_loggers)

[🏠 Configurations](/docs/CONFIGURATION.md)

