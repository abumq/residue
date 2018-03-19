<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# archived_log_filename
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

[← Configurations](/docs/CONFIGURATION.md)

