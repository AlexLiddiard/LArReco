import ROOT
f = ROOT.TFile.Open("PFOs.root", "read")
t = f.Get("PFO")
t.Print()
t.Scan("EventId:PfoId:ParentPfoId:DaughterPfoIds:HierarchyTier:Vertex:DriftCoordU:WireCoordU:ElectromagneticEnergyU:HadronicEnergyU")
