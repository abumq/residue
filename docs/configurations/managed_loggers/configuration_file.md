<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# known_loggers ↴
## configuration_file
[String] Path to [Easylogging++ configuration file](https://github.com/amrayn/easyloggingpp#using-configuration-file). You can use `$RESIDUE_HOME` environment variable in this file path. When the new logger is registered, it's configured using this configuration.

Residue supports following [custom format specifiers](https://github.com/amrayn/easyloggingpp#custom-format-specifiers):

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

You're not allowed to use following configurations in your configuration file as residue handles these configurations differently and ignore your configurations. Residue will not start until these configuration lines are removed. This is so that user does not expect anything from these configuration items.

 * `Max_Log_File_Size`
 * `To_Standard_Output`
 * `Log_Flush_Threshold`
 
 ### Sample
 A configuration file may look like
 ```
 * GLOBAL:
    FORMAT                  =   "%datetime{%H:%m:%s,%g} [%thread] [%level] %msg"
    FILENAME                =   "/var/log/residue/default.log"
    ENABLED                 =   true
* VERBOSE:
    FORMAT                  =   "%datetime{%H:%m:%s,%g} [%thread] [%vnamelevel] [%fbase:%line] %msg"
* DEBUG:
    ENABLED                 =   true
 ```
 
 ## See Also
 * [`RESIDUE_HOME`](https://github.com/amrayn/residue/blob/develop/docs/INSTALL.md#residue_home)
