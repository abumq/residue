﷽

![banner]

[![Build Status](https://img.shields.io/travis/muflihun/residue/master.svg)](https://travis-ci.org/muflihun/residue) (`master`)

[![Build Status](https://img.shields.io/travis/muflihun/residue/develop.svg)](https://travis-ci.org/muflihun/residue) (`develop`)

[![Version](https://img.shields.io/github/release/muflihun/residue.svg)](https://github.com/muflihun/residue/releases/latest)

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/MuflihunDotCom/25)

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

# Introduction
Residue is a real-time remote logging server that is secure, efficient with memory and CPU. It allows application to send log messages in real-time. This new design helps developers find more details on health of their applications. It is very useful in mobile/tablet apps where you cannot store much logs on the device itself.

Residue logging server is fully configurable and supports high-level security, compression and asyncronous operations for speed and security. It uses [Easylogging++](https://github.com/muflihun/easyloggingpp) under the hood, that itself is a feature-rich library.

# Features
Residue is feature-rich, highly configurable server that is light on CPU and RAM. Some of the notable features are listed below:

## Real-time logging
Real-time logging is a core feature of Residue. You can send log requests in real-time and expect them to be available on the server within few seconds.

## Log Rotation
Fully configurable log rotation is supported by Residue with custom filenames (using patterns) and rotation frequency

## Bulk Requests
Bulk requests makes logging requests faster to process and low on network and highly compressable.

## Secure Connections
Security has been taken in to consideration to prevent unauthorised applications writing logs. This is done using various technologies and techniques, that include:

 - Secure connections
 - Access codes to use logger(s)
 - Tokenized authentication
 
Residue is fully configurable and various security levels can be enabled/disabled depending on your requirement. See [SECURITY.md](/docs/SECURITY.md) for details

## Compression
Residue server and client libraries provide ability to compress the network packets which makes a big difference in amount of data transferred over the wire.

## Speed
Speed cannot be compromised with logging libraries and servers. All the requests are processed in background worker, providing server a lot of space to deal with incoming requests.

In addition to that, we use `single threaded event-driven approach` for new request (as opposed to `multi-threaded non-blocking worker` design) to keep memory low. This is because threads are expensive. This decision was carefully made after immense research and looking at all the requirements.

## Compatibility And Integration
No matter what language is your application written in, residue is compatible and can be used as central logging server. All you need is client library. You can either write your own using [CLIENT_DEVELOPMENT.md](/docs/CLIENT_DEVELOPMENT.md) guidelines or use existing one.

If your application is written in C++ you can use [CLIENT_API.md](/docs/CLIENT_API.md) to start integrating. If you're already using Easylogging++ or similar logging library (that supports `LOG` helper macros) in your application, integration can be done in no more than 2 minutes.

A very massive commercial application that has multiple dependencies and modules and it was using Log4J logging library took us no more than half hour to fully replace existing library with residue java client library.

We're also actively working on client library for residue in other languages. Please feel free to contribute.

# License
Please see [LICENSE.md](/LICENSE.md) for licensing. You are free to share the binaries however the residue server license must obtained from an authorised authority in order to host the server.

  [banner]: https://raw.githubusercontent.com/muflihun/residue/master/docs/Residue.png
