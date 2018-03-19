<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# known_clients ↴
## client_id
[String] The client ID (should be alpha-numeric and can include `-`, `_`, `@`, `.` and `#` symbols)

### Client
Client is an instance of residue connection. Multiple applications can connect single client on the server. A client is responsible for managing the associated loggers, OS user etc. It holds both symmetric and asymmetric keys for the connection.

A client-logger relation is one-to-many where one client can be associated to one or more loggers.

### Client Processor
Each client has it's own log processor assigned to it. These log processors are responsible to process the incoming logs. These processors have queues that are filled up by log request handlers a.k.a logging server.
