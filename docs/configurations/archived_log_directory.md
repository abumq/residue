<p align=center>
   ﷽
</p>

[🏠 Configurations](/docs/CONFIGURATION.md)

# archived_log_directory
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

[🏠 Configurations](/docs/CONFIGURATION.md)

