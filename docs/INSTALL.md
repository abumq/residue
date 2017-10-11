<p align="center">
   ﷽
</p>

# Overview
Residue can be installed either by building from the source or by downloading the binaries from respective releases.

# Download Binary
You can download binary from [releases](https://github.com/muflihun/residue/releases) page for your platform. They are standalone binaries but they require some external libraries installed that are mentioned with each release notes.

## Special Edition
Linux binaries come in two flavours, standard and special edition. Special edition is exactly same as standard edition except that it's built with older compiler, hence chances of it running on older systems is higher than that of standard edition.

We always recommend users to try standard edition first and if that does not work only then try special edition.

# Building From Source
This document shows you steps to install residue server on your machine. You can install it for development or for production use.

## Dependencies
  * C++11 (or higher)
  * Boost v1.53 or higher [Components: [system](http://www.boost.org/doc/libs/1_62_0/libs/system/doc/index.html)]
  * [Easylogging++](https://github.com/muflihun/easyloggingpp) v9.95.0
  * [Crypto++](https://www.cryptopp.com/) v5.6.5+ [with Pem Pack](https://raw.githubusercontent.com/muflihun/muflihun.github.io/master/downloads/pem_pack.zip)
  * [CMake Toolchains](https://cmake.org/) v2.8.12
  * [zlib-devel](https://zlib.net/)
  * [libcurl-devel](https://curl.haxx.se/libcurl/)
  * [Python](https://www.python.org) v2.7+ (optional, with extensions)
  * [Google Testing Framework](https://github.com/google/googletest/blob/master/googletest/docs/Primer.md)
  
## Get Code
You can either [download code from master branch](https://github.com/muflihun/residue/archive/master.zip) or clone it using `git`:

```
git clone git@github.com:muflihun/residue.git
```

## Build
Residue uses the CMake toolchains to generate makefiles.
Steps to build Residue:

```
mkdir build
cd build
cmake -Dtest=ON ..
make
```

You can define following options in CMake (using `-D<option>=ON`)

|    Option    | Description                     |
| ------------ | ------------------------------- |
| `test`       | Compile unit tests              |
| `debug`      | Turn on debug logging           |
| `production` | Compile for production use      |
| `profiling`  | Turn on profiling information for making server faster (goes together with `debug`) |
| `enable_extensions` | Enable extensions support for the build. Extensions require python |
| `use_mine` | Use mine crypto library (instead of ripe) whereever possible |

Please consider running unit tests before you move on.

```
make test
```

The compilation process creates executable `residue` in build directory. You can install it in system-wide directory using:

```
make install
```

If the default path (`/usr/local`) is not where you want things installed, then set the `CMAKE_INSTALL_PREFIX` option when running cmake. e.g,

```
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/bin
```

## Install Script
Make sure you have all the dependencies installed. You can use following script to install it all and then go back to [Build](#build) section (tested on Ubuntu 14.04 (Trusty) 64-bit)

```
## Essentials
sudo apt-get install -y cmake build-essential libcurl-dev libz-dev
    # sudo yum install -y cmake curl-devel zlib-devel # for rpm
    # sudo yum groupinstall -y 'Development Tools'

## Boost System
sudo apt-get install -y libboost-system-dev cmake
    # sudo yum install -y boost-devel boost-devel-static # for rpm

## Google Testing Library
wget -O gtest.tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz
tar xf gtest.tar.gz
cd googletest-release-1.7.0
cmake -DBUILD_SHARED_LIBS=ON .
make
sudo cp -a include/gtest /usr/include
sudo cp -a libgtest_main.so libgtest.so /usr/lib/
cd ..

## Python (Optional with extensions)
wget https://www.python.org/ftp/python/3.6.3/Python-3.6.3.tgz
tar xf Python-3.6.3.tgz
cd Python-3.6.3
./configure BASECFLAGS="-static" ## Do not use --enable-optimizations with static
##> If above fails, try without BASECFLAGS="-static" and manuall change Makefile and append -static to BASECFLAGS
make
sudo make install

## Easylogging++
wget -O elpp-master.zip https://github.com/muflihun/easyloggingpp/archive/master.zip
unzip elpp-master.zip
cd easyloggingpp-master
cmake .
make
sudo make install

## Crypto++
wget https://raw.githubusercontent.com/muflihun/muflihun.github.io/master/downloads/cryptocpp.tar.gz
tar xf cryptocpp.tar.gz
cd cryptopp-CRYPTOPP_5_6_5
wget https://raw.githubusercontent.com/muflihun/muflihun.github.io/master/downloads/pem_pack.zip
unzip pem_pack.zip
cmake .
make
sudo make install
```

# Run as `root`
You will need to run residue as root user. This is because residue needs to change the ownership of the files to the relevant users and yet need to write to those files.

If you are just need to test the residue before you run it in production (in production you should always run it as root) you can use `--force-without-root` command-line argument.

# What's Next?
You can run demo to see how residue works. Please refer to [DEMO.md](/docs/DEMO.md) documentation
