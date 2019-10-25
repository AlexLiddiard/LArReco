# This function is used to transcribe the data from the ROOT file stored as vectors into python arrays.
import ROOT
import statistics

# Algorithm parameters
binWidth = 0.3
minBins = 10
stdThresholdU = 1.00
stdThresholdV = 0.70
stdThresholdW = 0.70

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
	driftBins = GetBinCounts(driftCoord, binWidth)
	wireBins = GetBinCounts(wireCoord, binWidth)

	if (min(len(driftBins), len(wireBins)) < minBins):		# Make sure there are enough bin counts
		return (-1, 0)

	# Calculate the bin count standard deviations, take the smallest result
	stdMin = min(statistics.stdev(driftBins), statistics.stdev(wireBins))
	if (stdMin > stdThreshold): 					# If it is large enough the PFO is a (potential) shower
		return (1, stdMin)
	else:								# Otherwise it is a track
		return (0, stdMin)

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

for eventPfos in events:
	for pfo in eventPfos:
		if pfo.pfoId == 0:
			continue
		print(str(pfo), end = " ")
		result = TrackShowerAlg(pfo)
		if result == 1:
			print("Result: Shower")
		elif result == 0:
			print("Result: Track")
		else:
			print("Result: Unknown")

