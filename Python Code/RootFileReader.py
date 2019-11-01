# This module is used to transcribe PFO data from a ROOT file (stored as vectors) into python arrays.
import ROOT

# This class defines the data associated with each PFO.
class PfoClass(object):
	
	wireCoordErr = 0.3			# Sets wire coord error to 3 millimetres.

	def __init__(self, pfo):
		self.eventId = pfo.EventId
		self.pfoId = pfo.PfoId
		self.parentPfoId = pfo.ParentPfoId
		self.daughterPfoIds = ReadRootVector(pfo.DaughterPfoIds)
		self.heirarchyTier = pfo.HierarchyTier
		self.monteCarloPDGU = pfo.MCPdgCodeU
		self.monteCarloPDGV = pfo.MCPdgCodeV
		self.monteCarloPDGW = pfo.MCPdgCodeW
		self.vertex = ReadRootVector(pfo.Vertex)
		self.driftCoordW = ReadRootVector(pfo.DriftCoordW)
		self.driftCoordErrW = ReadRootVector(pfo.DriftCoordErrorW)
		self.wireCoordW = ReadRootVector(pfo.WireCoordW)
		self.driftCoordU = ReadRootVector(pfo.DriftCoordU)
		self.driftCoordErrU = ReadRootVector(pfo.DriftCoordErrorU)
		self.wireCoordU = ReadRootVector(pfo.WireCoordU)
		self.driftCoordV = ReadRootVector(pfo.DriftCoordV)
		self.driftCoordErrV = ReadRootVector(pfo.DriftCoordErrorV)
		self.wireCoordV = ReadRootVector(pfo.WireCoordV)
		self.energyU = ReadRootVector(pfo.EnergyU)
		self.energyV = ReadRootVector(pfo.EnergyV)
		self.energyW = ReadRootVector(pfo.EnergyW)

	# These change how the PFO is printed to the screen
	def __str__(self):
		return "(PFO eventID=" + str(self.eventId) + " pfoID=" + str(self.pfoId) + ")"
	def __unicode__(self):
		return str(self)
	def __repr__(self):
		return str(self)
	
	def TrueTypeU(self):
		if self.monteCarloPDGU != 0:
			return 1 if abs(self.monteCarloPDGU) == 11 else 0
		else
			return -1
	def TrueTypeV(self):
		if self.monteCarloPDGV != 0:
			return 1 if abs(self.monteCarloPDGV) == 11 else 0
		else
			return -1
	def TrueTypeW(self):
		if self.monteCarloPDGW != 0:s
			return 1 if abs(self.monteCarloPDGW) == 11 else 0
		else
			return -1

	# Uses the PDG codes of all wire plane to check if the PFO truly is a track or shower
	def TrueTypeCombined(self):
		absPDGs = [self.TrueTypeU(), self.TrueTypeV(), self.TrueTypeW()]
		if absPDGs.count(1) > 1:				# Is a shower if more than two views have majority of hits from electrons.
			return 1
		elif absPDGs.count(0) == 0:				# Is a track if more than two views have majority of hits from other particle types.
			return 0
		elif absPDGs.count(-1) == 1:				# The case with two conflicting PDGs. 
			return -1
		elif absPDGs.count(-1) == 2:				# The case with only a PDG from one view. We just use that PDG.
			return absPDGs.count(1)
		else:							# No PDG info available
			return -1

# This function inserts a pfo into an event. It also ensures that the list is ordered by the pfo ID.
def AddPfoToEvent(eventPfos, pfo):
	for i in range(0, len(eventPfos)):
		if pfo.pfoId < eventPfos[i].pfoId:
			eventPfos.insert(i, pfo)
			return
	eventPfos.append(pfo)

# Converts a ROOT vector into a Pyhon list
def ReadRootVector(rootVector):
	pyVector = []
	for i in rootVector:
		pyVector.append(i)
	return pyVector

# Transcribes data from a ROOT file to Python
def ReadRootFile(filepath):
	f = ROOT.TFile.Open(filepath, "read")
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
	# The for loop does not append the last event to the array
	events.append(eventPfos)
	return events
