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
sudo ./residue config/server.conf
```

## Send Log Request
Open another terminal and start sending log requests using sample client logger

```
wget https://github.com/muflihun/muflihun.github.io/raw/master/downloads/sample-logger.tar.gz
tar -xf sample-logger.tar.gz
./sample-logger/linux/residue-logger --conf=sample-logger/conf.json
```

This sample sends using `default` logger. Source code for this sample client logger can be found @ [Residue C++ samples](https://github.com/muflihun/residue-cpp/blob/master/samples/minimal/main.cc)

## View Logs
You can tail your logs using

```
tail -f /tmp/logs/residue.log
```

## Conclusion
This is a very simple _getting started_ guide. If you're facing any challenge setting your first server please feel free to create new issue on github.
