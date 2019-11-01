# This module is for track/shower algorithm #1
import statistics
import math

# Algorithm parameters
minBins = 2
binWidth = 0.45

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

def GetRotatedBinStdev(driftCoord, wireCoord, binWidth):
	b = OLS(driftCoord, wireCoord)[0]

	# Rotate the coords so that any tracks will lie roughly parallel to the x axis. Prevents tracks from having hits in very few bins, which would give high stdev.
	driftCoordRotated = RotateClockwise(driftCoord, wireCoord, b)[0]
	rotatedBins = GetBinCounts(driftCoordRotated, binWidth)

	if len(rotatedBins) >= minBins:					# Make sure there are enough bins
		rotatedStd = statistics.stdev(rotatedBins)		# Get stdev of the bin counts
	else:
		rotatedStd = -1						# Insufficient bins
	return rotatedStd

def RunAlgorithm(pfo):
	rotatedBinStdev = GetRotatedBinStdev(pfo.driftCoordW, pfo.wireCoordW, binWidth)
	print("chainCount: %.2f avgLengthRatio: %.2f" % (chainCount, avgLengthRatio))
	return rotatedBinStdev
