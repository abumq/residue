<p align="center">
   ﷽
</p>

# Protocol
This document explains the residue protocol.

## Connection
### Hello from Client
First request comes from client library either in plain format or encrypted with server public key, containing:

 * Type = 1 (CONNECT)
 * Client ID (if any)
 * Client public key
 
### Hello from Server
Server verifies the client and responds with:

 * Client ID (previously sent or newly generated)
 * Key
 * Acknowledgement status (whether client needs to acknowledge the connection or not)
 
### Acknowledgement from Client
Client the decrypts the above information using their private key and send acknowledgement request using key from the server, this request contains:

 * Type = 2 (ACKNOWLEDGEMENT)
 * Client ID
 * Timestamp (in seconds)
 
### Connection Establishment
After verifying the client, server finally establishes the full connection. This connection remains active until socket is connected. Server responds with full information that helps client library to send `TOUCH` requests when needed. This information consists of:

 * Status — 0 = successfully connected
 * Acknowledgement status — 1 = acknowledged and does not need any more `ACKNOWLEDGEMENT` connection
 * Server flags — an integer value specifying the different features enabled by the server, e.g, whether or not server accepts bulk requests or compression etc.
 * Maximum bulk size — If server allows bulk request, number of items client can send in a bulk
 * Key — The final key that all the future requests should be encrypted with
 * Token port — The port that server listens to, for obtaining tokens
 * Logging port — The port that server listens to, for log requests
 * Age — Maximum age of the client. A time in seconds which specifies when client will be removed. Client library will send `TOUCH` request if client needs to stay active. This request will give another life to the client.
 * Date created — Date the client was created on the server. This is what client library calculates the age off.
  * Server Info — Server information containing server version, licensee, license expiry date etc.

At this point, client knows server and server knows client. Now they can talk securily without letting any third-party interfering with the connection.

## Obtaining Token
Once connection is estabilished you may need to obtain token. This is done by sending request to `token_port` from connection response

The JSON payload will be:

 * `logger_id` (string)
 * `access_code` (string)
 
You may be interested in [create_request-token](/tools/netcat-client/create_request-token.php) from netcat client sample

## Log Request
Logs are send in JSON format with following payload

 * `token` (string, previously obtained token code)
 * `datetime` (integer, in millisecond)
 * `logger` (string, logger ID)
 * `msg` (string, log message)
 * `file` (string, log message originating file)
 * `line` (integer, log message originating source line)
 * `func` (string, log message originating function name)
 * `app` (string, application ID)
 * `level` (integer, mapped as `TRACE => 2, DEBUG => 4, FATAL => 8, ERROR => 16, WARNING => 32, VERBOSE => 64, INFO => 128`)
 * `vlevel` (integer, verbosity level if any)
 * `thread` (string, log message originating thread ID)
 
You may be interested in [create_request-log.php](/tools/netcat-client/create_request-log.php) and [create_request-bulk-log.php](/tools/netcat-client/create_request-bulk-log.php) from netcat client sample

## Bulk Log Request
Bulk requests is (JSON) array of log request


