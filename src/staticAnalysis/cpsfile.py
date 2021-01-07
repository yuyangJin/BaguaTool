#!/usr/bin/python
#compress all dirname and filename
import sys

binName = sys.argv[1]

inFileName = binName + ".psg-symb-1"
outFileName = binName + ".psg-symb"
outFileName2 = binName + ".psg-df"

with open(inFileName) as fi:
  inFileLines = fi.readlines()
fi.close()

fo1 = open(outFileName, "w")


dirMap = []
fileMap = []
i = 0
for inFileLine in inFileLines:
  infoStruct = inFileLine.strip().split(':')
  dirFile = infoStruct[0]
  lineNum = infoStruct[1]
  dirName = ""
  fileName = ""
  
  if '/' in dirFile:
    dirName = dirFile[:dirFile.rfind('/')]
    fileName = dirFile[(dirFile.rfind('/') + 1):]
  else:
    dirName = "NULL"
    fileName = dirFile
  
  if dirName not in dirMap:
    dirMap.append(dirName)

  if fileName not in fileMap:
    fileMap.append(fileName)
  
  if lineNum == '?':
    lineNum = str(-1)

  if i%2 == 0:
    fo1.write(str(dirMap.index(dirName)) + ' ') 
    fo1.write(str(fileMap.index(fileName)) + ' ')
    fo1.write(lineNum + ' ')
  else:
    fo1.write(lineNum + '\n')
  i += 1
  #print(dirName,fileName)

fo1.close()
fo2 = open(outFileName2, "w")

fo2.write(str(len(dirMap)) + '\n')
for dirName in dirMap:
  fo2.write(dirName + '\n')

fo2.write(str(len(fileMap)) + '\n')
for fileName in fileMap:
  fo2.write(fileName + '\n')

fo2.close()