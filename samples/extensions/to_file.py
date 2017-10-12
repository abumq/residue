
def handle_log(formattedLine, clientId, loggerId, datetime, level, vlevel, sourceFile, sourceLine, sourceFunc, msg, threadName, threadId, applicationName):
    f = open('/tmp/logs/from_extension.log', 'w')
    f.write(formattedLine)
    f.close()
    return 0
