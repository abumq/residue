<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# connect_port
[Integer] Port that connection server listens to. All the connection requests are sent to this port and handled accordingly. This is the port that all the clients libraries initially connect to.

Default: `8777`

## Connection Server
Developer will have this port in their configuration alongside the host. All the requests are made to this port for connection purposes. After successful connection, this server will notify client library about server information including the logging port that server is listening to.

## Responsibilities
This server is responsible for following tasks

 * Initial CONNECTion - this will receive an unencrypted or encrypted connection request, generate a unique key for the client and pass on via client's public key
 * ACKNOWLEDGEMENT of the connection - once client receives the newly generated key it will send an ACKNOWLEDGEMENT request to this port encrypted with newly generated key
 * TOUCH - when client is about to expire, client library sends a TOUCH request to re-alive the connection. This request is also sent to this server
 
## Summary
In summary, this port / server is responsible for keeping the connection between client and the residue core healthy. 

[← Configurations](/docs/CONFIGURATION.md)

