<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# known_loggers ↴
## rotation_freq
[String] One of [`never`, `hourly`, `six_hours`, `twelve_hours`, `daily`, `weekly`, `monthly`, `yearly`] to specify rotation frequency for corresponding log files. This is rotated regardless of file size.

Default: `never`

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

