## Failure Notify Extension
This extension sends email if logs failures have cross the threshold. It's sent repeatedly i.e, threshold is reset with every notification.

### Install
To install this extension simply add following lines in `extensions` config
```
    {
        "name": "failure-notify",
        "module": "path/to/failure-notify.so",
        "description": "Notifies the recipients when log failures have hit the threshold",
        "config": {
            "threshold": 20000,
            "recipients": ["operations@example.com"]
        }
    }
``` 
