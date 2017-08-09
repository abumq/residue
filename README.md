                                       ‫بسم الله الرَّحْمَنِ الرَّحِيمِ

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
Residue takes logging to the next level. It allows application to send log messages to the remote server in real-time. This new design helps developers find more details on health of the application without asking user about it. It is very useful in mobile/tablet apps where you cannot store much logs on the device itself.

Residue logging server is fully configurable and supports encryption, compression and asyncronous operations for speed and security. Residue uses [Easylogging++](https://github.com/muflihun/easyloggingpp) under the hood, that itself is a feature-rich library. Server admin can provide access to their clients to update their loggers (configuration, access codes etc)

[![Residue Logging Server - Quick Demo](https://muflihun.github.io/residue/assets/yt-thumb.png)](https://www.youtube.com/watch?v=1r4zAkLIyOA)

# Security
Security has been taken in to consideration to prevent unauthorised applications writing logs. This is done using various technologies and techniques, that include:

 - Secure connections
 - Access codes to use logger(s)
 - Tokenized authentication
 
Residue is fully configurable and various security levels can be enabled/disabled depending on your requirement. See [SECURITY.md](/docs/SECURITY.md) for details

# Speed
Speed cannot be compromised with logging libraries and servers. All the requests are processed in background worker, providing server a lot of space to deal with incoming requests.

In addition to that, we use `single threaded event-driven approach` for new request (as opposed to `multi-threaded non-blocking worker` design) to keep memory low. This is because threads are expensive. This decision was carefully made after immense research and looking at all the requirements.

# Compatibility And Integration
No matter what language is your application written in, residue is compatible and can be used as central logging server. All you need is client library. You can either write your own using [CLIENT_DEVELOPMENT.md](/docs/CLIENT_DEVELOPMENT.md) guidelines or use existing one.

However, if your application is written in C++ you can use [CLIENT_API.md](/docs/CLIENT_API.md) to start integrating. If you're already using Easylogging++ or similar logging library (that supports `LOG` helper macros) in your application, integration can be done in no more than 2 minutes.

We're also actively working on client library for residue in other languages. Please feel free to contribute.

# License
Copyright (c) 2017 Muflihun Labs

You must obtain a license from authorised person in order to host residue server.

  [banner]: https://raw.githubusercontent.com/muflihun/residue/master/docs/Residue.png
