# This module is for track/shower algorithm #1
import statistics
import math

# Algorithm parameters
minBins = 2

binWidthU = 0.4
stdThresholdU = 0.75

binWidthV = 0.42
stdThresholdV = 0.74

binWidthW = 0.45
stdThresholdW = 0.74


# This function counts how many numbers fall in a set of bins of a given width. Empty bins are ignored.
def GetBinCounts(numbers, binWidth):
	if len(numbers) == 0:
		return []

	numbers.sort()
	upperBound = numbers[0]						# The upper bound used for checking if a number falls in the current bin
	binCounts = []							# Count of the numbers in each bin
	count = 0							# Current bin count
	for n in numbers:
		if n < upperBound:					# The number falls into the current bin
			count += 1
		else:
			if count > 0:					# Ignore empty bins
				binCounts.append(count)
			while n > upperBound:				# Find the bin that this number falls into
				upperBound += binWidth
			count = 1
	return binCounts

def ShowerInView(driftCoord, wireCoord, stdThreshold, binWidth):
	b = OLS(driftCoord, wireCoord)[0]

	# Rotate the coords so that any tracks will lie roughly parallel to the x axis. Prevents tracks from having hits in very few bins, which would give high stdev.
	driftCoordRotated = RotateClockwise(driftCoord, wireCoord, b)[0]
	rotatedBins = GetBinCounts(driftCoordRotated, binWidth)

	if len(rotatedBins) >= minBins:					# Make sure there are enough bin counts
		rotatedStd = statistics.stdev(rotatedBins)		# Get stdev of the bin counts
	else:
		return -1, 0						# Not enough bins
	if rotatedStd > stdThreshold:			 		# If the stdev is large enough the PFO deemed a shower
		return 1, rotatedStd
	else:								# Otherwise it is deemed a track
		return 0, rotatedStd

# Ordinary Least Squares line fit
def OLS(xCoords, yCoords):
	Sxy = 0
	Sxx = 0
	Sx = 0
	Sy = 0
	n = len(xCoords)
	if n == 0:
		return float('-inf'), 0

	for i in range(0, n):
		Sxy += xCoords[i] * yCoords[i]
		Sxx += xCoords[i] * xCoords[i]
		Sx += xCoords[i]
		Sy += yCoords[i]

	divisor = Sxx - Sx * Sx / n	
	if divisor == 0:
		return (float('inf'), 0)
	b = (Sxy - Sx * Sy / n) / divisor
	a = Sy / n - b * Sx / n
	return b, a

# Rotate a set of points clockwise by angle theta, note that tanTheta = tan(theta) = gradient
def RotateClockwise(xCoords, yCoords, tanTheta):
	cosTheta = 1 / math.sqrt(1 + tanTheta * tanTheta)
	sinTheta = tanTheta * cosTheta
	xCoordsNew = []
	yCoordsNew = []
	for i in range(0, len(xCoords)):
		xCoordsNew.append(xCoords[i] * cosTheta + yCoords[i] * sinTheta)
		yCoordsNew.append(yCoords[i] * cosTheta - xCoords[i] * sinTheta)
	return xCoordsNew, yCoordsNew

def RunAlgorithm(pfo):
	showerStats = []
	showerInViewU = ShowerInView(pfo.driftCoordU, pfo.wireCoordU, stdThresholdU, binWidthU)
	showerInViewV = ShowerInView(pfo.driftCoordV, pfo.wireCoordV, stdThresholdV, binWidthV)
	showerInViewW = ShowerInView(pfo.driftCoordW, pfo.wireCoordW, stdThresholdW, binWidthW)
	print("std: %.3f %.3f %.3f" % (showerInViewU[1], showerInViewV[1], showerInViewW[1]), end = " ")

	#return showerInViewU[0] # For testing a single view

#'''
	showerStats = (showerInViewU[0], showerInViewV[0], showerInViewW[0])
	


	if (showerStats.count(1) > 1): 		# A good shower score
		return 1
	elif showerStats.count(-1) == 0: 	# A good track score
		return 0
	elif showerStats.count(-1) == 1:	# The case whith conflicting info from two views. We favour the view with higher individual accuracy (W, V, U in descending order).
		return showerStats[2] if showerStats[2] != -1 else showerStats[1]
	elif showerStats.count(-1) == 2:	# The case with info from one view. We just take the result of that view.
		return showerStats.count(1)
	else: 					# No info from any views, cannot decide.
		return -1
#'''
