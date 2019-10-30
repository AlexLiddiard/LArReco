# This module is for track/shower algorithm #2
import statistics
import math

# Algorithm parameters
maxSeparationU = 2
chainCountThresholdU = 7

maxSeparationV = 2
chainCountThresholdV = 7

maxSeparationW = 2
chainCountThresholdW = 7

maxSeparationU2 = maxSeparationU * maxSeparationU
maxSeparationV2 = maxSeparationV * maxSeparationV
maxSeparationW2 = maxSeparationW * maxSeparationW

# Finds the nearest neighbour of a 2D point (out of a list of points)
# Its quite slow
def NearestPoint(pointX, pointY, pointListX, pointListY):
	nearestPointIndex = 0
	shortestDistance2 = float("inf")
	for i in range(0, len(pointListX)):
		tmp = Distance2(pointX, pointY, pointListX[i], pointListY[i])
		if tmp < shortestDistance2:
			nearestPointIndex = i
			shortestDistance2 = tmp
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
		nearestPointIndex, distance = NearestPoint(currentX, currentY, pointListX, pointListY)
		if distance < maxSeparation2:
			currentX = pointListX.pop(nearestPointIndex)
			currentY = pointListY.pop(nearestPointIndex)
			chainX.append(currentX)
			chainY.append(currentY)
		else:
			nearbyPoints = False
	return chainX, chainY

def ShowerInView(driftCoord, wireCoord, chainCountThreshold, maxSeparation2):		
	chainCount = 0
	while driftCoord:	# While pfo hit list is not empty
		CreatePointChain(driftCoord, wireCoord, maxSeparation2)
		chainCount += 1

	if (chainCount > chainCountThreshold):
		return 1, chainCount
	else:
		return 0, chainCount

def RunAlgorithm(pfo):
	# Using only W view
	showerInViewW = ShowerInView(pfo.driftCoordW, pfo.wireCoordW, chainCountThresholdW, maxSeparationW2)
	print("ChainCount: %d" % showerInViewW[1], end = " ")
	return showerInViewW[0]
	
	# Using all three views (slower)
'''
	showerStats = []
	showerInViewU = ShowerInView(pfo.driftCoordU, pfo.wireCoordU, chainCountThresholdU, maxSeparationU2)
	showerInViewV = ShowerInView(pfo.driftCoordV, pfo.wireCoordV, chainCountThresholdV, maxSeparationV2)
	showerInViewW = ShowerInView(pfo.driftCoordW, pfo.wireCoordW, chainCountThresholdW, maxSeparationW2)
	print("ChainCounts: %d %d %d" % (showerInViewU[1], showerInViewV[1], showerInViewW[1]), end = " ")

	showerStats = (showerInViewU[0], showerInViewV[0], showerInViewW[0])
	if (showerStats.count(1) > 1): 					# A good shower score
		return 1
	elif (showerStats.count(0) > 1): 				# A good track score
		return 0
	elif showerStats.count(-1) == 2 and showerStats[2] != -1:	# The case where only info from one plane. The W plane alone gives adequate info for a track/shower decision. 
		return showerStats[2]
	else: # Not enough info to decide
		return -1
'''
