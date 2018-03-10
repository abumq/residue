## Failure Notify Extension
This extension sends email if logs failure have cross the threshold.

### Install
To install this extension simply add following lines in `extensions` config
```
    {
        "name": "failure-notify",
        "module": "path/to/failure-notify.so",
        "description": "Notifies the recipients when log failures have hit the threshold",
        "config": {
            "threshold": 200,
            "recipients": ["operations@example.com"]
        }
    }
``` 
