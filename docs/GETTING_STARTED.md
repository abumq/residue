# Getting Started

This document will walk you through setup your first running server.

This is a simple setup guidelines, for details or building from source please see [INSTALL.md](/docs/INSTALL.md)

## Download Binary
[Download latest release](https://github.com/muflihun/residue/releases/latest) for your platform

```
wget https://github.com/muflihun/residue/releases/download/v1.4.5/residue-1.4.5-linux-x86_64.tar.gz
tar -xf residue-1.4.5-linux-x86_64.tar.gz
```

## Start Server
Start server using this configuration

```
cd residue-1.4.5-linux-x86_64/
sudo ./residue config/config.json
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
