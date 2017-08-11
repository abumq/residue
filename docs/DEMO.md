﷽

## Demo
This document runs you through to run a demo client to see how it all works.

## Start Residue Server
Once residue is (built and installed](/Docs/INSTALL.md), go to the root directory of the source and run using

```
 ./build/residue samples/residue.conf.json --verbose
```

Please take some time to understand [configuration file](/samples/residue.conf.json) and [sample files](/samples)

Now that residue is running on your computer use following commands to send logs to the server.

Log can be sent encrypted or in plain format. By default, `default` logger accepts any log messages.

## Run Demo Client
Use [netcat samples](/tools/netcat-client/) to test your logging server and to understand how it works. Please note that this sample is only for demo purposes, in real-time you will have your own RSA keys for both server and client.

Please consider reading through [DATA_FLOW](/docs/DATA_FLOW.md) to understand how client interacts with the server.

Change your working directory to the following

```
cd tools/netcat-client/
```

NOTE: Netcat sample was originally created to speed up the development. You should [check out full demo here](/demo/)

### 1. Connect
Client needs to connect to the server and obtain encryption key for secure connections.

```
sh connect.sh
```

This will create `connection.json` file which contains connection parameters required to connect to the server.

### 2. Obtain Token
You need tokens to access loggers<sup>[1]</sup>, in order to obtain token for sample logger `sample-app`, run:

```
sh token.sh
```

<sup>[1] This depends on your configuration. Please read [CONFIGURATION.md](/docs/CONFIGURATION.md) documentation for details.</sup>

### 3. Start Logging
Now that you have token you can send log requests, run:

```
sh log.sh
```

This sends plain (unencrypted) request.

Now check `/tmp/logs/sample-app.log`

## Full Demo
There is a full-cycle demo available to learn more about residue client-server interactions and security. See [`/demo`](/demo/) for more details


