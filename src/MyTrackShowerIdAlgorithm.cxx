/**
 *  @file   LArReco/src/MyTrackShowerIdAlgorithm.cc
 * 
 *  @brief  Implementation of my track shower ID algorithm class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"
#include "MyTrackShowerIdAlgorithm.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"

using namespace lar_content;

using namespace pandora;

StatusCode MyTrackShowerIdAlgorithm::Run()
{
    // Algorithm code here
    std::cout <<  "MyTrackShowerIdAlgorithm: Hello World!" << std::endl;

    const PfoList *pPfoList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pPfoList));
    
    PfoList neutrinoPfos;
    LArPfoHelper::GetRecoNeutrinos(pPfoList, neutrinoPfos);
    for (const ParticleFlowObject *const neutrinoPfo : neutrinoPfos) // for each neutrinoPfo (there might be some CRs reconstructed as neutrinos)
    {
        WritePfo(neutrinoPfo);
        m_EventId++;
    }
    return STATUS_CODE_SUCCESS;
}

int MyTrackShowerIdAlgorithm::WritePfo(const ParticleFlowObject *const pPfo ,int pfoId, int parentPfoId, int hierarchyTier)
{
    IntVector daughterPfoIds;	// daughterPfoIds = [empty vector of integers].
    int pfosWritten(0);		// pfosWritten = 0.
    for (const ParticleFlowObject *const daughterPfo : pPfo->GetDaughterPfoList()) // for each daughterPfo in pPfo.daughterPfos:
    {
        int daughterPfoId(pfoId + pfosWritten + 1); // daughterPfoId = pfoId + pfosWritten + 1
        daughterPfoIds.push_back(daughterPfoId); // put daughterPfoId into daughterPfoIds
        pfosWritten += WritePfo(daughterPfo, daughterPfoId, pfoId, hierarchyTier + 1); // pfosWritten += WritePfo(daughterPfoId, pfoId, daughterPfo)
    }
    std::cout << "MyTrackShowerIdAlgorithm: Writing a PFO to the tree!" << std::endl;
    std::cout << "MyTrackShowerIdAlgorithm: \tThe parent PFO ID is " << parentPfoId << std::endl;
    std::cout << "MyTrackShowerIdAlgorithm: \tThe PFO ID is " << pfoId << std::endl;

    if (daughterPfoIds.size() > 0)
    {
        std::cout << "MyTrackShowerIdAlgorithm: \tThe daughter PFO IDs are ";
        for (int daughterPfoId : daughterPfoIds)
        {
            std::cout << daughterPfoId << " ";
        }
        std::cout << std::endl;
    }
    else
    {
        std::cout << "MyTrackShowerIdAlgorithm: \tThe PFO has no daughters." << std::endl;
    }
    std::cout << "MyTrackShowerIdAlgorithm: \tThe hierarchy tier of this PFO is " << hierarchyTier << std::endl;

    m_ParentPfoId = parentPfoId; // Write parentPfoId to ROOT tree
    m_PfoId = pfoId; // Write pfoId to ROOT tree
    m_pDaughterPfoIds = &daughterPfoIds; // Write daughterPfoIds to ROOT tree
    m_HierarchyTier = hierarchyTier; // Write hierarchyTier to ROOT tree
    
    // Write all other properties of pPFO to ROOT tree   
    GetCaloHitInfo(pPfo, TPC_VIEW_U, &m_UCaloHits);
    GetCaloHitInfo(pPfo, TPC_VIEW_V, &m_VCaloHits);
    GetCaloHitInfo(pPfo, TPC_VIEW_W, &m_WCaloHits);

    m_pPfoTree->Fill(); // Fill the tree
    pfosWritten += 1;

    // Delete any locally created objects
    //delete m_pDaughterPfoIds;
    return pfosWritten;	// return pfosWritten += 1.
}

void MyTrackShowerIdAlgorithm::GetCaloHitInfo(
    const ParticleFlowObject *const pPfo,
    const HitType &hitType,
    PlaneCaloHits *planeCaloHits)
{
    planeCaloHits->driftCoord.clear();
    planeCaloHits->wireCoord.clear();
    planeCaloHits->electromagneticEnergy.clear();
    planeCaloHits->hadronicEnergy.clear();

    CaloHitList caloHitList;
    LArPfoHelper::GetCaloHits(pPfo, hitType, caloHitList);
    for (const CaloHit *const caloHit : caloHitList)
    {
        const CartesianVector &positionVector(caloHit->GetPositionVector());

        planeCaloHits->driftCoord.push_back(positionVector.GetX());
        planeCaloHits->wireCoord.push_back(positionVector.GetZ());
        planeCaloHits->electromagneticEnergy.push_back(caloHit->GetElectromagneticEnergy());
        planeCaloHits->hadronicEnergy.push_back(caloHit->GetHadronicEnergy());
        planeCaloHits->nHits++;
    }
}

MyTrackShowerIdAlgorithm::MyTrackShowerIdAlgorithm() :
    m_EventId(0),
    m_UCaloHits{0,{},{},{},{}},
    m_VCaloHits{0,{},{},{},{}},
    m_WCaloHits{0,{},{},{},{}}
{
}

MyTrackShowerIdAlgorithm::~MyTrackShowerIdAlgorithm()
{
    // Build index for PFOs
    m_pPfoTree->BuildIndex("EventId", "PfoId");

    // Save the root tree
    std::cout << "MyTrackShowerIdAlgorithm: Saving ROOT tree " << m_treeName << " to file " << m_fileName << std::endl;
    try
    {
        m_pPfoTree->Write();
        m_pTFile->Close();
    }
    catch (const StatusCodeException &)
    {
        std::cout << "MyTrackShowerIdAlgorithm: Unable to write tree!" << std::endl;
    }
    
    delete m_pTFile;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode MyTrackShowerIdAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    // Read settings from xml file here
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "OutputTree", m_treeName));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "OutputFile", m_fileName));

    // Open/create tree file
    std::cout <<  "MyTrackShowerIdAlgorithm: Opening tree file." << std::endl;
    m_pTFile = new TFile(m_fileName.c_str(), "UPDATE");
    m_pPfoTree = (TTree*)m_pTFile->Get(m_treeName.c_str());
    if (!m_pPfoTree)
    {
        std::cout <<  "MyTrackShowerIdAlgorithm: PFO tree not found, creating a new one." << std::endl;
        m_pPfoTree = new TTree("PFO","A PFO tree.");
        m_pPfoTree->Branch("EventId", &m_EventId);
        m_pPfoTree->Branch("PfoId", &m_PfoId);
	m_pPfoTree->Branch("ParentPfoId", &m_ParentPfoId);
        m_pPfoTree->Branch("DaughterPfoIds", &m_pDaughterPfoIds);
        m_pPfoTree->Branch("HierarchyTier",  &m_HierarchyTier);
        m_pPfoTree->Branch("DriftCoordU",  &(m_UCaloHits.driftCoord));
        m_pPfoTree->Branch("WireCoordU",  &(m_UCaloHits.wireCoord));
        m_pPfoTree->Branch("ElectromagneticEnergyU",  &(m_UCaloHits.electromagneticEnergy));
        m_pPfoTree->Branch("HadronicEnergyU",  &(m_UCaloHits.hadronicEnergy));
        m_pPfoTree->Branch("DriftCoordV",  &(m_VCaloHits.driftCoord));
        m_pPfoTree->Branch("WireCoordV",  &(m_VCaloHits.wireCoord));
        m_pPfoTree->Branch("ElectromagneticEnergyV",  &(m_VCaloHits.electromagneticEnergy));
        m_pPfoTree->Branch("HadronicEnergyV",  &(m_VCaloHits.hadronicEnergy));
        m_pPfoTree->Branch("DriftCoordW",  &(m_WCaloHits.driftCoord));
        m_pPfoTree->Branch("WireCoordW",  &(m_WCaloHits.wireCoord));
        m_pPfoTree->Branch("ElectromagneticEnergyW",  &(m_WCaloHits.electromagneticEnergy));
        m_pPfoTree->Branch("HadronicEnergyW",  &(m_WCaloHits.hadronicEnergy));
    }
    else
    {
       // TODO: Set m_EventId such that it is equal to next unused event number (won't be zero when events are already in the file)
	
        std::cout <<  "MyTrackShowerIdAlgorithm: Found an existing tree, already containing " << m_EventId << " events." << std::endl;
        m_pPfoTree->SetBranchAddress("EventId", &m_EventId);
        m_pPfoTree->SetBranchAddress("PfoId", &m_PfoId);
        m_pPfoTree->SetBranchAddress("ParentPfoId", &m_ParentPfoId);
        m_pPfoTree->SetBranchAddress("DaughterPfoIds", &m_pDaughterPfoIds);
        m_pPfoTree->SetBranchAddress("HierarchyTier",  &m_HierarchyTier);
        m_pPfoTree->SetBranchAddress("DriftCoordU",  &(m_UCaloHits.driftCoord));
        m_pPfoTree->SetBranchAddress("WireCoordU",  &(m_UCaloHits.wireCoord));
        m_pPfoTree->SetBranchAddress("ElectromagneticEnergyU",  &(m_UCaloHits.electromagneticEnergy));
        m_pPfoTree->SetBranchAddress("HadronicEnergyU",  &(m_UCaloHits.hadronicEnergy));
        m_pPfoTree->SetBranchAddress("DriftCoordV",  &(m_VCaloHits.driftCoord));
        m_pPfoTree->SetBranchAddress("WireCoordV",  &(m_VCaloHits.wireCoord));
        m_pPfoTree->SetBranchAddress("ElectromagneticEnergyV",  &(m_VCaloHits.electromagneticEnergy));
        m_pPfoTree->SetBranchAddress("HadronicEnergyV",  &(m_VCaloHits.hadronicEnergy));
        m_pPfoTree->SetBranchAddress("DriftCoordW",  &(m_WCaloHits.driftCoord));
        m_pPfoTree->SetBranchAddress("WireCoordW",  &(m_WCaloHits.wireCoord));
        m_pPfoTree->SetBranchAddress("ElectromagneticEnergyW",  &(m_WCaloHits.electromagneticEnergy));
        m_pPfoTree->SetBranchAddress("HadronicEnergyW",  &(m_WCaloHits.hadronicEnergy));
    }

    return STATUS_CODE_SUCCESS;
}
