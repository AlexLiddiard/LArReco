# This module is for track/shower algorithm #0
import statistics
import math

# Algorithm parameters
rThresholdU = 0.8
rThresholdV = 0.5
rThresholdW = 0.7


def ShowerInView(driftCoord, wireCoord, rThreshold):
	rSquared = RSquared(driftCoord, wireCoord)
	if rSquared == -1:
		return -1, -1
	if rSquared < rThreshold:
		return 1, rSquared
	else:
		return 0, rSquared

# Square of the Pearson product-moment correlation
# https://en.wikipedia.org/wiki/Residual_sum_of_squares
# It is a normalised measure of the sum of the residual squares (of the least squares regression line).
def RSquared(xCoords, yCoords):
	Sxy = 0
	Sxx = 0
	Syy = 0
	Sx = 0
	Sy = 0
	n = len(xCoords)
	if n == 0:
		return -1

	for i in range(0, n):
		Sxy += xCoords[i] * yCoords[i]
		Sxx += xCoords[i] * xCoords[i]
		Syy += yCoords[i] * yCoords[i]
		Sx += xCoords[i]
		Sy += yCoords[i]

	top = n * Sxy - Sy * Sx
	divisor = math.sqrt((n * Sxx - Sx * Sx) * (n * Syy - Sy * Sy))
	if divisor == 0:
		return -1	

	r = (n * Sxy - Sy * Sx) / divisor
	return r * r

def RunAlgorithm(pfo):
	showerStats = []
	showerInViewU = ShowerInView(pfo.driftCoordU, pfo.wireCoordU, rThresholdU)
	showerInViewV = ShowerInView(pfo.driftCoordV, pfo.wireCoordV, rThresholdV)
	showerInViewW = ShowerInView(pfo.driftCoordW, pfo.wireCoordW, rThresholdW)
	print("rSquared: %.3f %.3f %.3f" % (showerInViewU[1], showerInViewV[1], showerInViewW[1]), end = " ")

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
