
# Contributing

## Server
Residue server is actively being developed. We use C++ as our base language for the server. Scripting may be done in language or your choice but we prefer PHP and/or BASH. Development of the server includes development of [helper tools](/tools/).

Server is currently tested on following OS:

 * Linux (Ubuntu 64-bit, Fedora 64-bit)
 * macOS (El Capitan, Sierra)
 
It may be compilable on other operating systems but we have not tested it yet. Please feel free to compile it on other operating systems using various compilers, make PR requests and let us know if it compiles just fine. We will add it to this list. Our aimed operating systems are any major OS that can be turned in to a server, e.g, Android, Raspberry PI, Windows.

## Client
Clients are essentially libraries that help other developers to send requests to the residue server without exposing too much details to them. For details, please refer to [CLIENT_DEVELOPMENT.md](/docs/CLIENT_DEVELOPMENT.md).

If you wish to use residue for your C++ application, please take a look at [this sample](/samples/clients/c++) and [CLIENT_API.md](/docs/CLIENT_API.md) for details. This uses official client library for residue that is asyncronous, thread-safe and fast!

Similar to the server, we have tested official client libraries on the following OS:

 * Linux (Ubuntu 64-bit, Fedora 64-bit)
 * macOS (El Capitan, Sierra)
 
Please let us know if you successfully compile it on other operating systems.

## Submitting Patches 
Please feel free to submit your pull requests. Please read [development](#development) section before you start.

## Reporting Bugs
Please submit bug reports via [issue tracker](https://github.com/muflihun/residue/issues). We strongly recommend you to test this project in as many platforms as possible that can be turned in to logging server.

Please provide as much information as possible including steps to reproduce. This will help contributors to fix issue as soon as possible.

## Development
The `develop` branch contains the latest development codebase. This is where bug fixes and improvments should go. `master` branch is only for released versions.

For each new pull request you will create a feature branch that should be created off `develop` branch.

Please refer to [INSTALL.md](/docs/INSTALL.md) documentation for instructions to build / install Residue on your machine.
