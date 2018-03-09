<p align=center>
   ﷽
</p>

[🏠 Configurations](/docs/CONFIGURATION.md)
[→ Known Clients](/docs/configurations/known_clients/#known_clients)

# known_clients ↴
## user
[String] Linux / macOS user. All the log files associated to the corresponding loggers will changed to be owned by this user with `RW-R-----` permissions (subject to `file_mode` config)

Default: Current process user

Current process user will usually be `root` unless you have used `--force-without-root` flag.

## See Also
 * [`file_mode`](/docs/CONFIGURATION.md#file_mode)
