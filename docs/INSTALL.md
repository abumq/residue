<p align="center">
   ﷽
</p>

# Installation
This document shows you steps to install residue server on your machine. You can install it for development or for production use.

# Dependencies
  * C++11 (or higher)
  * [Ripe](https://github.com/muflihun/ripe)\* v3.3.0
  * [Mine](https://github.com/muflihun/mine) v1.0.0
  * Boost v1.59 or higher [Components: [asio](http://www.boost.org/doc/libs/master/doc/html/boost_asio.html)]
  * [Easylogging++](https://github.com/muflihun/easyloggingpp) v9.95.0
  * [CMake Toolchains](https://cmake.org/) v2.8.12
  
\* We're working to remove this dependency
 
# Get Code
You can either [download code from master branch](https://github.com/muflihun/residue/archive/master.zip) or clone it using `git`:

```
git clone git@github.com:muflihun/residue.git
```

# Build
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
| `disable_curl_support` | Do not use libcurl. If you turn it off querying https will not be possible |
| `use_mine` | Use [minimal encryption library](https://github.com/muflihun/mine) whereever possible (instead of ripe) |

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

### If Build Fails...
Make sure you have all the dependencies installed. You can use following script to install it all and then go back to [Build](#build) section (tested on Ubuntu 16.04 64-bit)

```
## Boost System and ASIO
sudo apt-get install -y libboost-system-dev cmake

## Google Testing Library
wget -O gtest.tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz
tar xf gtest.tar.gz
cd googletest-release-1.7.0
cmake -DBUILD_SHARED_LIBS=ON .
make
sudo cp -a include/gtest /usr/include
sudo cp -a libgtest_main.so libgtest.so /usr/lib/
cd ..

## Easylogging++
wget -O elpp-master.zip https://github.com/muflihun/easyloggingpp/archive/master.zip
unzip elpp-master.zip
cd easyloggingpp-master
cmake .
make
sudo make install

## Mine
wget -O mine-master.zip https://github.com/muflihun/mine/archive/master.zip
unzip mine-master.zip
cd mine-master
cmake .
make
sudo make install

## Ripe
wget -O ripe-bin.tar.gz https://github.com/muflihun/ripe/releases/download/v4.0.0/ripe-4.0.0-x86_64-linux.tar.gz
tar xfz ripe-bin.tar.gz
cd ripe-3.3.0-x86_64-linux
sudo cp libripe.* /usr/lib/
sudo cp Ripe.h /usr/include/
sudo cp ripe /usr/bin/
```

#### Boost Library Linking Error

If you get an error similar to

```
relocation R_X86_64_32 against .rodata.str1.1 can not be used when making a shared object; recompile with -fPIC 
... libboost_system.a: error adding symbols: Bad value
```

This happens when static library (`libboost_system` in our case) is linked against shared library (`libresidue` in our case), you can link it statically. See [Static Library](#static-library) section below.

# Run as `root`
You will need to run residue as root user. This is because residue needs to change the ownership of the files to the relevant users and yet need to write to those files.

If you are just need to test the residue before you run it in production (in production you should always run it as root) you can use `--force-without-root` command-line argument.

# What's Next?
You can run demo to see how residue works. Please refer to [DEMO.md](/docs/DEMO.md) documentation
