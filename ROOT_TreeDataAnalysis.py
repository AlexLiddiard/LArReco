# This function is used to transcribe the data from the ROOT file stored as vectors into python arrays.
import ROOT
import statistics
import math

# Algorithm parameters
binWidth = 0.45
minBins = 2
stdThresholdU = 0.55
stdThresholdV = 0.55
stdThresholdW = 0.55

def ReadROOTVector(ROOTVector):
	pyVector = []
	for i in ROOTVector:
		pyVector.append(i)
	return pyVector

# This function inserts a pfo into an event. It also ensures that the list is ordered by the pfo ID.
def AddPfoToEvent(eventPfos, pfo):
	for i in range(0, len(eventPfos)):
		if pfo.pfoId < eventPfos[i].pfoId:
			eventPfos.insert(i, pfo)
			return
	eventPfos.append(pfo)

class PfoClass(object):				# This class defines the data associated with each pfo.
	
	wireCoordErr = 0.3			# Sets wire coord error to 3 millimetres.

	def __init__(self, pfo):
		self.eventId = pfo.EventId
		self.pfoId = pfo.PfoId
		self.parentPfoId = pfo.ParentPfoId
		self.daughterPfoIds = ReadROOTVector(pfo.DaughterPfoIds)
		self.heirarchyTier = pfo.HierarchyTier
		self.monteCarloPDGU = pfo.MCPdgCodeU
		self.monteCarloPDGV = pfo.MCPdgCodeV
		self.monteCarloPDGW = pfo.MCPdgCodeW
		self.vertex = ReadROOTVector(pfo.Vertex)
		self.driftCoordW = ReadROOTVector(pfo.DriftCoordW)
		self.driftCoordWErr = ReadROOTVector(pfo.DriftCoordErrorW)
		self.wireCoordW = ReadROOTVector(pfo.WireCoordW)
		self.driftCoordU = ReadROOTVector(pfo.DriftCoordU)
		self.driftCoordUErr = ReadROOTVector(pfo.DriftCoordErrorU)
		self.wireCoordU = ReadROOTVector(pfo.WireCoordU)
		self.driftCoordV = ReadROOTVector(pfo.DriftCoordV)
		self.driftCoordVErr = ReadROOTVector(pfo.DriftCoordErrorV)
		self.wireCoordV = ReadROOTVector(pfo.WireCoordV)
		self.EnergyU = ReadROOTVector(pfo.EnergyU)
		self.EnergyV = ReadROOTVector(pfo.EnergyV)
		self.EnergyW = ReadROOTVector(pfo.EnergyW)

	# These change how the PFO is printed to the screen
	def __str__(self):
		return "(PFO eventID=" + str(self.eventId) + " pfoID=" + str(self.pfoId) + ")"
	def __unicode__(self):
		return str(self)
	def __repr__(self):
		return str(self)

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

def ShowerInView(driftCoord, wireCoord, stdThreshold):
	# Get bin counts for each of the coordinate components of the W plane hits
	#driftBins = GetBinCounts(driftCoord, binWidth)
	#wireBins = GetBinCounts(wireCoord, binWidth)

	b = OLS(driftCoord, wireCoord)[0]
	driftCoordRotated = RotateClockwise(driftCoord, wireCoord, b)[0] # Rotate the coords so that any tracks will lie roughly parallel to the x axis.
	rotatedBins = GetBinCounts(driftCoordRotated, binWidth)

	# Make sure there are enough bin counts
	#driftStd = float('inf')
	#wireStd = float('inf')
	#rotatedStd = float('inf')
	#if len(driftBins) >= minBins:
	#	driftStd = statistics.stdev(driftBins)
	#if len(wireBins) >= minBins:
	#	wireStd = statistics.stdev(wireBins)
	if len(rotatedBins) >= minBins:
		rotatedStd = statistics.stdev(rotatedBins)
	else:
		return -1, 0						# Not enough bins

	# Calculate the bin count standard deviations, take the smallest result
	#stdMin = min(driftStd, wireStd, rotatedStd)
	#if stdMin == float('inf'):					# Not enough bins
	#	return -1, 0
	if rotatedStd > stdThreshold: #stdMin > stdThreshold: 		# If it is large enough the PFO is a (potential) shower
		return 1, rotatedStd #stdMin
	else:								# Otherwise it is a track
		return 0, rotatedStd #stdMin

def TrackShowerAlg(pfo):
	showerStats = []
	showerInViewU = ShowerInView(pfo.driftCoordU, pfo.wireCoordU, stdThresholdU)
	showerInViewV = ShowerInView(pfo.driftCoordV, pfo.wireCoordV, stdThresholdV)
	showerInViewW = ShowerInView(pfo.driftCoordW, pfo.wireCoordW, stdThresholdW)
	print("std: %.3f %.3f %.3f" % (showerInViewU[1], showerInViewV[1], showerInViewW[1]), end = " ")

	showerStats = (showerInViewU[0], showerInViewV[0], showerInViewW[0])
	if (showerStats.count(1) > 1): 					# A good shower score
		return 1
	elif (showerStats.count(0) > 1): 				# A good track score
		return 0
	elif showerStats.count(-1) == 2 and showerStats[2] != -1:	# The case where only info from one plane. The W plane alone gives adequate info for a track/shower decision. 
		return showerStats[2]
	else: # Not enough info to decide
		return -1

# Ordinary Least Squares line fit
def OLS(xCoords, yCoords):
	Sxy = 0
	Sxx = 0
	Sx = 0
	Sy = 0
	n = len(xCoords)
	if n == 0:
		return float('inf'), 0

	for i in range(0, n):
		Sxy += xCoords[i] * yCoords[i]
		Sxx += xCoords[i] * xCoords[i]
		Sx += xCoords[i]
		Sy += yCoords[i]
	
	if (Sx == 0 and Sxx == 0):
		print(xCoords)
		print(yCoords)
	#print("Sxy:%.3f Sxx:%.3f Sx:%.3f Sy:%.3f" % (Sxy, Sxx, Sx, Sy), end = " ")
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


def TrulyIsShower(pfo):
	absPDGs = [abs(pfo.monteCarloPDGU), abs(pfo.monteCarloPDGV), abs(pfo.monteCarloPDGW)]
	if absPDGs.count(11) > 1:
		return 1
	else:
		return 0

f = ROOT.TFile.Open(input("Root file name/path: "), "read")
events = []		# Array containing arrays of Pfos from the same event.
eventPfos = []		# Array containing PfoObjects
currentEventId = 0	# Allows function writing to the events and eventPfos arrays to work (see below).

for pfo in f.PFOs:
	# Inputing the variables read from the ROOT file into the class to create the PfoObject.

	PfoBeingRead = PfoClass(pfo)
	if currentEventId == pfo.EventId:
		AddPfoToEvent(eventPfos, PfoBeingRead)
	else:
		events.append(eventPfos)
		eventPfos = [PfoBeingRead]
		currentEventId = pfo.EventId
# The for loop does not append the event to the array
eventPfos.append(eventPfos)

score = 0
count = 0
for eventPfos in events:
	for pfo in eventPfos:
		if pfo.pfoId == 0 or TrulyIsShower(pfo) == 1:
			continue
		count += 1
		print(str(pfo), end = " ")
		result = TrackShowerAlg(pfo)
		if result == 1:
			print("Result: S", end=" ")
		elif result == 0:
			print("Result: T", end=" ")
		else:
			print("Result: U")
		
		if result != -1:
			if result == TrulyIsShower(pfo):
				print("Correct")
				score += 1
			else:
				print("Incorrect")
print("Accuracy: " + str(100 * score / count) + "%")
