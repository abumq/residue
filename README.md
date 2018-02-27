<p align="center">
  ﷽
</p>

![banner]

[![Build Status](https://img.shields.io/travis/muflihun/residue/master.svg)](#build-matrix)
[![Build Status](https://img.shields.io/travis/muflihun/residue/develop.svg)](#build-matrix)
[![Version](https://img.shields.io/github/release/muflihun/residue.svg)](https://github.com/muflihun/residue/releases/latest)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/MuflihunDotCom/25)

# Overview 
Residue is an effort to standardize application logging by implementing simple logging API, making it easily configurable, build-in configurable log rotators, simple way to format your log messages according to need of your application and most of all language-independent.

![overview]

# Details
Logging is an essential part of any application. But not all the applications can store these logs on the same device where they're running. For this reason, the logs must be transferred to a remote system.

Currently, there are two ways to do this:

 1. Collect log messages locally and have a scheduled task to synchronize all the log files to a remote system.
 2. Using real-time message forwarding protocols.
 
First one has many drawbacks especially when you need to debug a remote application in real-time. Also, transferring files in bulk can be heavy on network and possibly slow.

Second one is what residue client does. It delievers log messages in real-time to the (residue) server. 

Residue is fully configurable and supports high-level security, compression and asyncronous operations. It uses [Easylogging++](https://github.com/muflihun/easyloggingpp) under the hood, that itself is a feature-rich library.

You can choose to integrate similar solutions like [`rsyslog`](https://github.com/rsyslog/rsyslog) however residue has a lot of other built-in features that other solutions may lack, like built-in log rotation, bulk message forwarding etc. See [Features](#features) section for details. 

# Features
This above diagram in [Overview](#overview) section only shows a very high-level picture of what residue server does. There are details around each area for example:

 * `Send Log` is sent as JSON packet, residue takes care of formatting it, processing the batch of requests (aka bulk request)
 * `Scheduled Backups` can be configured so that residue can act as auto-log-rotator, for example you can [set the format](/docs/CONFIGURATION.md#archived_log_compressed_filename) in a way that logs get rotated weekly and each day in that week has it's own backup.
 
Some of the notable features are listed below

 * Lightweight: Residue is very light on CPU and RAM. On start-up it will only take approx 2mb of RAM and while running it will keep the memory as clean as possible using as low CPU as possible.
 * Asyncronous: Residue has been built with speed in mind right from the beginning. It implements various techniques to speed up the process.
 * _Log rotation_ has been built in to the server that runs as scheduled task, allowing you to create backups and compress them without effecting the logging process. Every logger can be set up to have it's own rotation frequency from `hourly` to `yearly` depending on your needs.
 * _Bulk requests_ is another concept that makes logging requests process much faster and make them highly compressible to reduce traffic on the network.
 * Encryption: All the incoming and outgoing data is automatically encrypted. Residue also has several layers and techniques to secure the requests and to prevent unauthorised application from logging without compromising the speed.
 * Compression: Residue server and official client libraries provide ability to compress the network packets which makes a big difference in amount of data transferred over the wire.
 * Speed is not compromised in any way and there is a lot of testing being done every day to ensure that logging server meets the speed requirements.
 * Reliability: We are actively working on making the server much more reliable from restarting from the crash to handling bad traffic.
 * Residue comes with various [tools](https://github.com/topics/residue-tools) for devops and [client libraries](https://github.com/topics/residue-client) for easy integration.
 * There are many more features available. Please feel free to download your copy of residue binary and give it a try.

# Getting Started
## Download Binary
[Download latest release](https://github.com/muflihun/residue/releases/latest) for your platform

```
wget https://github.com/muflihun/residue/releases/download/v1.6.0/residue-1.6.0-linux-x86_64.tar.gz
tar -xf residue-1.6.0-linux-x86_64.tar.gz
cd residue-1.6.0-linux-x86_64/
```

## Start Server
Start server using this configuration

```
sudo ./residue config/server.conf
```

## Send Logs
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
# Supported Platforms
Residue binaries are tested on the following platforms. 

 * Ubuntu 14.04 or higher (64-bit)
 * macOS 10.11 (El Capitan) or higher (64-bit)
 * Amazon Linux AMI 2017.03 or higher
 * Fedora 24 or higher (64-bit)
 * Fedora 19 or higher (64-bit)
 * CentOS 7 or higher (64-bit)
 
Other distributions that _should_ work (but have not yet been tested)
 * Oracle Linux
 * RHEL 7 or higher (64-bit)
 * Amazon Linux AMI 2016.03

# Compatibility And Integration
No matter what language is your application written in, residue is compatible and can be used as central logging server. All you need is client library. You can either write your own using [CLIENT_DEVELOPMENT.md](/docs/CLIENT_DEVELOPMENT.md) guidelines or use [one of the existing ones](https://github.com/search?q=topic%3Aresidue-client+org%3Amuflihun&type=Repositories).

We're also actively working on client libraries for residue in other languages. Please feel free to [contribute](/CONTRIBUTING.md).

# Build Matrix

| Branch | Platform | Build Status |
| -------- |:------------:|:------------:|
| `develop` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `clang++` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/develop/1)](https://travis-ci.org/muflihun/residue) |
| `develop` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `g++-4.9` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/develop/2)](https://travis-ci.org/muflihun/residue) |
| `develop` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `g++-5` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/develop/3)](https://travis-ci.org/muflihun/residue) |
| `develop` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `g++-6` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/develop/4)](https://travis-ci.org/muflihun/residue) |
| `develop` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `g++-7` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/develop/5)](https://travis-ci.org/muflihun/residue) |
| `master` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `clang++` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/master/1)](https://travis-ci.org/muflihun/residue) |
| `master` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `g++-4.9` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/master/2)](https://travis-ci.org/muflihun/residue) |
| `master` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `g++-5` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/master/3)](https://travis-ci.org/muflihun/residue) |
| `master` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `g++-6` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/master/4)](https://travis-ci.org/muflihun/residue) |
| `master` | GNU/Linux 4.4 / Ubuntu 4.8.4 64-bit / `g++-7` | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/muflihun/residue/branches/master/5)](https://travis-ci.org/muflihun/residue) |

# License
Please see [LICENSE](/LICENSE) for licensing information.

  [banner]: https://raw.githubusercontent.com/muflihun/residue/master/docs/Residue.png
  [overview]: https://raw.githubusercontent.com/muflihun/residue/develop/docs/residue-overview.png?v2
