<p align="center">
   ﷽
</p>

[← Configurations](/docs/CONFIGURATIONS.md)

# admin_port
[Integer] Port that admin server listens to. All the admin requests are sent to this port. These requests must be encrypted using [`server_key`](#server_key).

Default: `8776`

## Admin Server
Admin server is responsible for all the admin requests. These requests can either return some useful information or perform some task.

## Admin Request
All the admin requests require timestamp (`_t`), type (`type`) and relevant information required by specified type.

These requests are of following types

| **Type** | **`type` Value** |
|----------|-----------------|
| Reload Config | 1 |
| Add client | 2 |
| Remove client | 3 |
| Reset  | 4 |
| List logging files | 5 |
| Force log rotation | 6 |
| Stats | 7 |
| List clients | 8 |

A typical admin request will look like:

```
{
    "_t" => 1519812917, // EPOCH IN SECONDS
    "type" => 6, // FORCE_LOG_ROTATION
    "logger_id" => "sample-app" // LOGGER ID IS REQUIRED BY TYPE=6
}
```

## Encrypted Request
Each admin request is encrypted using [`server_key`](/docs/CONFIGURATION.md#server_key) that in turn is used by admin server to read the request.

## See Also
 * [`server_key`](/docs/CONFIGURATION.md#server_key)
 * [`requires_timestamp`](/docs/CONFIGURATION.md#requires_timestamp)
 * [`timestamp_validity`](/docs/CONFIGURATION.md#timestamp_validity)

[← Configurations](/docs/CONFIGURATIONS.md)
