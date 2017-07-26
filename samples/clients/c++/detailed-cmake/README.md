                                       ‫بسم الله الرَّحْمَنِ الرَّحِيمِ

A very C++ app that connects to the logging server using [official residue client library for C++](/include/Residue.h)

This sample contains various `#if 0` lines that are followed by comment explaining it. This is because you can enable these features and play around.

### Pre-requisite

Make sure residue server is running locally on port `8777` (or change it in configuration). See [INSTALL.md](/docs/INSTALL.md) for instructions

### Run
```
mkdir build
cd build
cmake ..
make
cd ../../../../../ ## This is for relative config files path only not a MUST
./samples/clients/c++/detailed-cmake/build/residue-sample-app
```

Check the logs on server on `/tmp/logs/sample-app.log`
