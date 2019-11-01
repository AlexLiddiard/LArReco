# This module is for track/shower feature #2
import math

# Algorithm parameters
rectWidth = 5
rectHeight = 5

# Finds the nearest neighbour of a 2D point (out of a list of points)
# It's quite slow
def NearestPoint(pointX, pointY, pointListX, pointListY):
	nearestPointIndex = 0
	shortestDistance2 = float("inf")
	for i in range(0, len(pointListX)):
		tmp = Distance2(pointX, pointY, pointListX[i], pointListY[i])
		if tmp < shortestDistance2:
			nearestPointIndex = i
			shortestDistance2 = tmp
	return nearestPointIndex, shortestDistance2

# Finds the nearest neighbour of a 2D point (out of a list of points)
# The list of points checked is a subsample of the input list. It consists of all points within a rectangle
# of width rectWidth and height rectHeight centred on the point.
# If the subsample is empty, a distance of infinity and point index of -1 is returned.
def NearestPointInRectangle(pointX, pointY, pointListX, pointListY, rectWidth, rectHeight):
	xLow = pointX - rectWidth / 2
	xHigh = pointX + rectWidth / 2
	yLow = pointY - rectHeight / 2
	yHigh = pointY + rectHeight / 2
	nearestPointIndex = -1
	shortestDistance2 = float("inf")
	for i in range(0, len(pointListX)):
		xTest = pointListX[i]
		yTest = pointListY[i]
		if xTest < xLow:
			continue
		if xTest > xHigh:
			continue
		if yTest < yLow:
			continue
		if yTest > yHigh:
			continue
		
		distance2 = Distance2(pointX, pointY, pointListX[i], pointListY[i])
		if distance2 < shortestDistance2:
			nearestPointIndex = i
			shortestDistance2 = distance2
	return nearestPointIndex, shortestDistance2

# Resurns the square of the separation distance of a pair of 2D points
def Distance2(pointAX, pointAY, pointBX, pointBY):
	deltaX = pointAX - pointBX
	deltaY = pointAY - pointBY
	return deltaX * deltaX + deltaY * deltaY

# Creates a list of points that form a chain. Each point pair in the chan has a squared separation smaller than maxSeparation2.
# The chain starts with the first point in the input list
# The chain ends when no more nearby points can be found.
# The points added to the chain are removed from the input list of points.
def CreatePointChain(pointListX, pointListY, maxSeparation2):
	currentX = pointListX.pop(0)
	currentY = pointListY.pop(0)
	chainX = [currentX]
	chainY = [currentY]
	nearbyPoints = True
	while (nearbyPoints):
		nearestPointIndex = NearestPoint(currentX, currentY, pointListX, pointListY)
		if distance < maxSeparation2:
			currentX = pointListX.pop(nearestPointIndex)
			currentY = pointListY.pop(nearestPointIndex)
			chainX.append(currentX)
			chainY.append(currentY)
		else:
			nearbyPoints = False
	return chainX, chainY

# Using a square instead of a circle to limit the max separation, a bit more efficient
def CreatePointChain2(pointListX, pointListY, rectWidth, rectHeight):
	currentX = pointListX.pop(0)
	currentY = pointListY.pop(0)
	chainX = [currentX]
	chainY = [currentY]
	chainLength = 0
	nearbyPoints = True
	while (nearbyPoints):
		nearestPointIndex, distance2 = NearestPointInRectangle(currentX, currentY, pointListX, pointListY, rectWidth, rectHeight)
		if nearestPointIndex >= 0:
			currentX = pointListX.pop(nearestPointIndex)
			currentY = pointListY.pop(nearestPointIndex)
			chainX.append(currentX)
			chainY.append(currentY)
			chainLength += math.sqrt(distance2)
		else:
			nearbyPoints = False
	return chainX, chainY, chainLength

def GetChainInfo(driftCoord, wireCoord, chainThreshold, rectWidth, rectHeight):		
	chainCount = 0
	sumLengthRatios = 0
	while driftCoord:	# While pfo hit list is not empty
		chainX, chainY, chainLength = CreatePointChain2(driftCoord, wireCoord, rectWidth, rectHeight)
		if len(chainX) > 1:
			lengthRatio = math.sqrt(Distance2(chainX[0], chainY[0], chainX[-1], chainY[-1])) / chainLength
		else:
			lengthRatio = 1
		sumLengthRatios += lengthRatio
	avgLengthRatio = sumLengthRatios / chainCount
	return chainCount, avgLengthRatio


def RunAlgorithm(pfo):
	chainCount, avgLengthRatio = GetChainInfo(pfo.driftCoordW, pfo.wireCoordW, chainThresholdW, rectWidthW, rectHeightW, crumpleConstW)
	print("chainCount: %.2f avgLengthRatio: %.2f" % (chainCount, avgLengthRatio))
	return chainCount, avgLengthRatio
