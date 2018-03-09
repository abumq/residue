<p align=center>
   ﷽
</p>

[🏠 Configurations](/docs/CONFIGURATION.md)
[→ Known Clients](/docs/configurations/known_clients/#known_clients)

# known_clients ↴
## client_id
[String] The client ID (should be alpha-numeric and can include `-`, `_`, `@`, `.` and `#` symbols)

### Client
Client is an instance of residue connection. A client is responsible for managing the associated loggers, associated OS user. It holds both symmetric and asymmetric keys for the connection.

### Client Processor
Each client has it's own log processor assigned to it. These log processors are responsible to process the incoming logs. These processors have queues that are filled up by log request handlers a.k.a logging server.
