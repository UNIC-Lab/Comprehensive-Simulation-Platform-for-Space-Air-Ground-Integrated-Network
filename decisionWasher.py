import os
import time

curPath = os.path.abspath(os.path.dirname("."))
# fatherPath = os.path.abspath(os.path.dirname(curPath))
stopTime = 599

with open(curPath + "/Decision_learning_2.txt", "r") as reqFile:
	# read UAVPosition_X file
	with open(curPath + "/decision_learning_to_ns3_2", "w") as targetFile:
		# write UAVPositionNS2_X file
		# reqFile.readline()  # read first line useless
		# reqFile.readline()  # read second line useless
		# reqFile.readline()  # read third line useless
		# initialize initial location to NS2 trace

		currentTime = 0.0
		traceCircleTime = 0.0

		while currentTime < stopTime:
			currentLine = reqFile.readline()
			if currentLine == "":
				break
			currentLineList = currentLine.strip("\n").split(' ')
			currentLineListWashed = []
			for eleCurrLine in currentLineList:
				if eleCurrLine is not '':
					currentLineListWashed.append(eleCurrLine)
			# print(currentLineListWashed[-1])
			for eleDecision in currentLineListWashed[-1]:
				targetFile.write(eleDecision + " ")
			targetFile.write(currentLineListWashed[0] + "\n")
			currentTime = float(currentLineListWashed[0])

print("done it")


		# 	if currentLine == "":
		# 		# break
		# 		reqFile.seek(1)
		# 		currentLine = reqFile.readline()
		# 		# currentLine
		# 		traceCircleTime = currentTime
		# 		continue
		# 	else:
		# 		currentLineList = currentLine.strip("\n").split('  ')
		# 		currentTimeList = currentLineList[1].split('.')
		# 		currentTime = time.mktime(
		# 			time.strptime(currentTimeList[0], '%d %b %Y %H:%M:%S')) + traceCircleTime - startTime + float(
		# 			"0." + currentTimeList[1].strip(";"))
		# 		currentY = (float(currentLineList[4].strip(';')) - originLat) / LatDifference * Ylength
		# 		currentX = (float(currentLineList[6].strip(';')) - originLog) / LogDifference * Xlength
		# 		currentZ = float(currentLineList[-1].strip(';')) * 1000
		# 		targetFile.write(
		# 			"$ns_ at %.6f \"$node_(" % currentTime + str(currentNodeNum) + ") set X_ %.6f\"\n" % currentX)
		# 		targetFile.write(
		# 			"$ns_ at %.6f \"$node_(" % currentTime + str(currentNodeNum) + ") set Y_ %.6f\"\n" % currentY)
		# 		targetFile.write(
		# 			"$ns_ at %.6f \"$node_(" % currentTime + str(currentNodeNum) + ") set Z_ %.6f\"\n" % currentZ)
		#
		#
		# firstLine = reqFile.readline().strip("\n").split('  ')
		# startTime = time.mktime(time.strptime(firstLine[1].split('.')[0], '%d %b %Y %H:%M:%S'))
		# startY = (float(firstLine[4].strip(';')) - originLat) / LatDifference * Ylength
		# startX = (float(firstLine[6].strip(';')) - originLog) / LogDifference * Xlength
		# startZ = float(firstLine[-1].strip(';')) * 1000
		# targetFile.write("$node_(" + str(currentNodeNum) + ") set X_ %.6f\n" % startX)
		# targetFile.write("$node_(" + str(currentNodeNum) + ") set Y_ %.6f\n" % startY)
		# targetFile.write("$node_(" + str(currentNodeNum) + ") set Z_ %.6f\n" % startZ)

	# print(startTime)