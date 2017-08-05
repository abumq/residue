                                       ‫بسم الله الرَّحْمَنِ الرَّحِيمِ

## Configuration
This document gives you details on configuring residue

Residue is fully configurable to support various features and for security. Configuration is always in [JSON](http://json.org/) format.

You can use [Server Config Tool](https://muflihun.github.io/residue/create-server-config) to create configurations for your server

### `admin_port`
[Integer] Port that admin server listens to. All the admin requests are sent to this port. These requests must be encrypted using [`server_key`](#server_key).

Default: `8776`

### `connect_port`
[Integer] Port that connection server listens to. All the connection requests are sent to this port and handled accordingly. This is the port that all the clients libraries initially connect to.

Default: `8777`

### `token_port`
[Integer] Port that token server listens to. If loggers require a token to use it, this is where client will obtain the token.

Default: `8778`

### `logging_port`
[Integer] Port that logging server listens to. This is where all the log requests are sent.

Default: `8779`

### `default_key_size`
[Integer] Default symmetric key size (`128`, `192` or `256`) for clients that do not specify key size. See [`key_size`](#known_clientskey_size)

Default: `256`

### `server_key`
[String] A 256-bit server key that is used for admin services. See [`admin_port`](#admin_port)

Default: Randomly generated and residue outputs it in start-up

### `server_rsa_private_key`
[Optional, String] RSA private key (PEM format file path). If provided, it is used to read initial requests for extra security.

Note: You should have big enough key to cover for unknown clients. Remember, unknown clients will need to send their public key in initial request, which makes request quite bigger.

You can use following ripe command to generate the key (this will generate 8192-bit key which can encrypt 1013 bytes to data)

```
ripe -g --rsa --length 8192 --out-public server-public.pem --out-private server-private.pem
```

Alternatively, you can use openssl command-line tool.

See [Ripe](https://github.com/muflihun/ripe#readme) for more details.

### `server_rsa_public_key`
[String] Corresponding public key for [`server_rsa_private_key`](#server_rsa_private_key)

### `server_rsa_secret`
[String] If private key is encrypted, this is the secret (passphrase) to decrypt it. **THIS SHOULD BE HEX ENCODED**.

### `accept_input`
[Boolean] Whether server accepts input from user or not. See [CLI_COMMANDS.md](/docs/CLI_COMMANDS.md) for possible input commands.

Default: `true`

### `allow_plain_connection`
[Boolean] Specifies whether plain connections to the server are allowed or not. Either this should be true or server key pair must be provided (or both)

Default: `true`

### `allow_default_access_code`
[Boolean] Specifies whether all the loggers with no access codes are automatically allowed or not. If allowed, Easylogging++ default configurations are used.

Default: `true`

### `allow_unknown_loggers`
[Boolean] Specifies whether loggers other than the ones in [`known_loggers`](#known_loggers) list are allowed or ignored / rejected.

Default: `true`

### `allow_unknown_clients`
[Boolean] Specifies whether clients other than the ones in [`known_clients`](#known_clients) list are allowed or ignored / rejected.

Default: `true`

### `check_tokens`
[Boolean] Turn on/off token checking with each log request.

Default: `true`

### `immediate_flush`
[Boolean] Specifies whether to flush logger immediately after writing to it or not. Performance of the server is not affected with turning it on as we use separate thread for logging.

Default: `true`

### `requires_timestamp`
[Boolean] Specifies whether timestamp is absolutely required or not. Timestamp is `_t` value for each incoming requests.

This does not affect admin requests, timestamps for admin requests is always required.

See [`timestamp_validity`](#timestamp_validity)

Default: `false`

### `compression`
[Boolean] Specifies whether compression is enabled or not.

You should note few points:

 * If this is enabled client libraries should take advantage of it and send compressed data. However, if it doesn't, request is still processed.
 * Only log requests should be compressed, other requests (connection, token, touch etc) are sent normally.
 * Outgoing data (from server) is never compressed.

Compression has great affect and can save big data. We recommend you to enable compression in your server. Just to give you little bit of idea, when we run [simple example project](/samples/clients/c%2B%2B/detailed-cmake) to log 1244 times, without compression data transferred was `488669 bytes` and with compression it was `44509 bytes`. Same data transferred has same performance with high reliability.

It always has very good performance when you have [`compression`](#compression) and [`allow_bulk_log_request`](#allow_bulk_log_request) both active. In one of our big test we had following result:

 * With compression and bulk (size: 20): `24100423 bytes`
 * Without compression and bulk (size: 20): `327100683 bytes` (more than 92.6% of less-data transferred)
 * Without compression and non-bulk request: `398001463 bytes`

This is because lossless-compression is done on similar bytes. If you wish to know more about compression algorithm see [gzlib algorithm](www.gzip.org/algorithm.txt).

Default: `true`

### `allow_plain_log_request`
[Boolean] Specifies whether to allow clients to send plain log requests or not.

We recommend that you keep it disabled as it has some security implications, i.e, tokens can be compromised by party in the middle. Thus they can send unauthorised log requests. If this is enabled, we highly recommend you do allow loggers with forever lifetime and you keep [`token_age`](#token_age) to minimum (and not `0` which is "forever"), e.g, 300 (5 minutes)

Default: `false`

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
[Integer] Value (in seconds) that defines the age of a client. After this age, client is considered *dead*. Clients library can `TOUCH` request just before (subject to `TOUCH_THRESHOLD` value in the library) dying to bring it back to life provided it's not already dead. After client is dead, it needs to reconnect and obtain a new key.

Default: `259200` (3 days)

Minimum: `120`

Forever: `0` (not recommended)

### `non_acknowledged_client_age`
[Integer] Value (in seconds) that defines the age of a client that is not yet acknowledged by client library.

Note: You cannot `TOUCH` a non-acknowledged client.

Minimum: `120`

Default: `300` (5 minutes)

Cannot set it to *forever*

### `max_token_age`
[Integer] Value (in seconds) that defines maximum age of the token

Minimum: `15`

Default: `0` (Forever)

### `token_age`
[Integer] Value (in seconds) that defines default age of the token.

This age is used in following scenarios:

 * If [`allow_default_access_code`](#allow_default_access_code) is `true` and token with unknown access code is requested, this is the age of the token.
 * If no access code is specified with access code, this is the age of that token.

Minimum: `15`

Maximum: [`max_token_age`](#max_token_age)

Default: `3600` or `max_token_age` (which ever is smaller)

Forever: `0` (not recommended)

### `client_integrity_task_interval`
[Integer] Value that should be >= `client_age` or `non_acknowledged_client_age` (whichever is lower).

This is a task that ensures integrity of the clients to remove dead clients that are unusable (and tokens accordingly)

Default: 300 or min(`client_age`, `non_acknowledged_client_age`) [whichever is higher]

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

 * `%original`: Path to original log file
 * `%logger`: Logger ID
 * `%hour`: 24-hours zero padded hour (`09`, `13`, `14`, ...)
 * `%wday`: Day of the week (`sun`, `mon`, ...)
 * `%day`: Day of month (`1`, `2`, ...)
 * `%month`: Month name (`jan`, `feb`, ...)
 * `%quarter`: Month quarter (`1`, `2`, `3`, `4`)
 * `%year`: Year (`2017`, ...)
 * `%level`: log level (`info`, `error`, ...)

Default: It must be provided by user

### `archived_log_filename`
[String] Default filename for archived log files.

Possible format specifiers:

 * `%logger`: Logger ID
 * `%hour`: 24-hours zero padded hour (`09`, `13`, `14`, ...)
 * `%wday`: Day of the week (`sun`, `mon`, ...)
 * `%day`: Day of month (`1`, `2`, ...)
 * `%month`: Month name (`jan`, `feb`, ...)
 * `%quarter`: Month quarter (`1`, `2`, `3`, `4`)
 * `%year`: Year (`2017`, ...)
 * `%level`: log level (`info`, `error`, ...)

Default: It must be provided by user.

### `archived_log_compressed_filename`
[String] Filename for compressed archived log files. It should not contain `/` or `\` characters.

Possible format specifiers:

 * `%logger`: Logger ID
 * `%hour`: 24-hours zero padded hour (`09`, `13`, `14`, ...)
 * `%wday`: Day of the week (`sun`, `mon`, ...)
 * `%day`: Day of month (`1`, `2`, ...)
 * `%month`: Month name (`jan`, `feb`, ...)
 * `%quarter`: Month quarter (`1`, `2`, `3`, `4`)
 * `%year`: Year (`2017`, ...)

Default: It must be provided by user.

### `known_clients`
[Array] Object of client that are known to the server. These clients will have allocated RSA public key that will be used to transfer the symmetric key.

#### `known_clients`::`client_id`
[String] The client ID (should be alpha-numeric and can include `-`, `_`, `@` and `#` symbols)

#### `known_clients`::`public_key`
[String] Path to RSA public key file for associated client ID. This key must be present and readable at the time of starting the server.

#### `known_clients`::`key_size`
[Optional, Integer] Integer value of `128`, `192` or `256` to specify key size for this client.

This is useful when client libraries connecting cannot handle bigger sizes, e.g, java clients without JCE Policy Files.

See [`default_key_size`](#default_key_size)

#### `known_clients`::`loggers`
[Optional, Array] Object of logger IDs that must be present in [`known_loggers`](#known_loggers) array.

This is to map the client with multiple loggers. Remember, client is not validated against the logger using this array, this is only for extra information.

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
[Array] Object of loggers that are known to the server. You can use this to provide extra security and setup [`access_codes`](#access_codes) for each logger (to obtain the token and their respective lifetime)

#### `known_loggers`::`logger_id`
[String] The logger ID

#### `known_loggers`::`configuration_file`
[String] Path to [Easylogging++ configuration file](https://github.com/muflihun/easyloggingpp#using-configuration-file). When the new logger is registered, it's configured using this configuration.

Residue supports following [custom format specifiers](https://github.com/muflihun/easyloggingpp#custom-format-specifiers):

| Format Specifier | Description |
| ---------------- | ----------- |
| `%client_id`     | Print client ID with specified log format / level |
| `%ip`     | IP address of the request |
| `%vnamelevel`     | Verbose level name (instead of number) |

#### `known_loggers`::`access_codes`
[Array] Object that is defined as `{"code" : "<string of any length>", "token_age" : <integer>}`.

See [known_loggers::access_codes::code](#known_loggersaccess_codescode) and [known_loggers::access_codes::token_age](#known_loggersaccess_codestoken_age)

#### `known_loggers`::`access_codes`::`code`
[String] The access code require to get the token for this logger

#### `known_loggers`::`access_codes`::`token_age`
[Optional, Integer] Age of token (in seconds) generated as a result of this access code. Do not add it if you want it to stay alive forever.

Default: [`token_age`](#token_age)

Maximum: [`max_token_age`](#max_token_age)

#### `known_loggers`::`access_codes_blacklist`
[Array] String that define black listed access codes. You may access code to this list if one of the access codes is compromised. (alternatively, you may remove from the original list)

#### `known_loggers`::`allow_plain_log_request`
[Boolean] Value that allows users to send plain log requests if server level [`allow_plain_log_request`](#allow_plain_connection) configruation is enabled.

Default: `false`

#### `known_loggers`::`rotation_freq`
[String] One of [`never`, `hourly`, `daily`, `weekly`, `monthly`, `quarterly`, `yearly`] to specify rotation frequency for corresponding log files. This is rotated regardless of file size.

Default: `never`

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

### `loggers_blacklist`
[Array] String where each string is logger ID. Whenever request using these loggers are received, they are ignored without notifying the user.

## Sample
Please refer to [sample configuration file](/samples/residue.conf.json) for understanding it better.
