import os
import time

curPath = os.path.abspath(os.path.dirname("."))
# fatherPath = os.path.abspath(os.path.dirname(curPath))
originLat = 43.461859
originLog = -80.563406
finalLat = 43.478942
finalLog = -80.535474
LatDifference = finalLat - originLat
LogDifference = finalLog - originLog
Xlength = 2360.76
Ylength = 1993.42
existNodeNumLast = 29
stopTime = 610

for reqNum in range(1, 4):
# iteration of trace files for multiple UAVs
	currentNodeNum = existNodeNumLast+reqNum

	with open(curPath + "/UAVPosition_"  + str(reqNum) + ".txt", "r") as reqFile:
	# read UAVPosition_X file
		with open(curPath + "/UAVPositionRoad_"  + str(reqNum), "w") as targetFile:
		# write UAVPositionNS2_X file
			titleLine = reqFile.readline() # read first line useless
			# initialize initial location to NS2 trace
			firstLine = reqFile.readline().strip("\n").split('  ')
			startTime = time.mktime(time.strptime(firstLine[1].split('.')[0],'%d %b %Y %H:%M:%S'))
			startY = (float(firstLine[4].strip(';')) - originLat)/LatDifference * Ylength
			startX = (float(firstLine[6].strip(';')) - originLog)/LogDifference * Xlength
			startZ = float(firstLine[-1].strip(';')) * 1000
			targetFile.write("$node_(" + str(currentNodeNum) + ") set X_ %.6f\n" % startX)
			targetFile.write("$node_(" + str(currentNodeNum) + ") set Y_ %.6f\n" % startY)
			targetFile.write("$node_(" + str(currentNodeNum) + ") set Z_ %.6f\n" % startZ)

			currentTime = 0.0
			traceCircleTime = 0.0

			while currentTime < stopTime:
				currentLine = reqFile.readline()
				if currentLine == "":
					# break
					reqFile.seek(1)
					currentLine = reqFile.readline()
					# currentLine
					traceCircleTime = currentTime
					continue
				else:
					currentLineList = currentLine.strip("\n").split('  ')
					currentTimeList = currentLineList[1].split('.')
					currentTime = time.mktime(time.strptime(currentTimeList[0],'%d %b %Y %H:%M:%S')) + traceCircleTime - startTime + float("0."+currentTimeList[1].strip(";"))
					currentY = (float(currentLineList[4].strip(';')) - originLat)/LatDifference * Ylength
					currentX = (float(currentLineList[6].strip(';')) - originLog)/LogDifference * Xlength
					currentZ = float(currentLineList[-1].strip(';')) * 1000	
					targetFile.write("$ns_ at %.6f \"$node_(" % currentTime + str(currentNodeNum) + ") set X_ %.6f\"\n" % currentX)
					targetFile.write("$ns_ at %.6f \"$node_(" % currentTime + str(currentNodeNum) + ") set Y_ %.6f\"\n" % currentY)
					targetFile.write("$ns_ at %.6f \"$node_(" % currentTime + str(currentNodeNum) + ") set Z_ %.6f\"\n" % currentZ)


	# print(startTime)