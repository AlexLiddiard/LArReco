# This module is for track/shower algorithm #1
import statistics
import math

# Algorithm parameters
minBins = 2
binWidth = 0.45


# This function counts how many numbers fall in a set of bins of a given width.
# Empty bins are ignored.

def GetBinCounts(numbers, binWidth):
    if len(numbers) == 0:
        return []

    numbers.sort()
    # The upper bound used for checking if a number falls in the current bin
    upperBound = numbers[0]
    # The numbers in each bin
    binCounts = []

    count = 0
    for n in numbers:
        if n < upperBound:
            # The number falls into the current bin
            count += 1
        else:
            if count > 0:
                # Only consider non-empty bins
                binCounts.append(count)
            # Find the bin that this number falls into
            while n > upperBound:
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


# Rotate a set of points clockwise by angle theta
# Note that tanTheta = tan(theta) = gradient

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

    # Rotate the coords so that any tracks are roughly parallel to the x axis.
    # Prevents tracks from having hits in very few bins, giving high stdev.
    driftCoordRotated = RotateClockwise(driftCoord, wireCoord, b)[0]
    rotatedBins = GetBinCounts(driftCoordRotated, binWidth)

    # Ensure there are enough bins
    if len(rotatedBins) >= minBins:
        # Get stdev of the bin counts
        return statistics.stdev(rotatedBins)
    else:
        # Insufficient bins
        return -1


def RunAlgorithm(pfo):
    binStd = GetRotatedBinStdev(pfo.driftCoordW, pfo.wireCoordW, binWidth)
    print("chainCount: %.2f avgLengthRatio: %.2f" % binStd)
    return binStd
