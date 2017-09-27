<p align="center">
   ﷽
</p>

# CLI Commands
When residue server starts it can accept some commands from command-line interface. This document lists these commands.

Please note, some of these commands can also be triggered using admin requests.

### `quit`
Quits the server gracefully

### `reload`
Reloads configurations from same file as initially loaded

### `license`
Checks for subscription and current license. It is safe to do so and won't kill the server if your license is already expired, however, in such case you won't be able to restart the server. (Reloading should be OK)

### `reset`
Resets the server with following things (be careful in doing this as it may affect connected clients)

 1. Reloads configurations
 2. Disconnect all the clients (and remove corresponding tokens)
 3. Disconnect active sessions

### `history`
Display history of valid commands (default maximum = 5)

### `stats`
Displays server stats

### `clients`
Number of connected clients (dead or alive)

##### `remove --client-id <client-id>`
Removes the existing client. Please be careful with this command. If client has unprocessed requests it may crash.

##### `add --client-id <id> --rsa-public-key-file <rsa_key>`
Adds new client with specific id and public key.

The public key should exist on the server

##### `list`
Lists all the connected clients (and `DEAD` status if they're dead)

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

### `sess`
Number of active sessions

##### `--stats`
Displays stats for active sessions (received, sent and how long session has been active for)


### `stats`
Displays server stats

### `files`
Displays list of logging files for speicified client, logger or levels

##### `--client-id <id>`
Client to display files for (all loggers if no `--logger-id` specified)

##### `--logger-id <id>`
Limit the list to specified logger id (for specified client)

##### `--levels <levels>`
Comma seperated logging levels, e.g, `info,error`
