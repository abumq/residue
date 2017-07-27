                                       ‫بسم الله الرَّحْمَنِ الرَّحِيمِ

## Client Development
This document show you steps required in order to write a new client library for residue server.

In order to develop an efficient client library you should understand how it works by checking [DATA_FLOW](/docs/DATA_FLOW.md)

Residue comes with official C++ client library for C++ applications and NodeJS library for javascript applications. This document contains core concepts behind development of these libraries. You can use same concepts to create your own library in language of your choice for other developers or for your own applications.

### What is Client Library?
Server is useless without a client. In residue's world, client is what sends log request payload to the server. Client library is abstract layer between developer of the application and the residue server. 

### Make It Official
If you have written client library for your organization and wish to make it officially part of Residue client libraries following conditions must be met:

#### Seamless
All the requests to and from server should be seamless and user should not have to manually enter any details other than initial setup (`connect`, `disconnect`, `setParams`)

#### API
API should be easy to follow and use. User should not be asked for too many details. For example, your library should calculate the source line number to `%line` format specifier. 

#### Documentation
You should provide full documentation of public API that is relevant to the user. Documentation for underlying details are recommended but not required to make it official library.

#### License
You can choose to sell the library but in order for it to be official it should be open-source licensed and code should be open source. This is for security purposes.

#### Plug & Play
If you are writing a library in language that is portable, make sure it is cross-platform so user does not have to do anything special for special platforms. For example, if you are writing your own C++ client library (and do not want to use official library for some reason), it should work on all major operating systems i.e, Windows, macOS, Linux, Android (JNI) and iOS (Objective-C++). Number of operating system vary with scope of your library, for example, Java library will not be expected to support iOS but should support Android (for apps).

#### Asyncronous
User should not be blocked by log requests. Ideally, library should have it's own dispatch thread that will send the requests over the network.

#### Compression
Although server will work fine without compression enabled but you should always make sure your library will compress the log data when compression is supported by the server (you can check this using server flags).

### Development
Following are the public functions that must be available for user

#### CONNECT
Provide a public function for developer to connect to the server.

Official library has singleton class called `Residue` that contains static helper function called `connect`:

```c++
void connect(const std::string& host, int port, AccessCodeMap* accessCodeMap) noexcept;
```

`accessCodeMap` is pointer to locally created map of `logger_id` to it's corresponding `access_code` that will be used to get all the tokens in future.

User can use this function in applications entry point:

```c++

#include <Residue.h>

int main() {
    Residue::AccessCodeMap accessCodes = {
        {
            "muflihun", "a2dcb"
        }
    };
	
    Residue::connect(Residue::LOCALHOST, Residue::DEFAULT_PORT, &accessCodes);
    ...
}
```

Please see [implementation details of this function](/lib/Residue.cc).

### LOG
After user is connected, they should be able to send log requests. Provide public methods that will take simple parameters (e.g, log message) and will generate source file, source function, source line etc. DO NOT ASK USER TO PROVIDE LOW LEVEL DETAILS.

For official library, it was easy as we use same API as Easylogging++. For example, user can send log message using:

```c++
LOG(INFO) << ...
```

Please note following important points:

 * We have our own log dispatcher that essentially disables default dispatcher (to file or console) and sends it to the network.

 * With each log request, library should check whether token on server is still valid or not (you can use `life` and `date_created`. If token is not valid on server, first obtain a new token (using access code map provided by the user in `connect()` function) and then send the request.

 * With each log request, first check whether client is still alive or not, if it's not dead yet, send a ping request. If you find out that client died in the server, reconnect before sending requests.
 
All of this is done under the hood and user should not know the details of it.

### PARAMETERS
You can ask users to provide additional paramters, for example, bulk size etc.

### DISCONNECT
Server does not accept any disconnect requests. Closing the socket should suffice. This is thoughtful decision as multiple known clients may be connected using same client ID.

### Technical Details
In order to develop client library you will need following implementations:

 * Network client: To read and write data to the server socket. This can be utility functions or class
 * RSA Encryption/Decryption: To encrypt connection request in order to exchange symmetric key
 * AES Encryption/Decryption: To encrypt all the conversation to and from the server
 * Base64 Encoding/Decoding: All the conversation to and from the server in base64 encoding (rather than hex)
 * GZip Compression: Whether server supports compression or not, you should always aim to support compression on your client library. The compression is only done on plain JSON log request before encrypting or base64 encoding.
 
These are minimal features that should be supported by client library. If you wish to add something more i.e, to have extra features you can wish to do so e.g, bulk requests etc.

### Conclusion
Core concept is encapsulating the details and making connection to residue server as seamless as possible. Once you understand this you should have no problem in create libraries for the developers.