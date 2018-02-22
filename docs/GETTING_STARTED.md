# Getting Started

This document will walk you through setup your first running server.

This is a simple setup guidelines, for details or building from source please see [INSTALL.md](/docs/INSTALL.md)

## Download Binary
[Download latest release](https://github.com/muflihun/residue/releases/latest) for your platform

```
wget https://github.com/muflihun/residue/releases/download/v1.4.4/residue-1.4.4-linux-x86_64.tar.gz
tar -xf residue-1.4.4-linux-x86_64.tar.gz
```
 
## Configure

### Loggers' Config

default-logger.conf
```
* GLOBAL:
    FORMAT                  =   "%datetime [%logger] [%app] [%thread] %level %msg"
    FILENAME                =   "/tmp/logs/default.log"
    ENABLED                 =   true
* VERBOSE:
    FORMAT                  =   "%datetime [%logger] [%app] [%thread] %level-%vlevel %msg"
```

residue-logger.conf
```
* GLOBAL:
    FORMAT                  =   "%datetime [%logger] [%app] [%thread] %level %msg"
    FILENAME                =   "/tmp/logs/residue.log"
    ENABLED                 =   true
* VERBOSE:
    FORMAT                  =   "%datetime [%logger] [%app] [%thread] %level-%vlevel %msg"
* DEBUG:
    ENABLED                 =   false
```

### Server Config
Write configuration file either using [Create Config Tool](https://muflihun.github.io/residue/create-server-config) or by copying basic configuration file

server.conf
```
{
    "admin_port": 8776,
    "connect_port": 8777,
    "token_port": 8778,
    "logging_port": 8779,
    "allow_default_access_code": true,
    "immediate_flush": false,
    "token_age": 900,
    "client_age": 3600,
    "file_mode": 416,
    "timestamp_validity": 120,
    "requires_timestamp": true,
    "non_acknowledged_client_age": 300,
    "client_integrity_task_interval": 300,
    "archived_log_directory": "%original/backups/%logger/",
    "archived_log_filename": "%level-%hour-%min-%day-%month-%year.log",
    "archived_log_compressed_filename": "%hour-%min-%day-%month-%year.tar.gz",
    "known_clients": [
    ],
    "known_loggers": [
        {
            "logger_id": "residue",
            "configuration_file": "residue-logger.conf",
        },
        {
            "logger_id": "default",
            "configuration_file": "default-logger.conf",
        }
    ]
}
```

## Start Server
Start server using this configuration

```
sudo ./residue-1.4.4-linux-x86_64/residue server.conf
```

## Send Log Request
Your server is now accepting log requests. Try sending simple log request using [netcat demo client](https://github.com/muflihun/residue/blob/master/tools/netcat-client). You will need to install [`ripe`](https://github.com/muflihun/ripe/releases/latest) for this demo. Alternatively, you can try [client libraries' samples](https://github.com/muflihun/residue-node/tree/master/samples/cli)

```
cd tools/netcat-client
sh connect.sh
sh token.sh
sh log.sh
```

## Conclusion
This is a very simple "getting started" guide. If you're facing any challenge setting your first server please feel free to create new issue on github.
