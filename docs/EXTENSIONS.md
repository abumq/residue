<p align="center">
   ﷽
</p>

# Extensions
Residue server can be extended using extensions written in [python v3.6.3+](https://www.python.org/ftp/python/3.6.3/Python-3.6.3.tgz) (see [INSTALL](/docs/INSTALL.docs) for instructions on installation)

You should ensure the server build you are using supports extensions. When you run `residue --version` it will print `(with extension support)` next to version number if the build supports extensions.

These extensions are triggered against log message received. You can use this information to do anything you like with the data, e.g, add it to the database, add it to specific index, send it to bot or web hooks etc.

## Writing New Extension
All you need is Python script containing `execute` method. The script should be installed inside your python module directories (this is where residue will pick it up from). Alternatively, you can set `PYTHONPATH` environment variable.

Let's say you call this extension `push_to_db.py`

In your configurations, you will then add `extensions` array and item `push_to_db`

## Execute Method
The execute method takes 12 parameters. Value for each of them are set in following order.

 * `formattedLine` : Full formatted line that was dispatched to the file
 * `clientId` : Client ID
 * `loggerId` : Logger ID where message was generated from
 * `datetime` : Datetime epoch (int)
 * `level` : Logging level (string) - one of `DEBUG`, `INFO`, `WARNING`, `ERROR`, `FATAL`, `VERBOSE`, `TRACE`
 * `vlevel` : Verbosity level (if any) (int)
 * `sourceFile` : Origin of message
 * `sourceLine` : Origin of message
 * `sourceFunc` : Origin of message
 * `msg` : The log message
 * `threadName` : Thread name (if any)
 * `threadId` : Thread ID (if any)
 * `applicationName` : Application name (if any)
 
## Basic Sample
```python
def execute(formattedLine, clientId, loggerId, datetime, level, vlevel, sourceFile, sourceLine, sourceFunc, msg, threadName, threadId, applicationName):
    print("formattedLine: ", formattedLine)
    print("clientId: ", clientId)
    print("loggerId: ", loggerId)
    print("datetime: ", datetime)
    print("level: ", level)
    print("vlevel: ", vlevel)
    print("sourceFile: ", sourceFile)
    print("sourceLine: ", sourceLine)
    print("sourceFunc: ", sourceFunc)
    print("msg: ", msg)
    print("threadName: ", threadName)
    print("threadId: ", threadId)
    print("applicationName: ", applicationName)
    return 0
```

This will print each line with each log received.

Please note, the logging is ignored for internal logger (logger ID: `residue`)

## Test Your Extension
You can run your extension using following script

```python
import <module>

<module>.execute('formattedLine', 'clientId', 'loggerId', 1507706003171, 'INFO', 0, 'sourceFile', 'sourceLine', 'sourceFunc', 'log msg', 'threadName', 'threadId', 'applicationName')
```

Save this as `test.py` and run `python test.py`

Make sure you use compatible executable for python if needed (e.g `python-3.6m test.py`)

## Samples
You can have a look at [samples](/samples/extensions) directory to browse for sample extensions
