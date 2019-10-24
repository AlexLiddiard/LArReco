import ROOT
f = ROOT.TFile.Open("PFOs.root", "read")
t = f.Get("PFO")
t.Print()
#t.Scan("EventId:PfoId:ParentPfoId:DaughterPfoIds:HierarchyTier:Vertex:DriftCoordW:WireCoordW:ElectromagneticEnergyW:HadronicEnergyW")
t.GetEntryWithIndex(0, 1)
t.Draw("DriftCoordU:WireCoordU", "PfoId == 1 && EventId == 0")
input()