<p align=center>
   ﷽
</p>

[← Configurations](/docs/CONFIGURATION.md)

# file_mode
[Integer] File mode is default permissions for log file.

Default: 416 (i.e, `RW-R----`)

In order to build a correct value, you can either use [Create Config](https://muflihun.github.io/residue/create-server-config?) tool or just a simple calculation

| **Mode** | **Value** |
|----------|-----------|
| `R` (User) | 256 |
| `W` (User) | 128 |
| `X` (User) | 64 |
| `R` (Group) | 32 |
| `W` (Group) | 16 |
| `X` (Group) | 8 |
| `R` (Other) | 4 |
| `W` (Other) | 2 |
| `X` (Other) | 1 |

For `RW-R----` you will just do a bitwise OR `256 | 128 | 32` i.e, `416`

## Exceptions
You cannot have:
 * `W` (Other)
 * `X` (Other)
 
You must have:
 * `R` (User)
 * `W` (User)

## See Also
 * [ArchLinux Documentation](https://wiki.archlinux.org/index.php/File_permissions_and_attributes)

[← Configurations](/docs/CONFIGURATION.md)

