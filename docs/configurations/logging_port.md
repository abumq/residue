<p align=center>
   ﷽
</p>

[🏠 Configurations](/docs/CONFIGURATION.md)

# logging_port
[Integer] Port that logging server listens to. This is where all the log requests are sent.

Default: `8778`

## Logging Server
This server is responsible for handing the log requests, passing it to one of the log processors and responding client with the status code.

Logging server only checks for the initial validity. The details are checked by log processor (as they may be heavy and may be queued up for later).

Once this server receives the log request it:

 * Decrypts and inflates the request
 * Makes sure the client is valid and alive (client's validity is not checked in case of bulk log requests)
 * Responds to the client with one of the following codes
 
| **Code** | **Description** |
|----------|-----------------|
| `0`      | `OK`            |
| `1`      | `BAD_REQUEST`            |
| `2`      | `INVALID_CLIENT`            |

The response looks like `{"r":0}`

[🏠 Configurations](/docs/CONFIGURATION.md)

