## Log Analyzer Extension
This extension analyzes the log message and executes command based on configuration for each level.

### Configurations
Each key is level, e.g,
    ```
    "info": "logger %formatted_msg"
    ```
will trigger `logger` command on followed by formatted message.

Accepted format specifiers are `msg` and `formatted_msg`

### Install
To install this extension simply add following lines in `extensions` config
```
    {
        "name": "log-analyzer",
        "module": "path/to/log-analyzer.so",
        "description": "Analyzes the log and executes specific commands based on logging level",
        "config": {
            "error": "logger %msg"
        }
    }
``` 