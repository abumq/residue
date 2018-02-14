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
### Helpers
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
 * `Ripe::Ripe::generateRSAKeyPair`, `Ripe::decryptRSA`, `Ripe::encryptRSA` in [C++](https://github.com/muflihun/residue-cpp/blob/master/src/Residue.cc) (C++ library uses [Ripe](https://github.com/muflihun/ripe))
 * `getPemPublicKey`, `getPemPrivateKey`, `createNewKeyPair`, `encryptRSA`, `decryptRSA` in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java)
 * `Utils.extractPublicKey`, `Utils.generateKeypair`, `Utils.encryptRSA`, `Utils.decryptRSA` in [Node.js](https://github.com/muflihun/residue-node/blob/master/src/residue.js)
 
#### AES-CBC Encryption/Decryption
You can put this class in single utility class.

You will need following functions:
 * `encrypt`: which takes string and hex key and returns encrypted data in form: `<initialization_vector>:<client_id>:<base64_encoded_encryption>`
 * `decrypt`: which takes string in form `<initialization_vector>:<client_id>:<base64_encoded_encryption>` and hex key and decrypts the `base64_encoded_encryption` to string
 
 See also
 * `Ripe::decryptAES`, `Ripe::encryptAES` in [C++](https://github.com/muflihun/residue-cpp/blob/master/src/Residue.cc) (C++ library uses [Ripe](https://github.com/muflihun/ripe))
 * `ResidueUtils.encrypt`, `ResidueUtils.decrypt` in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java)
 * `Utils.encrypt`, `Utils.decrypt` in [Node.js](https://github.com/muflihun/residue-node/blob/master/src/residue.js)
 
#### Base64 Encoding/Decoding
All the data from and to the server are encoded using base64 encoding.

You can put this in single utility class.

You will need following functions:
 * `encodeBase64`: which takes raw bytes and encodes them in to base-64
 * `decodeBase64`: which takes base-64 encoding and returns raw bytes
 
 See also
 * `Ripe::base64Encode`, `Ripe::base64Decode` in [C++](https://github.com/muflihun/residue-cpp/blob/master/src/Residue.cc) (C++ library uses [Ripe](https://github.com/muflihun/ripe))
 * `ResidueUtils.base64Encode`, `ResidueUtils.base64Decode` in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java)
 * `Utils.base64Encode`, `Utils.base64Decode` in [Node.js](https://github.com/muflihun/residue-node/blob/master/src/residue.js)

#### Base16 Encoding/Decoding (optional)
You may need this function or may not, depending on your implementation.

You can put this in single utility class.

You will need following functions:
 * `encodeBase16`: which takes raw bytes and encodes them in to base-16
 * `decodeBase16`: which takes base-16 encoding and returns raw bytes
 
 See also
 * Not used in [C++](https://github.com/muflihun/residue-cpp/blob/master/src/Residue.cc) library
 * `ResidueUtils.hexEncode`, `ResidueUtils.hexDecode` in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java)
 * [Node.js](https://github.com/muflihun/residue-node/blob/master/src/residue.js) library uses javascript's `toString('hex')` and `new Buffer(..., 'hex')`

#### GZip Compression (optional)
You may want to compress the packets before sending to the server that support [`Compression`](/docs/CONFIGURATION.md#compression)

You will need following functions:
 * `compress`: which takes raw bytes and compress them using zlib algorithm
 
 See also
 * `Ripe::compressString` in [C++](https://github.com/muflihun/residue-cpp/blob/master/src/Residue.cc) (C++ library uses [Ripe](https://github.com/muflihun/ripe))
 * [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java) library uses `DeflaterOutputStream`. See `dispatcher` thread
 * [Node.js](https://github.com/muflihun/residue-node/blob/master/src/residue.js) uses `zlib.deflateSync` from
 
### Public API
Once you have created helper classes you should be good to write public API that will be used by the user of your library (developer)

Public API will have following features/functions

#### `loadConfiguration`
This function will allow user to pass in JSON file or JSON data and load all the necessary information from it. This JSON data should have same object names to what we officially have, at the time of writing this document, the configuration format is:

```javascript
{
    "url": "localhost:8777",
    "access_codes": [
        {
            "logger_id": "<logger-id>",
            "code": "<access-code>"
        }
    ],
    "application_id": "<application-id>",
    "rsa_key_size": 2048,
    "plain_request": false,
    "utc_time": false,
    "time_offset": 0,
    "dispatch_delay": 1,
    "main_thread_id": "main_thread",
    "client_id": "my_client",
    "client_private_key": "<private-key-path>",
    "client_key_secret": "<secret-if-any>",
    "server_public_key": "<server-public-key-path>",
    "internal_logging_level": 0
}
```

On calling this function the data will be loaded to the memory and will be used thereof.

See `loadConfigurations` from [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java) library for example

#### `connect`
This function connects user to specified host and port running residue server.

The connection is estabilished syncronously with short timeout. This function can be overloaded with various parameters, i.e., host, port or access code map (like we have for [C++](https://muflihun.github.io/residue/docs/class_residue.html) library

This function will notify user if there was failure in estabilishing the connection and clear reasoning:
 * If host is unavailable or not connected to the network
 * If residue denied the connection e.g, invalid access code or invalid public key
 
Once connected, the connection response is most important thing, it contains what has been mentioned in [`PROTOCOL`](/docs/PROTOCOL.md#connection-establishment) specification. You will need it throughout.
 
See `Residue.connect` in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java) library for example

#### `log`
These are set of functions that take user's input (log message) and puts them in a list, a.k.a, _log backlog_

Make sure this is done thread-safely. User may be writing the logs from various threads.

### Dispatcher Thread
This thread is responsible of looking at the _log backlog_ and dispatch the items it to the server. This thread will need to check various states before deciding what to do next.

What do we mean by that? this is where "seamless" comes in, it will check for:

 * Whether still connecting - if connecting then come back and try in few moment (Find `Still connecting...` on [C++](https://github.com/muflihun/residue-cpp/blob/master/src/Residue.cc) for example)
 * Whether client is still valid or not - if not it will reconnect and then call do it again (See `isClientValid()` on [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java) and see the usages)
 * Whether client is about to die and can be _retouched_ (See `shouldTouch()` and `touch()` on [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java) and see the usages)
 * Whether we have token or need to obtain token or token is valid - if not we will first obtain token using access code previously provided by the user (See `obtainToken()` on [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java) and it's usages especially in `dispatcher` thread)
 * Create bulk request if your library and server supports it (see [`allow_bulk_log_request`](/docs/CONFIGURATION.md#allow_bulk_log_request))
 * Compress the data if your library and server supports it (see [`compression`](/docs/CONFIGURATION.md#compression) and search for `Flag.COMPRESSION.isSet()` in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java) library for example)
 * Encrypt the JSON object if needed. Otherwise, if developer prefers plain log request and server supports it, you can ignore encryption. (see [`allow_plain_log_request`](/docs/CONFIGURATION.md#allow_plain_log_request) and search for `Flag.ALLOW_PLAIN_LOG_REQUEST.isSet()` in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java) library for example)

Remember, all of this happens behind the scenes and developer does not have to know these details.

See also
 * `Residue::dispatch` in [C++](https://github.com/muflihun/residue-cpp/blob/master/src/Residue.cc)
 * `dispatcher` thread in [Java](https://github.com/muflihun/residue-java/blob/master/src/com/muflihun/residue/Residue.java) library
 * `sendLogRequest` in [Node.js](https://github.com/muflihun/residue-node/blob/master/src/residue.js) library does not have multi-threading so it's dispatched as is (but it does check for all the above items)

### JSON Payload
All the requests are JSON based and each type of connection is sent to specific port as specified in [`PROTOCOL`](/docs/PROTOCOL.md) specification

For those of you who are familiar with bash scripting and PHP scripting we have [`netcat` client for demo purposes](/tools/netcat-client/) that will also help you create correct JSON payload

### Conclusion
Core concept is encapsulating the details and making connection to the server as seamless as possible. Once you understand this you should have no problem in writing client libraries for the developers.

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

