# This function is used to transcribe the data from the ROOT file stored as vectors into python arrays.
import ROOT

def ReadROOTVector(ROOTVector):
	pyVector = []
	for i in ROOTVector:
		pyVector.append(i)
	return pyVector

class PfoClass(object):				# This class defines the data associated with each pfo.
	
	wireCoordErr = 0.003			# Sets wire coord error to 3 millimetres.

	def __init__(self, parentPFOId, daughterPfoIds, heirarchyTier, monteCarloPDGU, monteCarloPDGV, monteCarloPDGW, vertex, driftCoordW, driftCoordWErr, wireCoordW, driftCoordU, driftCoordUErr, wireCoordU, driftCoordV, driftCoordVErr, wireCoordV, EnergyU, EnergyV, EnergyW):

		self.parentPFOId = parentPFOId
		self.daughterPfoIds = daughterPfoIds
		self.heirarchyTier = heirarchyTier
		self.monteCarloPDGU = monteCarloPDGU
		self.monteCarloPDGV = monteCarloPDGV
		self.monteCarloPDGW = monteCarloPDGW
		self.vertex = vertex
		self.driftCoordW = driftCoordW
		self.driftCoordWErr = driftCoordWErr
		self.wireCoordW = wireCoordW
		self.driftCoordU = driftCoordU
		self.driftCoordUErr = driftCoordUErr
		self.wireCoordU = wireCoordU
		self.driftCoordV = driftCoordV
		self.driftCoordVErr = driftCoordVErr
		self.wireCoordV = wireCoordV
		self.EnergyU = EnergyU
		self.EnergyV = EnergyV
		self.EnergyW = EnergyW

f = ROOT.TFile.Open("PFOs.root", "read")
events = []						# Array containing arrays of Pfos from the same event.
eventPfos = []						# Array containing PfoObjects
currentEventId = 0					# Allows function writing to the events and eventPfos arrays to work (see below).

for pfo in f.PFOs:

	# Setting variables to values read from root file, in order that they can be used as PfoClass arguments to create a PfoObject.

	parentPfoId = pfo.ParentPfoId
	daughterPfoIds = ReadROOTVector(pfo.DaughterPfoIds)
	heirarchyTier = pfo.HierarchyTier
	monteCarloPDGU = pfo.MCPdgCodeU
	monteCarloPDGV = pfo.MCPdgCodeV
	monteCarloPDGW = pfo.MCPdgCodeW
	vertex = ReadROOTVector(pfo.Vertex)
	driftCoordW = ReadROOTVector(pfo.DriftCoordW)
	driftCoordWErr = ReadROOTVector(pfo.DriftCoordErrorW)
	wireCoordW = ReadROOTVector(pfo.WireCoordW)
	driftCoordU = ReadROOTVector(pfo.DriftCoordU)
	driftCoordUErr = ReadROOTVector(pfo.DriftCoordErrorU)
	wireCoordU = ReadROOTVector(pfo.WireCoordU)
	driftCoordV = ReadROOTVector(pfo.DriftCoordV)
	driftCoordVErr = ReadROOTVector(pfo.DriftCoordErrorV)
	wireCoordV = ReadROOTVector(pfo.WireCoordV)
	EnergyU = ReadROOTVector(pfo.EnergyU)
	EnergyV = ReadROOTVector(pfo.EnergyV)
	EnergyW = ReadROOTVector(pfo.EnergyW)


	# Inputing the variables read from the ROOT file into the class to create the PfoObject.

	PfoBeingRead = PfoClass(parentPfoId, daughterPfoIds, heirarchyTier, monteCarloPDGU, monteCarloPDGV, monteCarloPDGW, vertex, driftCoordW, driftCoordWErr, wireCoordW, driftCoordU, driftCoordUErr, wireCoordU, driftCoordV, driftCoordVErr, wireCoordV, EnergyU, EnergyV, EnergyW)


	if currentEventId == pfo.EventId:
		eventPfos.append(PfoBeingRead)

	else:
		events.append(eventPfos)
		eventPfos = [PfoBeingRead]
		currentEventId = pfo.EventId

# The for loop does not append the event to the array
eventPfos.append(eventPfos)

print(events)
