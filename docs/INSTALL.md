<p align="center">
   ﷽
</p>

# Overview
Residue can be installed either by building from the source or by downloading the binaries from respective releases.

# Supported Platforms
Residue **has been tested** on the following platforms and should work on other major distributions (that are not listed here) especially the higher versions of listed platforms

## Linux
 * Ubuntu 14.04 (64-bit)
 * Amazon Linux AMI 2017.03
 * Fedora 19 (64-bit)
 * Fedora 24 (64-bit)
 * CentOS 7 (64-bit)
 
Other distributions that _should_ work (but have not yet been tested)
 * Oracle Linux
 * RHEL 7 or higher (64-bit)
 * Amazon Linux AMI 2016.03

## macOS
 * macOS 10.13 (High Sierra) (64-bit)
 
Other distributions that _should_ work (but have not yet been tested) - you may need to rebuild from source
 * macOS 10.11 (El Capitan) (64-bit)
 * macOS 10.12 (Sierra) (64-bit)

# Download Binary
You can download binary from [releases](https://github.com/muflihun/residue/releases) page for your platform. They are standalone binaries but they require some external libraries installed that are mentioned with each release notes. Most of these external libraries come with respective operating system distributions.

## Via NPM

### Linux
```
$ sudo npm install -g residue-linux@latest
sudo ln -s `which residue-linux` /usr/local/bin/residue
```

### macOS
```
$ sudo npm install -g residue-darwin@latest
sudo ln -s `which residue-darwin` /usr/local/bin/residue
```

## Direct
### Linux
```
wget https://github.com/muflihun/residue/releases/download/v2.1.0/residue-2.1.0-linux-x86_64.tar.gz
tar -xf residue-2.1.0-linux-x86_64.tar.gz
cd residue-2.1.0-linux-x86_64/
```

### macOS
```
wget https://github.com/muflihun/residue/releases/download/v2.1.0/residue-2.1.0-darwin-x86_64.tar.gz
tar -xf residue-2.1.0-darwin-x86_64.tar.gz
cd residue-2.1.0-darwin-x86_64/
```

## Start Server

### Via NPM
If you downloaded Residue binary using NPM, you can use following lines to start server with sample configs.

You will need to find out where global packages for NPM are installed (use `npm root -g`)

If you are using residue extensions you will also need to update `LD_LIBRARY_PATH` environment variable where libresidue-extension is located.

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/node_modules/residue-linux/
```

We set `RESIDUE_HOME` environment variable as they are needed by sample server config. `RESIDUE_HOME` is usually the root for residue configurations.

```
export RESIDUE_HOME=/usr/local/lib/node_modules/residue-linux/config/
## Following will line will set environment variable for node
sudo RESIDUE_HOME=$RESIDUE_HOME residue $RESIDUE_HOME/server.conf
```

### Direct
Start server using this configuration

If you are using residue extensions you will also need to update `LD_LIBRARY_PATH` environment variable where libresidue-extension is located.

```
## LD_LIBRARY_PATH always include current working directory so we're commenting it out here
## but if you are running residue from elsewhere you will need to update it

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
```

```
sudo ./residue config/server.conf
```
## Sample Client
## Download
Open another terminal and start sending log requests using sample client logger

```
wget https://github.com/muflihun/muflihun.github.io/raw/master/downloads/sample-logger.tar.gz
tar -xf sample-logger.tar.gz
```

## Send Logs

### Linux
```
./sample-logger/linux/residue-logger --conf=sample-logger/conf.json
```

### macOS
```
./sample-logger/darwin/residue-logger --conf=sample-logger/conf.json
```

This sample sends using `default` logger. Source code for this sample client logger can be found @ [Residue C++ samples](https://github.com/muflihun/residue-cpp/blob/master/samples/minimal/main.cc)

## View Logs
You can tail your logs using

```
tail -f /tmp/logs/residue.log
``` 

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
cmake ..
make
```

## CMake Options
You can change following options in CMake (using `-D<option>=ON`)

|    Option    | Description                     | Default |
| ------------ | ------------------------------- |---------|
| `debug`      | Turn on extra logging           | `OFF`   |
| `production` | Compile for production use      | `ON`   |
| `enable_extensions` | Enable extension support      | `ON`   |
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

If you are just need to test the residue before you run it in production (in production you should always run it as root) you can use `--force-without-root` command-line argument. [NOT RECOMMENDED]

# `RESIDUE_HOME`
`RESIDUE_HOME` is environment variable that is resolved at runtime if provided with, in the value. (Only applicable to some configurations)

E.g, if `RESIDUE_HOME` is exported to `/opt/residue/config` and you have public key `$RESIDUE_HOME/server-priv.pem` the final file that will be read will be `/opt/residue/config/server-priv.pem`

If you provide `--home-path=<path>` when starting the server, `RESIDUE_HOME` environment variable will be overridden by the value provided at startup.

# Verbose Options
You can turn on verbose logging using `--v=<level>`. `-v` is equivalent to `--v=9`

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
