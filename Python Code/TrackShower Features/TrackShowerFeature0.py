# This module is for track/shower algorithm #0

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
	rSquared = RSquared(xCoords, yCoords)
	print("rSquared: %.2f" % showerInViewU[1])
	return rSquared
