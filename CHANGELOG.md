# Change Log

## [Unreleased]
### Updates
- Removed ability to dynamically add/remove client #107

## [1.6.1] - 28-02-2018
### Updates
- New format specifier `%session_id`
- Client integrity tasks are paused for specific clients being processed instead of all clients
- Unknown client trying to use known logger using valid access code is no longer allowed

## [1.6.0] - 27-02-2018
### Updates
- Separate log dispatchers and logging queue by clients #96
- Some configurations now support `RESIDUE_HOME` environment variable
- Known loggers configured at start up
- Client ID now supports `.` character
- Introduced `_close` for closing connection immediately after response

### Fixes
- Fix rotation locks
- `--force-without-root` message fix at startup
- Fix standard responses if initial check on log request failed

## [1.5.0] - 23-02-2018
### Updates
- Upgraded Easylogging++ to 9.96.1
- Implemented file locks for updated logging library

### Changes
- `ALLOW_PLAIN_LOG_REQUEST` removed #97
- Changed `ALLOW_PLAIN_CONNECTION` to `ALLOW_INSECURE_CONNECTION` to be more clear

## [1.4.5] - 22-02-2018
### Updates
- Configuration saving is not pretty-printed
- Server requires timestamp by default

## [1.4.4] - 20-02-2018
### Fixes
- Fix output help from `--verbose` to `--v`
- Bulk items do not have correct translations for unsigned long #88

## [1.4.3] - 17-02-2018
### Updates
- Performance improvement using standard response
- Changed `access_codes_blacklist` to `access_code_blacklist`
- Changed `ACCEPT_INPUT` renamed to `ENABLE_CLI`

### Fixes
- Crash detected when writing to session #79
- Fixed undefined behaviour with destroyed sessions 

## [1.4.2] - 14-02-2018
### Fixes
- Client integrity task can remove pending dead client #73
- Deadlock detected while dispatching log #76
- Race condition causing deadlock when creating log file #77
- Long pending requests fail after dead client's key is reset #75

### Updates
- Upgraded underlying logging library Easylogging++ from 9.95.0 to 9.96.0
- Moved some of the unnecessary logs from production-debug to dev-debug

## [1.4.1] - 12-02-2018
### Fixes
- Output for `sess --stats` fixed for multiple sessions
- Fix race condition on sessions when resetting

### Updates
- Session now has unique IDs
- Merged `sess` and `stats` in to single CLI command
- Changed `sess --stats` to `stats list`
- Ability to filter sessions by client IDs
- New option `--with-key` with `clients list`
- Changed `--logger` to `logger-id` in `rconfig` for consistency

## [1.4.0] - 11-02-2018
### Updates
- Based on standalone ASIO instead of boost

## [1.3.2] - 10-02-2018
### Fixes
- Fix crash when failed to process zlib operations
- Upgraded ripe library to 4.1.1

### Updates
- User agent for HTTP client changed from `Residue/x.x` to `muflihun/residue/x.x.x`

## [1.3.1] - 08-02-2018
### Fixes
- Fix incorrect message for checking client age

### Added
- Changed `reload` to `rconfig`
- Added ability to reconfigure logger using `--logger` with `rconfig`

## [1.3.0] - 06-02-2018
### Added
- New clients CLI option `clean`

### Fixes
- Log rotator task - daily, weekly, monthly and yearly schedule (issue #49)
- Improved the command line handler to support arrow keys and history using up key
- Log rotators that take longer to archive now correctly resolve subsequent format specifiers
- Log rotator did not resolve correct `%level` for destination directory
- Key is reset if client connected after client is dead before removal

### Updates
- Debug printed in server version in connection response if build has debug logs enabled
- Quarter format specifier now has `Q` prefix

### Internal
- Extra logging for debugging
- LogArchiver thread name starts with log rotator name for different frequencies

## [1.2.3] - 31-01-2018
### Updates
- Removed licensing
- Minor tweaks for performance

## [1.2.2]
### Added
- Extension support via python (disabled in official releases - see EXTENSIONS.md under `docs/`)

### Fixes
- Residue crash handler installed by default - all crashes will be logged via `residue` logger

### Updates
- _Context switching_ improved by extracting it seperately and only one lock at the time of switching

## [1.2.1] - 10-10-2017
### Updates
- Updated Ripe to 4.1.0

### Added
- `client_id` added to log files admin response

## [1.2.0] - 06-10-2017
### Changes
- Reset license keys to final new keys
- Moved server and license info to `server_info` object in response
- Added `license_expiry` in connection response
- Removed `v` from `server_info.version`

### Improvements
- Loggers and clients endpoints only called when available in configuration

### Fixes
- Log rotator rounds up to nearest hour
- Issue with loading license with signature fixed
- Logging files now lists default logger for client

## [1.1.0] - 01-10-2017
### Changes
- `LIST_LOGGING_FILES` admin request now returns logger - files map instead of flat list of files
- Mine is disabled for future releases until fully stable

### Fixed
- Admin request to list files runs correct command
- `LIST_LOGGING_FILES` shows files for all levels if not specified otherwise

## [1.0.0] - 27-09-2017
### Added
- License issuing authority in connection response

### Internals
- Linking of some libraries for portability
- Admin requests now calls commands using command handler
- License authority technical updates
- Concept of extensions

## [1.0.0-rc.2] - 05-09-2017
### Internals
- A lot of refactoring and wrapper for cryptography

## [1.0.0-rc] - 16-08-2017
### Fixes
- Attach crash handler opt
- Fixed up licensing authorities
- Fixed issue with license with signature not loading properly

## [1.0.0-beta.18] - 15-08-2017
### Fixes
- Fix issue with crashing on non-bulk requests

### Changes
- Re-structured the source code for scalability

## [1.0.0-beta.17] - 15-08-2017
### Fixes
- Critical fix to incorrect dereferencing making server even faster and more logging
- Fix to client checking after client integrity task execution

### Changes
- Updated logging levels for some logs

## [1.0.0-beta.16] - 13-08-2017
### Changes
- More logging for debugging
- `CHECK_TOKENS` changed to `REQUIRES_TOKEN`
- Bypass checks for same bulk if first check passed
- Log rotation for global level is not seperated with each levels rather it will use `GLOBAL`
- Changes file mode for logger files

### Added
- Ability to set custom default logger under `known_clients` for unknown logger configurations
- New command to remove client manually
- Run log rotator using CLI and admin requests
- Added licenseee in connection response

### Fixes
- Fixed log rotator to fix permissions for archived file
- Fixed user owner issue with known client (with user) and unknown logger
- Context switching for the queue has now threshold of 500 items
- Fixed issue with check client when processing to use date received instead of current date
- Critical bug with not being able to write verbose log is fixed
- Log rotation file naming issue fixed
- Issue with log rotation for loggers are not supposed to be rotated
- Fix log rotation for `residue` logger

## [1.0.0-beta.15] - 05-08-2017
### Added
- License manager functionality

### Updated
- First demo server

## [1.0.0-beta.14] - 03-08-2017
- Separated library code from the server

## [1.0.0-beta.13] - 02-08-2017
### Updated
- Removed `allow_pinging_dead_client` config
- New configs `timestamp_validity` and `requires_timestamp`

### Changed
- Changed `authorize_loggers_with_no_access_code` to `allow_default_access_code`

### Fixed
- Healing the connection will respond with flags and other connection response items
- Fixed release build

## [1.0.0-beta.12] - 27-07-2017
### Added
- Ability to specify key size for each client
- Added `max_token_age` configuration

### Updated
- Performance update for I/O operation on public key
- Default token age for known loggers is now value of global `token_age` configuration
- `server_rsa_secret` should be hex-encoded value now

## [1.0.0-beta.11] - 22-07-2017
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
### Added
- Fixed admin request handler and decryption
- Ability to save configuration
- Ability to add and remove known clients using admin requests
- Ability to reset the server using CLI
- Ability to reset the server using admin request

## [1.0.0-beta.3] - 09-05-2017
### Changes
- Fixed typo in documentation

## [1.0.0-beta.2] - 20-04-2017
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
