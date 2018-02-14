<p align="center">
   ﷽
</p>

# Overview
Residue can be installed either by building from the source or by downloading the binaries from respective releases.

# Download Binary
You can download binary from [releases](https://github.com/muflihun/residue/releases) page for your platform. They are standalone binaries but they require some external libraries installed that are mentioned with each release notes. Most of these external libraries come with respective operating system distributions.

## Special Edition
Linux binaries come in two flavours, standard and special edition. Special edition is exactly same as standard edition except that it's built with older compiler, hence chances of it running on older systems is higher than that of standard edition.

**We always recommend users to try standard edition first and if that does not work only then try special edition.**

# Building From Source
You can follow steps below to build and install residue server on your machine.

## Dependencies
  * C++11 compiler (or higher)
  * [Crypto++](https://www.cryptopp.com/) v5.6.5+ [with Pem Pack](https://raw.githubusercontent.com/muflihun/muflihun.github.io/master/downloads/pem_pack.zip)
  * [zlib-devel](https://zlib.net/)
  * [libcurl-devel](https://curl.haxx.se/libcurl/)
  * [Google Testing Framework](https://github.com/google/googletest/blob/master/googletest/docs/Primer.md) (optional - for testing)
  
## Get The Code
You can either [download code from master branch](https://github.com/muflihun/residue/archive/master.zip) or clone it using `git`:

```
git clone git@github.com:muflihun/residue.git
```

## Build
Residue uses the CMake toolchains to generate makefiles.

In a nut shell, you will do:

```
mkdir build
cd build
cmake -Dtest=ON ..
make
```

## CMake Options
You can change following options in CMake (using `-D<option>=ON`)

|    Option    | Description                     | Default |
| ------------ | ------------------------------- |---------|
| `test`       | Compile unit tests (require gtest)              | `OFF`   |
| `debug`      | Turn on extra logging           | `OFF`   |
| `production` | Compile for production use      | `ON`   |
| `profiling`  | Turn on profiling information (for development only - must have `debug` option turned on) | `OFF` |
| `use_boost` | Link against boost library instead of standalone asio (must have boost 1.54-static installed) | `OFF` |

## Run Tests
Please consider running unit tests before you move on.

```
make test
```

## Install
The compilation process creates executable `residue` in the build directory. You can install it in system-wide directory using:

```
make install
```

If the default path (`/usr/local/bin`) is not where you want things installed (which is not recommended), then set the `CMAKE_INSTALL_PREFIX` option when running cmake. e.g,

```
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/bin
```

## Setup
Make sure you have all the dependencies installed. You can use following script to install it all and then go back to [Build](#build) section (tested on Ubuntu 14.04 (Trusty) 64-bit)

```
## Essentials
sudo apt-get install -y cmake build-essential libcurl-dev libz-dev
    # sudo yum install -y cmake curl-devel zlib-devel # for rpm
    # sudo yum groupinstall -y 'Development Tools'

## Google Testing Library
wget -O gtest.tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz
tar xf gtest.tar.gz
cd googletest-release-1.7.0
cmake -DBUILD_SHARED_LIBS=ON .
make
cp -a include/gtest /usr/local/include
cp -a libgtest_main.* libgtest.* /usr/local/lib/
cd ..

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

# Verbose Options
You can turn on verbose logging using `--v=<level>` or `-v` for level 9

| **Level** | **Name**        | **Description**                                                       |
|-----------|-----------------|-----------------------------------------------------------------------|
| 1         | INFO            | General information                                                   |
| 2         | NOTICE          | Warnings that must be addressed by residue server admin               |
| 3         | WARNING         | Warnings that can be ignored                                          |
| 4         | ERROR           | Error information                                                     |
| 5         | DETAILS         | General information with extra details                                |
| 6         | DEBUG           | Debug information that contains useful information for devs           |
| 7         | DEBUG 2         | Debug information that contains a lot of useful information for devs  |
| 8         | TRACE           | Trace function calls                                                  |
| 9         | CRAZY           | A lot of information - very useful for development purposes           |

You can also specify verbose logging by modules. Please see [Application Arguments](https://github.com/muflihun/easyloggingpp#application-arguments) section on Easylogging++
