import time

def handle_log(formattedLine, clientId, loggerId, datetime, level, vlevel, sourceFile, sourceLine, sourceFunc, msg, threadName, threadId, applicationName):
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
    time.sleep(1)
    return 0
