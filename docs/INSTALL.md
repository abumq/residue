
                                       ‫بسم الله الرَّحْمَنِ الرَّحِيمِ

# Installation
This document shows you steps to install reside on your machine. You can install it for development or for production use.

# Dependencies
  * C++11 (or higher)
  * [Ripe](https://github.com/muflihun/ripe) v2.4.1
  * Boost 1.59 or higher [Components: system]
  * [Easylogging++](https://github.com/muflihun/easyloggingpp) v9.94.2
  * [CMake Toolchains](https://cmake.org/) 2.8.12
 
# Get Code
You can either [download code from master branch](https://github.com/muflihun/residue/archive/master.zip) or clone it using `git`:

```
git clone git@github.com:muflihun/residue.git
```

# Build
Residue uses the CMake toolchains to create makefiles.
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
| `build_sample_app` | Builds sample app (to check client library in action) |
| `build_static_lib` | Builds static library for official c++ client library (see [Static Library](#static-library))

Please consider running unit test before you move on

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
sudo apt-get install -y libboost-system-dev cmake
wget -O gtest.tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz
tar xf gtest.tar.gz
cd googletest-release-1.7.0
cmake -DBUILD_SHARED_LIBS=ON .
make
sudo cp -a include/gtest /usr/include
sudo cp -a libgtest_main.so libgtest.so /usr/lib/
cd ..
wget -O elpp-master.zip https://github.com/muflihun/easyloggingpp/archive/master.zip
unzip elpp-master.zip
cd easyloggingpp-master
cmake .
make
sudo make install
wget -O ripe-bin.tar.gz https://github.com/muflihun/ripe/releases/download/v2.2.0/ripe-2.2.0-linux-x86_64.tar.gz
tar xfz ripe-2.2.0-linux-x86_64.tar.gz
cd ripe-2.2.0-linux-x86_64
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

#### RESIDUE_LIBRARY_LOCAL-NOTFOUND Error

If you get error at `cmake -Dbuild_sample_app=ON ..` step complaining about `RESIDUE_LIBRARY` not found, this is because sample-client application uses this, you can turn this option off, install the library and then rebuild with this option.

### Static Library
By default, residue builds shared library that you can link in order to connect to residue server seamlessly. You can choose to build static library instead using `build_static_lib` option in cmake

For example,

```
cmake -Dbuild_static_lib=ON ..
```

This will create `libresidue.a` and you can create one single static library (with boost bindings) using

```
ar -x libresidue.a
cp /usr/local/lib/libboost_system-mt.a . # Path has to match your system's path
ar -x libboost_system-mt.a
ar -qc libresidue_full.a *.o
```

This will essentially join all the object files from both the libraries (no name overlaps).

You then link with this library and ripe (dynamic) library using options `-lresidue_full -lripe`

# What's Next?
You can run demo to see how residue works. Please refer to [DEMO.md](/docs/DEMO.md) documentation
