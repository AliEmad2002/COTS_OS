# This script adds preprocessor conditioning to all files in the given directory.

import os

DIR = "C:\\Users\\ali20\\github\\COTS_OS\\Dependencies\\SM32F401RCT6"

INSERTION1 = "\n\n/*	Target checking	*/\n""#include \"MCAL_Port/Port_Target.h\"\n""#ifdef MCAL_PORT_TARGET_STM32F401RCT6\n"

INSERTION2 = "\n\n#endif /* Target checking */"

fileDirList = []

for path, subdirs, files in os.walk(DIR):
    for name in files:
        fileDirList.append(os.path.join(path, name))

for fileDir in fileDirList:
    print(fileDir)
    file = open(fileDir, 'r')
    fileContent = file.read()
    insertionPos = fileContent.find("*/") + 2
    fileContent = fileContent[:insertionPos] + INSERTION1 + fileContent[insertionPos:] + INSERTION2
    file.close()
    file = open(fileDir, 'w')
    file.write(fileContent)