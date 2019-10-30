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
		self.driftCoordWErr = ReadRootVector(pfo.DriftCoordErrorW)
		self.wireCoordW = ReadRootVector(pfo.WireCoordW)
		self.driftCoordU = ReadRootVector(pfo.DriftCoordU)
		self.driftCoordUErr = ReadRootVector(pfo.DriftCoordErrorU)
		self.wireCoordU = ReadRootVector(pfo.WireCoordU)
		self.driftCoordV = ReadRootVector(pfo.DriftCoordV)
		self.driftCoordVErr = ReadRootVector(pfo.DriftCoordErrorV)
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

	# Uses the PDG codes of each wire plane to check if the PFO truly is a track or shower
	def TrulyIsShower(self):
		absPDGs = [abs(self.monteCarloPDGU), abs(self.monteCarloPDGV), abs(self.monteCarloPDGW)]
		if absPDGs.count(11) > 1:
			return 1
		else:
			return 0

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
