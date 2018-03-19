<p align="center">
   ﷽
</p>

# CLI Commands
When residue server starts it can accept some commands from command-line interface. This document lists these commands.

Please note, some of these commands can also be triggered using admin requests.

## Table of Contents

* [quit](#quit)
* [rconfig](#rconfig)
* [reset](#reset)
* [history](#history)
* [clients](#clients)
* [tokens](#tokens)
* [rotate](#rotate)
* [stats](#stats)
* [files](#files)
               
### `quit`
Quits the server gracefully

### `rconfig`
Reloads configurations from same file as initially loaded

##### `--logger-id <logger_id>`
Reloads server configuration and reconfigure loggers

### `reset`
Resets the server with following things (be careful in doing this as it may affect connected clients)

 1. Reloads configurations
 2. Disconnect all the clients (and remove corresponding tokens)
 3. Disconnect active sessions

### `history`
Display history of valid commands (default maximum = 5)

### `clients`
Number of connected clients (dead or alive) that have not yet been removed from the server

##### `clean`
Runs client integrity task and cleans all the dead clients

##### `list`
Lists all the connected clients (and `DEAD` status if they're dead)

###### `--with-key`
List associated key with the rest of the details

### `tokens`
Lists all the tokens for selected client (and `EXPIRED` status if they're expired).

##### `--client-id <client-id>`
The client ID to see tokens for

### `rotate`
Manually run log rotator for specified logger

##### `--logger-id <logger-id>`
The logger ID to run log rotation for.

#### `[--ignore-archive]`
If this option is set the archival task will not run and it will just copy the logs to backup directory.

#### `[--check-only]`
Only check the schedule for this logger rotation

### `stats`
Displays server stats and number of active sessions

##### `list`
Lists for active sessions (received, sent and how long session has been active for and associated clients if registered)

##### `--client-id <client-id>`
Filters stats for specified client. Some of the clients may not be listed as they're only registered when server receives anything from them.

### `files`
Displays list of logging files for speicified client, logger or levels

##### `--client-id <id>`
Client to display files for (all loggers if no `--logger-id` specified)

##### `--logger-id <id>`
Limit the list to specified logger id (for specified client)

##### `--levels <levels>`
Comma seperated logging levels, e.g, `info,error`
