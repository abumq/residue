﷽

# Client API
Residue comes with official C++ client library for users to use in their C++ applications. You can easily integrate residue in to your existing C++ application with literally 3 changes.

For other client libraries please refer to corresponding repository.

### 1. Download Library and Header
You need 2 things to connect to residue

1. `Residue.h` header file - this contains API for connecting to residue server
2. `libresidue` shared library - you will need to link your application to this library in order to successfully compile.

You may be interested in [compiling static library](/docs/INSTALL.md#static-library)

### 2. Define `ELPP_THREAD_SAFE`
Make sure you define `ELPP_THREAD_SAFE` preprocessor at compile level (whole application takes this affect)

### 3. Initialize
In your `main()`, send connection to residue. (NOTE: Residue server should be running in order to connect otherwise you will get error)

```c++
#include <map>
#include <Residue.h>

int main() {
   
    // You will need to fill this map with access codes to access correct logger (depends on the server)
    // For our samples we have used "sample-app" logger whose access code are in config file
    // https://github.com/muflihun/residue/blob/master/samples/residue.conf.json
    // so we define these access code for seamless interaction
    Residue::AccessCodeMap accessCodes = {
        {
            "sample-app", "a2dcb"
        }
    };
    
    // Connects to localhost on Residue::DEFAULT_PORT (8777)
    Residue::connect(&accessCodeMap);
    
    ///
    /// Start logging using Easylogging++ macros i.e, LOG(INFO) etc
    ///
    /// If you wish to log using specific logger you will have to use CLOG(INFO, "<logger-id>")
    /// See https://github.com/muflihun/easyloggingpp#logging for details
    ///
    
    // It's always best to safely disconnect before your application ends, this ensures 
    // all the logs have been flushed to the server
    Residue::disconnect();
    
    return 0;
}
```

### Documentation
Please refer to [this link](https://muflihun.github.io/residue/docs/) for documentation for API

### Helper
In our [sample](/samples/clients/c++/fast-dictionary/log.h) we create helper header `log.h` and included that everywhere. This helps ensure we don't have to use `CLOG(INFO, "<logger_id>")` everywhere and gives us ability to use `LOG(INFO)` that will log using your own logger.

### Samples
We have written [some samples using this library](/samples/clients/c++), please feel free to download and play around.
