<p align="center">
   ﷽
</p>

# Client Development
This document gives you detailed guidelines on how to write a new client library.

We highly recommend you understand how the server and client interact and what's the role of a developer in the whole process. You can understand all this by looking at [DATA_FLOW](/docs/DATA_FLOW.md) diagram and reading [PROTOCOL](/docs/PROTOCOL.md) specifications.

## Official Libraries
Residue comes with following official client libraries. You can look at their source code for references and guidelines.

 * [C++ (C++11 or higher)](https://github.com/muflihun/residue-cpp)
 * [Node.js](https://github.com/muflihun/residue-node)
 * [Java (Incl. Android, API level 26 or higher)](https://github.com/muflihun/residue-java)
 
## What is Client Library?
Server is useless without a client. In _residue_'s world, client is what sends log request payload to the server. Client library is an abstract layer between end-developer and the server (residue). 

## Development
### Essentials
You should start by implementing following helper classes (or modules) that you will need later

#### Asyncronous TCP Client
Recommended name: `ResidueClient` or `NetworkClient`

You will need following functions:
 * `connect()`: which will estabilish connection to the host and port. This estabilishment of connect should be syncronous for it to correctly work
 * `read()`: which reads the data from the server. Each packet from the server ends with `\r\n\r\n` (a.k.a `PACKET_DELIMITER`)
 * `send()`: which sends the data to the server. This sending can be syncronous or asyncronous. In either case it should have ability to expect a response and trigger a callback on successfully receiving data. (this callback will be `read` in most cases)
 
See also
 * `ResidueClient` in [C++](https://github.com/muflihun/residue-cpp/blob/master/src/Residue.cc)
 * `ResidueClient` in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java)
 * `Utils.sendRequest`, `Params.*_socket.on('data', ...)` in [Node.js](https://github.com/muflihun/residue-node/blob/master/src/residue.js)
 
#### RSA Encryption/Decryption with PKCS#1 padding
You can put this class in single utility class.

You will need following functions:
 * `readPemPrivateKey`: which will take PEM format (and secret) for the key and returns private key object
 * `readPemPublicKey`: which will take PEM format for the public key and returns public key object
 * `createNewRSAKey`: which will generate new RSA key of specified size
 * `encryptRSA`: which will encrypt the data and returns raw bytes
 * `decryptRSA`: which will decrypt raw bytes using private key
 
See also
 * `generateRSAKeyPair`, `decryptRSA`, `encryptRSA` in [C++](https://github.com/muflihun/residue-cpp/blob/master/src/Residue.cc)
 * `getPemPublicKey`, `getPemPrivateKey`, `createNewKeyPair`, `encryptRSA`, `decryptRSA` in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java)
 * `Utils.extractPublicKey`, `Utils.generateKeypair`, `Utils.encryptRSA`, `Utils.decryptRSA` in [Node.js](https://github.com/muflihun/residue-node/blob/master/src/residue.js)
 
#### AES Encryption/Decryption
 
#### Base16 Encoding/Decoding
 
#### Base64 Encoding/Decoding

#### GZip Compression (optional)

### Conclusion
Core concept is encapsulating the details and making connection to residue server as seamless as possible. Once you understand this you should have no problem in writing client libraries for the developers.

## Make It Official
If you have written client library for your organization and wish to make it part of _official Residue client libraries_, following conditions must be met:

#### Seamless
All the requests to and from server should be seamless and user should not have to manually enter any details other than initial setup (i.e, loading configuration and connecting)

#### API
API should be easy to follow and use. User should never be asked for too much details. For example, your library should calculate the source line number to `%line` format specifier. 

#### No Malware Policy
Your library should be ethical and should not send the irrelavant data. It should be straight forward. We will review the source code for the library thoroughly before it can become an official library.

#### Documentation
You should provide full documentation of public API that is relevant to the developer.

Documentation for underlying details are recommended but not required (to make it official library).

#### Licensing
You can choose to sell the library but in order for it to be official it should be open-source licensed and code should be open source. This is for security purposes. You can choose from any of the following licences (unless you have better suggestion to make, feel free to do so)

 * MIT
 * Apache 2.0

#### Naming Conventions
With each of the steps above we have provided recommended names for each "helper" classes. You must name your classes according to our recommendations if you wish to make it an official library.

#### Plug & Play
If you are writing a library in language that is portable, make sure it is cross-platform so user does not have to do anything special for special platforms. For example, if you are writing your own C++ client library (and do not want to use official library for some reason), it should work on all major operating systems i.e, Windows, macOS, Linux, Android (JNI) and iOS (Objective-C++). Number of operating system vary with scope of your library, for example, Java library will not be expected to support iOS but should support Android (for apps).

If your library works for one major platform and not other you should specify this so developer knows it upfront.

#### Asyncronous
User should not be blocked by log requests. Ideally, library should have it's own dispatch thread that will send the requests over the network.

#### Compression
Although server will work fine without compression enabled but you should always make sure your library will compress the log data when compression is supported by the server (you can check this using server flags).

#### Re-establish Lost Connection
Library should track whether the connection was closed by remote or not. If so, it should strive to re-connect and in the meantime should keep local copies of log request. Once reconnected, all the log requests should be back to the server.

This is extremely important as it will make upgrading the server software easier.

