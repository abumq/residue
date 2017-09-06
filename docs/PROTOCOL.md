﷽

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

 * Status —— 0 = successfully connected
 * Acknowledgement status —— 1 = acknowledged and does not need any more `ACKNOWLEDGEMENT` connection
 * Server flags —— an integer value specifying the different features enabled by the server, e.g, whether or not server accepts bulk requests or compression etc.
 * Maximum bulk size —— If server allows bulk request, number of items client can send in a bulk
 * Licensee —— Server's licensee name
 * Key —— The final key that all the future requests should be encrypted with
 * Token port —— The port that server listens to, for obtaining tokens
 * Logging port —— The port that server listens to, for log requests
 * Age —— Maximum age of the client. A time in seconds which specifies when client will be removed. Client library will send `TOUCH` request if client needs to stay active. This request will give another life to the client.
 * Date created —— Date the client was created on the server. This is what client library calculates the age off.
 
At this point, client knows server and server knows client. Now they can talk securily without letting any third-party interfering with the connection.

