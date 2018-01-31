<p align="center">
  ﷽
</p>

![banner]

[![Build Status](https://img.shields.io/travis/muflihun/residue/master.svg)](https://travis-ci.org/muflihun/residue/branches)
[![Build Status](https://img.shields.io/travis/muflihun/residue/develop.svg)](https://travis-ci.org/muflihun/residue/branches)
[![Version](https://img.shields.io/github/release/muflihun/residue.svg)](https://github.com/muflihun/residue/releases/latest)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/MuflihunDotCom/25)

# Overview 
Residue is an effort to standardize application logging and by implementing simple logging API, easily configurable, build-in log rotators, simple way to format your log messages according to need of your application and most of all language-independent.

# Details
Logging is an essential part of any application. But not all the devices can store these logs on the same device where the application is running. For this reason, the logs must be transferred to a remote system.

Currently, there are two ways to do this:

 1. Collect log messages locally and have a scheduled task to synchronize all the log files to a remote system.
 2. Using real-time message forwarding protocols.
 
First one has many drawbacks especially when you need to debug a remote application in real-time. Also, transferring files in bulk can be heavy on network and possibly slow.

Second one is what residue client does. It delievers log messages in real-time to the (residue) server. 

Residue is fully configurable and supports high-level security, compression and asyncronous operations. It uses [Easylogging++](https://github.com/muflihun/easyloggingpp) under the hood, that itself is a feature-rich library.

You can choose to integrate similar solutions like (`rsyslog`)[https://github.com/rsyslog/rsyslog] however residue has a lot of other built-in features that other solutions may lack, like built-in log rotation, bulk message forwarding etc. 

Next section lists some of the notable features.

# Features
Some of the notable features are listed below

 * Lightweight: Residue is very light on CPU and RAM. On start-up it will only take approx 2mb of RAM and while running it will keep the memory as clean as possible using as low CPU as possible.
 * Asyncronous: Residue has been built with speed in mind right from the beginning. It runs on 10 threads on idle and an extra threads for scheduled task.
 * _Context switching_ is a concept designed as part of asyncronous processing where a *backlog* is different from *processing queue*. Once all the items from _processing queue_ are processed, the queues are swapped. This swapping is what we call *context switching*.
 * _Log rotation_ has been built in to the server that runs as scheduled task, allowing you to create backups and compress them without effecting the logging process. Every logger can be set up to have it's own rotation frequency from `hourly` to `yearly` depending on your needs.
 * _Bulk requests_ is another concept that makes logging requests process much faster and make them highly compressible to reduce traffic on the network.
 * Encryption: All the incoming and outgoing data is automatically encrypted. Residue also has several layers and techniques to secure the requests and to prevent unauthorised application from logging without compromising the speed.
 * Compression: Residue server and official client libraries provide ability to compress the network packets which makes a big difference in amount of data transferred over the wire.
 * Speed is not compromised in any way and there is a lot of testing being done every day to ensure that logging server meets the speed requirements.
 * Reliability: We are actively working on making the server much more reliable from restarting from the crash to handling bad traffic.
 * There are many more features available for this new application. Please feel free to download your copy of residue server and give it a try.
 
# Supported Platforms
Residue binaries can be run directly on the following platforms. 

Please note, you will need _libstdc++_ (C++11) runtime libraries (please refer to compiler version for the respective binary to check minimum version required)

 * Ubuntu 14.04 or higher (64-bit)
 * macOS 10.11 (El Capitan) or higher (64-bit)
 * Amazon Linux AMI 2017.03 or higher
 * Fedora 24 or higher (64-bit)
 * Fedora 19 or higher (64-bit) ([SE Only](https://github.com/muflihun/residue/blob/master/docs/INSTALL.md#special-edition))
 * CentOS 7 or higher (64-bit)
 
Other distributions that _should_ work (but have not yet been tested)
 * Oracle Linux
 * RHEL 7 or higher (64-bit)
 * Amazon Linux AMI 2016.03

# Compatibility And Integration
No matter what language is your application written in, residue is compatible and can be used as central logging server. All you need is client library. You can either write your own using [CLIENT_DEVELOPMENT.md](/docs/CLIENT_DEVELOPMENT.md) guidelines or use existing one.

C++: If your application is written in C++ you can use [CLIENT_API.md](/docs/CLIENT_API.md) to start integrating. If you're already using Easylogging++ or similar logging library (that supports `LOG` helper macros) in your application, integration can be done in no more than 2 minutes.

Java: A very massive commercial application that has multiple dependencies and modules and it was using Log4J logging library took us no more than half hour to fully replace existing library with [residue java client library](https://github.com/muflihun/residue-java).

We're also actively working on client libraries for residue in other languages. Please feel free to contribute.

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
