<p align="center">
   ﷽
</p>

# Configuration

This document gives you details on configuring residue

Residue is fully configurable to support various features and for security. Configuration is always in [JSON](http://json.org/) format.

You can use [Server Config Tool](https://muflihun.github.io/residue/create-server-config) to generate configurations for your server

## Table of Contents

* [admin_port](#admin_port)
* [connect_port](#connect_port)
* [logging_port](#logging_port)
* [default_key_size](#default_key_size)
* [server_key](#server_key)
* [server_rsa_private_key](#server_rsa_private_key)
* [server_rsa_public_key](#server_rsa_public_key)
* [server_rsa_secret](#server_rsa_secret)
* [enable_cli](#enable_cli)
* [allow_insecure_connection](#allow_insecure_connection)
* [allow_unknown_loggers](#allow_unknown_loggers)
* [allow_unknown_clients](#allow_unknown_clients)
* [immediate_flush](#immediate_flush)
* [requires_timestamp](#requires_timestamp)
* [compression](#compression)
* [file_mode](#file_mode)
* [allow_bulk_log_request](#allow_bulk_log_request)
* [max_items_in_bulk](#max_items_in_bulk)
* [timestamp_validity](#timestamp_validity)
* [client_age](#client_age)
* [non_acknowledged_client_age](#non_acknowledged_client_age)
* [client_integrity_task_interval](#client_integrity_task_interval)
* [dispatch_delay](#dispatch_delay)
* [archived_log_directory](#archived_log_directory)
* [archived_log_filename](#archived_log_filename)
* [archived_log_compressed_filename](#archived_log_compressed_filename)
* [known_clients](#known_clients)
   * [client_id](#known_clientsclient_id)
   * [public_key](#known_clientspublic_key)
   * [key_size](#known_clientskey_size)
   * [loggers](#known_clientsloggers)
   * [default_logger](#known_clientsdefault_logger)
   * [user](#known_clientsuser)
* [known_clients_endpoint](#known_clients_endpoint)
* [known_loggers](#known_loggers)
   * [logger_id](#known_loggerslogger_id)
   * [configuration_file](#known_loggersconfiguration_file)
   * [rotation_freq](#known_loggersrotation_freq)
   * [user](#known_loggersuser)
   * [archived_log_filename](#known_loggersarchived_log_filename)
   * [archived_log_compressed_filename](#known_loggersarchived_log_compressed_filename)
   * [archived_log_directory](#known_loggersarchived_log_directory)
* [known_loggers_endpoint](#known_loggers_endpoint)
* [extensions](#extensions)
* [loggers_blacklist](#loggers_blacklist)
* [Comments](#comments)
* [Sample](#sample)
      
  
### `admin_port`
[Integer] Port that admin server listens to. All the admin requests are sent to this port. These requests must be encrypted using [`server_key`](#server_key).

Default: `8776`

[Learn more...](/docs/configurations/admin_port.md)

### `connect_port`
[Integer] Port that connection server listens to. All the connection requests are sent to this port and handled accordingly. This is the port that all the clients libraries initially connect to.

Default: `8777`

[Learn more...](/docs/configurations/connect_port.md)

### `logging_port`
[Integer] Port that logging server listens to. This is where all the log requests are sent.

Default: `8778`

[Learn more...](/docs/configurations/logging_port.md)

### `default_key_size`
[Integer] Default symmetric key size (`128`, `192` or `256`) for clients that do not specify key size. See [`key_size`](#known_clientskey_size)

Default: `256`

### `server_key`
[String] A 256-bit server key that is used for admin services. See [`admin_port`](#admin_port)

Default: Randomly generated and residue outputs it in start-up

### `server_rsa_private_key`
[Optional, String] RSA private key (PEM format file path). You can use `$RESIDUE_HOME` environment variable in this file path. If provided, it is used to read initial requests for extra security.

[Learn more...](/docs/configurations/server_rsa_private_key.md)

### `server_rsa_public_key`
[String] Corresponding public key for [`server_rsa_private_key`](#server_rsa_private_key). You can use `$RESIDUE_HOME` environment variable in this file path.

[Learn more...](/docs/configurations/server_rsa_public_key.md)

### `server_rsa_secret`
[String] If private key is encrypted, this is the secret (passphrase) to decrypt it. **THIS SHOULD BE HEX ENCODED**.

[Learn more...](/docs/configurations/server_rsa_secret.md)

### `enable_cli`
[Boolean] Whether CLI is enabled on server or not. See [CLI_COMMANDS.md](/docs/CLI_COMMANDS.md) for possible input commands.

Default: `true`

### `allow_insecure_connection`
[Boolean] Specifies whether plain connections to the server are allowed or not. Either this should be true or server key pair must be provided (or both)

Default: `true`

### `allow_unknown_loggers`
[Boolean] Specifies whether loggers other than the ones in [`known_loggers`](#known_loggers) list are allowed or ignored / rejected.

Default: `true`

### `allow_unknown_clients`
[Boolean] Specifies whether clients other than the ones in [`known_clients`](#known_clients) list are allowed or ignored / rejected.

Default: `true`

### `immediate_flush`
[Boolean] Specifies whether to flush logger immediately after writing to it or not. Performance of the server is not affected with turning it on as we use separate thread for logging.

Default: `true`

### `requires_timestamp`
[Boolean] Specifies whether timestamp is absolutely required or not. Timestamp is `_t` value for each incoming requests.

This does not affect admin requests, timestamps for admin requests is always required.

See [`timestamp_validity`](#timestamp_validity)

Default: `true`

### `compression`
[Boolean] Specifies whether compression is enabled or not.

Default: `true`

[Learn more...](/docs/configurations/compression.md)

### `file_mode`
[Integer] Default file mode for the log files

[Learn more...](/docs/configurations/file_mode.md)

### `allow_bulk_log_request`
[Boolean] Specifies whether clients can send bulk log requests or not.

See [`max_items_in_bulk`](#max_items_in_bulk) and [`compression`](#compression)

Default: `true`

### `max_items_in_bulk`
[Integer] Maximum number of bulk items allowed.

If client sends more requests than this all the extra requests are ignored by server (with verbose warning). User is not informed about this as we handle log requests asynchronously.

Default: `5`

You may be interested in [`compression`](#compression)

### `timestamp_validity`
[Integer] Integer value in seconds that specifies validity of timestamp `_t` in request

Minimum: `30`

Default: `120`

### `client_age`
[Integer] Value (in seconds) that defines the age of a client. After this age, client is considered _dead_. Clients library can `TOUCH` request just before (subject to `TOUCH_THRESHOLD` value in the library) dying to bring it back to life provided it's not already dead. After client is dead, it needs to reconnect and obtain a new key.

Default: `259200` (3 days)

Minimum: `120`

Forever: `0` (not recommended)

### `non_acknowledged_client_age`
[Integer] Value (in seconds) that defines the age of a client that is not yet acknowledged by client library.

Note: You cannot `TOUCH` a non-acknowledged client.

Minimum: `120`

Default: `300` (5 minutes)

Cannot set it to *forever*

### `client_integrity_task_interval`
[Integer] Value that should be >= `client_age` or `non_acknowledged_client_age` (whichever is lower).

This is a task that ensures integrity of the clients to remove dead clients that are unusable

Default: `300` or `min(client_age, non_acknowledged_client_age)` [whichever is higher]

Minimum: `300`

### `dispatch_delay`
[Integer] Value that defines the delay (in milliseconds) between two log message dispatch.

**Note** Increasing this value is going to slow down your server (for obvious reasons, i.e, it's a delay).

Default: `1` (recommended)

Maximum: `500`

Turn off delay: `0` (not recommended)

### `archived_log_directory`
[String] Default destination for archived logs files

Possible format specifiers:

|   Format specifier    |   Description   |
|-----------------------|-----------------|
| `%original` | Path to original log file |
| `%logger` | Logger ID |
| `%hour` | 24-hours zero padded hour (`09`, `13`, `14`, ...) |
| `%wday` | Day of the week (`sun`, `mon`, ...) |
| `%day` | Day of month (`1`, `2`, ...) |
| `%month` | Month name (`jan`, `feb`, ...) |
| `%quarter` | Month quarter (`Q1`, `Q2`, `Q3`, `Q4`) |
| `%year` | Year (`2016`, `2017`, `2018`, ...) |

Default: It must be provided by the user

Example: `%original/backups/%logger/`

### `archived_log_filename`
[String] Default filename for archived log files.

Possible format specifiers:

|   Format specifier    |   Description   |
|-----------------------|-----------------|
| `%logger`| Logger ID |
| `%min`| Zero padded hour (`09`, `13`, `14`, ...) - the time when log rotator actually ran |
| `%hour`| 24-hours zero padded hour (`09`, `13`, `14`, ...) |
| `%wday`| Day of the week (`sun`, `mon`, ...) |
| `%day`| Day of month (`1`, `2`, ...) |
| `%month`| Month name (`jan`, `feb`, ...) |
| `%quarter`| Month quarter (`Q1`, `Q2`, `Q3`, `Q4`) |
| `%year`| Year (`2017`, ...) |
| `%level`| log level (`info`, `error`, ...) |

Default: It must be provided by the user

Example: `%level-%hour-%min-%day-%month-%year.log`

### `archived_log_compressed_filename`
[String] Filename for compressed archived log files. It should not contain `/` or `\` characters.

Possible format specifiers:

|   Format specifier    |   Description   |
|-----------------------|-----------------|
| `%logger` | Logger ID |
| `%hour` | 24-hours zero padded hour (`09`, `13`, `14`, ...) |
| `%wday` | Day of the week (`sun`, `mon`, ...) |
| `%day` | Day of month (`1`, `2`, ...) |
| `%month` | Month name (`jan`, `feb`, ...) |
| `%quarter` | Month quarter (`Q1`, `Q2`, `Q3`, `Q4`) |
| `%year` | Year (`2017`, ...) |

Default: It must be provided by the user

Example: `%hour-%min-%day-%month-%year.tar.gz`

### `known_clients`
[Array] Object of client that are known to the server. These clients will have allocated RSA public key that will be used to transfer the symmetric key.

[Learn more...](/docs/configurations/known_clients/#known_clients)

#### `known_clients`::`client_id`
[String] The client ID (should be alpha-numeric and can include `-`, `_`, `@`, `.` and `#` symbols)

[Learn more...](/docs/configurations/known_clients/client_id.md)

#### `known_clients`::`public_key`
[String] Path to RSA public key file for associated client ID. This key must be present and readable at the time of starting the server.

You can use `$RESIDUE_HOME` environment variable in this file path.

[Learn more...](/docs/configurations/known_clients/public_key.md)

#### `known_clients`::`key_size`
[Optional, Integer] Integer value of `128`, `192` or `256` to specify key size for this client.

This is useful when client libraries connecting cannot handle bigger sizes, e.g, java clients without JCE Policy Files.

See [`default_key_size`](#default_key_size)

#### `known_clients`::`loggers`
[Optional, Array] Object of logger IDs that must be present in [`known_loggers`](#known_loggers) array.

This is to map the client with multiple loggers. Remember, client is not validated against the logger using this array, this is only for extra information.

#### `known_clients`::`default_logger`
[String] Default logger for the client. This is useful when logging using unknown logger but connected as known client. The configurations from this logger is used.

Default: `default`

See [`known_clients::loggers`](#known_clientsloggers)

#### `known_clients`::`user`
[String] Linux / macOS user. All the log files associated to the corresponding loggers will belong to this user with `RW-R-----` permissions (subject to `file_mode` config)

Default: Current process user

[Learn more...](/docs/configurations/known_clients/user.md)

### `known_clients_endpoint`
[String] This is URL where we can pull *more* known clients from. The endpoint should return JSON with object [`known_clients`](#known_clients), e.g,

```
<?php
header('Content-Type: application/json');
$list = array(
    "known_loggers" => array(
        array(
            "logger_id" => "another",
            "configuration_file" => "samples/configurations/blah.conf"
        ),
    ),
);

echo json_encode($list);
```

Call this file `localConfig.php` and run it as server `php -S localhost:8000`

Endpoint URL is `http://localhost:8000/localConfig.php`

You need to make sure that [`configuration_file`](#configuration_file) exists on the server.

### `known_loggers`
[Array] Object of loggers that are known to the server. 

#### `known_loggers`::`logger_id`
[String] The logger ID

#### `known_loggers`::`configuration_file`
[String] Path to [Easylogging++ configuration file](https://github.com/muflihun/easyloggingpp#using-configuration-file). You can use `$RESIDUE_HOME` environment variable in this file path. When the new logger is registered, it's configured using this configuration.

Residue supports following [custom format specifiers](https://github.com/muflihun/easyloggingpp#custom-format-specifiers):

| Format Specifier | Description |
| ---------------- | ----------- |
| `%client_id`     | Print client ID with specified log format / level |
| `%ip`     | IP address of the request |
| `%session_id`     | Current session ID |
| `%vnamelevel`     | Verbose level name (instead of number) |

Verbose level names are:

| Verbose Level | Name |
| ---------------- | ----------- |
| 9 | `vCRAZY` |
| 8 | `vTRACE` |
| 7 | `vDEBUG2` |
| 6 | `vDEBUG` |
| 5 | `vDETAILS` |
| 4 | `vERROR` |
| 3 | `vWARNING` |
| 2 | `vNOTICE` |
| 1 | `vINFO` |

##### Configuration Errors
***

You're not allowed to use following configurations in your configuration file as residue handles these configurations differently and ignore your configurations. Residue will not start until these configuration lines are removed. This is so that user does not expect anything from these configuration items.

 * `Max_Log_File_Size`
 * `To_Standard_Output`
 * `Log_Flush_Threshold`

#### `known_loggers`::`rotation_freq`
[String] One of [`never`, `hourly`, `six_hours`, `twelve_hours`, `daily`, `weekly`, `monthly`, `yearly`] to specify rotation frequency for corresponding log files. This is rotated regardless of file size.

Log rotation rounds off to the nearest denominator. To get better understanding of this round off, consider following table.

|   Frequency        |       Next Schedule                                      |
|--------------------|----------------------------------------------------------|
| `hourly`           | Last second of hour i.e, `<hour>:59:59`                  |
| `six_hours`        | Every 6 hours i.e, at `06:00`, `12:00`, `18:00`, `00:00` |
| `twelve_hours`     | Every 12 hours, i.e, at `12:00`, `00:00`                 |
| `daily`            | Last second of the day i.e, `23:59:59`                   |
| `weekly`           | Each sunday at `23:59:59`                                |
| `monthly`          | Last day of each month at `23:59:59`                     |
| `yearly`           | Last day of the each year at `23:59:59`                  |

Default: `never`

#### `known_loggers`::`user`
[String] Linux / mac user assigned to known logger. All the log files associated to the corresponding logger will belong to this user with `RW-R-----` permissions

Default: Current process user

#### `known_loggers`::`archived_log_filename`
[String] Filename for rotated or archived log file

See [archived_log_filename](#archived_log_filename)

#### `known_loggers`::`archived_log_compressed_filename`
[String] Filename for rotated or archived log file in compressed form (ideally ending with `.tar.gz`)

See [archived_log_compressed_filename](#archived_log_compressed_filename)

#### `known_loggers`::`archived_log_directory`
[String] Overrides default [`archived_log_directory`](#archived_log_directory) to logger specific directory.

See [archived_log_directory](#archived_log_directory)

### `known_loggers_endpoint`
[String] This is URL same as [`known_clients_endpoint`](#known_clients_endpoint) with JSON object containing same properties as [`known_loggers`](#known_loggers)

### `extensions`
[Array] Contains various types of extensions. You can specify each type of extension's array shared library modules.

Extensions API documentation is available [here](https://muflihun.github.io/residue/extensions/)

**NOTE You will need `libresidue-extension` in your `LD_LIBRARY_PATH`**

### `loggers_blacklist`
[Array] String where each string is logger ID. Whenever request using these loggers are received, they are ignored without notifying the user.

## Comments
Because Residue configuration is in JSON format, comments are not supported as there should be no comment in JSON file. With that said there are ways to have comments around it.

1. You can have a seperate "object" for comment e.g,

```javascript
...
    "comment", "following backup directory will be in original logging directory",
    "archived_log_directory": "%original/backups/%logger/",
    "comment", "another comment",
    "archived_log_filename": "%hour-%min-%day-%month-%year.log",
    "comment", "another comment",
    "archived_log_compressed_filename": "%hour-%min-%day-%month-%year.tar.gz",
    "known_clients": [
...
```

Please note, this is not preferred way as parsing may get slower with increasing comments.

2. You can have comment using `//` or `/* */` in the file and then before you pass it on to residue, run it through jsMin. e.g, for file `test.json`

```javascript
...
    // following backup directory will be in original logging directory
    "archived_log_directory": "%original/backups/%logger/",
    // another comment
    "archived_log_filename": "%hour-%min-%day-%month-%year.log",
    // another comment
    "archived_log_compressed_filename": "%hour-%min-%day-%month-%year.tar.gz",
    "known_clients": [
...
```

```
cat test.json | jsmin > test-nocomments.json && residue test-nocomments.json
```

**This is preferred way** as it will only pass on what is really needed.

## Sample
Please refer to [sample configuration file](/samples/residue.conf.json) for understanding it better.
