<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# archived_log_compressed_filename
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

[← Configurations](/docs/CONFIGURATION.md)

