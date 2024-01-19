import os
# import time
import numpy as np

curPath = os.path.abspath(os.path.dirname("."))
# fatherPath = os.path.abspath(os.path.dirname(curPath))
stopTime = 599 # 694
lteDelayList = []
uavDelayList = []
satDelayList = []
meanList = []
stdList = []
coverageList = []
totalReqNum = 30*stopTime

with open(curPath + "/UlConMsg30_10k_600.dat", "r") as reqFile:
	# read UAVPosition_X file
	# with open(curPath + "/decision_to_ns3", "w") as targetFile:
		# write UAVPositionNS2_X file

	currentTime = 0.0
	# traceCircleTime = 0.0

	while currentTime < stopTime:
		currentLine = reqFile.readline()
		if currentLine == "":
			break
		currentLineList = currentLine.strip("\n").split('\t')

		if currentLineList[2] == "lte":
			if (float(currentLineList[0]) - float(currentLineList[6])) < 0.4:
				lteDelayList.append(float(currentLineList[0]) - float(currentLineList[6]))
		elif currentLineList[2] == "sat":
			if (float(currentLineList[0]) - float(currentLineList[6])) < 0.4:
				satDelayList.append(float(currentLineList[0]) - float(currentLineList[6]))
		else:
			if (float(currentLineList[0]) - float(currentLineList[6])) < 0.4:
				uavDelayList.append(float(currentLineList[0]) - float(currentLineList[6]))
		currentTime = float(currentLineList[0])

# print(max(lteDelayList))
# print(max(satDelayList))

meanList.append(np.mean(lteDelayList))
meanList.append(np.mean(uavDelayList))
meanList.append(np.mean(satDelayList))

stdList.append(np.sqrt(((lteDelayList - np.mean(lteDelayList)) ** 2).sum() / len(lteDelayList)))
stdList.append(np.sqrt(((uavDelayList - np.mean(uavDelayList)) ** 2).sum() / len(uavDelayList)))
stdList.append(np.sqrt(((satDelayList - np.mean(satDelayList)) ** 2).sum() / len(satDelayList)))

# stdList.append(np.std(uavDelayList))
# stdList.append(np.std(satDelayList))

coverageList.append(len(lteDelayList)/totalReqNum)
coverageList.append(len(uavDelayList)/totalReqNum)
coverageList.append(len(satDelayList)/totalReqNum)

print(meanList)
print(stdList)
print(coverageList)

print("done it")