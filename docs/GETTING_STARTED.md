# Getting Started

This document will walk you through setup your first running server.

This is a simple setup guidelines, for details or building from source please see [INSTALL.md](/docs/INSTALL.md)

## Download Binary
[Download latest release](https://github.com/muflihun/residue/releases/latest) for your platform

```
wget https://github.com/muflihun/residue/releases/download/v1.4.5/residue-1.4.5-linux-x86_64.tar.gz
tar -xf residue-1.4.5-linux-x86_64.tar.gz
cd residue-1.4.5-linux-x86_64/
```

## Start Server
Start server using this configuration

```
sudo ./residue config/config.json
```

## Send Log Request
You can send log requests to local server using sample client logger

```
./sample-logger/linux/residue-logger --conf=sample-logger/conf.json
```

Source code for this sample client logger can be found @ [Residue C++ samples](https://github.com/muflihun/residue-cpp/blob/master/samples/minimal/main.cc)

## Conclusion
This is a very simple "getting started" guide. If you're facing any challenge setting your first server please feel free to create new issue on github.
