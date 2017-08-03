# Change Log

## [1.0.0-beta.14] - 03-08-2017
### Library Changes
- Fixed issue with logging DEBUG when built with Release build (use Easylogging++ v9.95.0+)
- Use of `CHECK_TOKENS` server flag to reduce overhead of pulling token when not needed
- Ability to re-estabilish connection if disconnected from remote
- Increased `TOUCH_THRESHOLD` to 2 minutes

## [1.0.0-beta.13] - 02-08-2017
### Updated
- Removed `allow_pinging_dead_client` config
- New configs `timestamp_validity` and `requires_timestamp`

### Changed
- Changed `authorize_loggers_with_no_access_code` to `allow_default_access_code`

### Fixed
- Healing the connection will respond with flags and other connection response items
- Fixed release build

### Library Changes
- Fixed compression flag
- Internal logging level helper enum class

## [1.0.0-beta.12] - 27-07-2017
### Added
- Ability to specify key size for each client
- Added `max_token_age` configuration

### Updated
- Performance update for I/O operation on public key
- Default token age for known loggers is now value of global `token_age` configuration
- `server_rsa_secret` should be hex-encoded value now

### Library Changes
- Ability to set internal logging level via configuration using `internal_logging_level`
- Fixed issue with pinging client when client_age < 60

## [1.0.0-beta.11] - 22-07-2017
### Library Changes
- Provide RSA key secret with `secret_key`

### Fixes
- Fixed issue with plain connection

## [1.0.0-beta.10] - 21-07-2017
### Added
- New configuration `known_clients_endpoint` and `known_loggers_endpoint`

### Fixed
- Issue with clean start and reloading configuration fixed

## [1.0.0-beta.9] - 16-07-2017
### Added
- Loggers map for `known_clients`

### Changes
- Changed `CHECK_LOGGING_FILE` to support searching by multiple logging levels and by `client_id`
- Removed references for `max_file_size` as we do not need it because of log rotation.

### Library Changes
- Doc fix for `setKeySize`

## [1.0.0-beta.8] - 15-07-2017
### Added
- New admin request `CHECK_LOGGING_FILE` to get known logger file for specified level

### Fixes
- Issue with timestamp validation check when timestamp is newer than date received

### Changes
- Changed licence to Apache 2.0

## [1.0.0-beta.7] - 14-07-2017
### Fixes
- Fix logging failure issue from some client libraries
- Build with `g++-5`

## [1.0.0-beta.6] - 13-07-2017
### Fixes
- Prevent replay attack using `_t` request item
- Some inaccessible variables issue fixes

## [1.0.0-beta.5] - 12-07-2017
### Added
- `accept_input` configuration

## [1.0.0-beta.4] - 07-07-2017
### Library Changes
- Fixed dead lock on `reset()`

### Added
- Fixed admin request handler and decryption
- Ability to save configuration
- Ability to add and remove known clients using admin requests
- Ability to reset the server using CLI
- Ability to reset the server using admin request

## [1.0.0-beta.3] - 09-05-2017
### Library Changes
- Fixed issue with failing to connect to token and/or logging server. Now throws exception
- Error text on failure
- Fixed exception throwing in `connect()`
- Fixed issue with re-connecting broken socket

### Changes
- Fixed typo in documentation

## [1.0.0-beta.2] - 20-04-2017
### Library Changes
- Ability to specify server public key
- By default `AutoBulkParams` is now enabled
- Added `Residue::setThreadName` (wrapper for `el::Helpers::setThreadName`)
- Added `Residue::setInternalLoggingLevel` for internal logging
- Added `Residue::setApplicationArgs` (wrapper for `START_EASYLOGGINGPP`)
- Added `Residue::reconnect()`
- Added `Residue::moveAccessCodeMap`
- Added `Residue::connect(host, port)` without access code map to be able to connect to different host using existing map
- Added `Residue::enableCrashHandler`
- Added JSON configuration helper `Residue::loadConfiguration`

### Added
- Ignore interrupt signal with message to use `quit` command
- Quarter and year units for log rotation
- Added `default_key_size` configuration

### Fixed
- Log successful with dead client #24
- Resetting connection deadlock #25
- [api] Obtain token and pinging fails #26
- [api] Fixed issue with `disconnect()` and `connect()` again
- Dead lock on client integrity task
- Added mutex for session join/leave to prevent race-condition and crash

### Changes
- Removed `DISCONNECT` and `SAFE_DISCONNECT` connection types
- Moved node client to separate repo
- Known client ID can only be alpha-numeric with few exceptional symbols

## [1.0.0-beta] - 31-03-2017
### Added
- Send server flags with final connection #14
- Support sending plain log requests in lib #13
- Code refactor #4
- Support compression #19
- Compression for archived log files should be async process #2
- New helper `enableAutoBulkParams` to support auto setting bulk parameters
- Implement archived log compression #1

### Changes
- Ignore archiving empty log files #20

### Fixed
- Issue with dead client and resetting connection caused issue with dispatcher thread in client lib #22

## [1.0.0-alpha] - 19-03-2017
### Added
- Initial alpha release
