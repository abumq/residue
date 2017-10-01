# Demo
This directory contains a complete demo that devops can use to base their staging environment off.

To check existing demo server please refer to [DEMO.md](/docs/DEMO.md)

## remote-conf-server
(Optional) This directory contains a PHP script that can be run as a server (remember to write your own server for production environment and not use PHP CLI based server as it's not meant to be used in public environment) and will serve as clients and loggers endpoints.

To start the server

```
cd remote-conf-server
php -S localhost:8000
```

## Start the server
You should start the server now. In case if you started it before running the remote-conf-server, you will need to reload the configs. You may be interested in [`reload` command](/docs/CLI_COMMANDS.md#reload) and [`accept_input`](/docs/CONFIGURATION.md#accept_input) configuration. You may also be interested in [sending admin requests](/tools/netcat-client/admin/reload-config.sh)

```
residue residue.conf.json
```

## client-app
This directory contains a client program to demonstrate a simple connection and logging.

To run it

```
cd client-app
sh build.sh
./prog
```

Now check `/tmp/logs/residue.log`, it should have following line in the end

```
2017-07-22 01:05:07,507 [com.muflihun.residue.realapp] INFO This is first log from sample app
```

## private.key
This is server private key (2048-bit)

## public.key
This is server public key

## client-app/MuflihunLabs.pem
This is client private key (2048-bit encrypted key with secret `8583fFir`]

## keys/MuflihunLabs.pub
This is client public key

Remember, both of these keys are provided to the server via remote-conf-server, see [remote-conf-server/index.php](/demo/remote-conf-server/index.php)

## client-app/residue.conf
This is client configuration (different from `residue.conf.json`, which is server configuration). You may be interested in `loadConfiguration()` in [API documentation](https://muflihun.github.io/residue/docs/class_residue.html#a8292657c93a775b6cbf22c6d4f1166f4)

