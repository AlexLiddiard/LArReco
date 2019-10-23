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
    if (neutrinoPfos.size() == 1) // White this event if there is exactly one neutrino PFO in the event (which there should be)
    {
        this->WritePfo(neutrinoPfos.front());
    }

    m_EventId++;
    return STATUS_CODE_SUCCESS;
}


/*
Inputs:
const ParticleFlowObject *const pPfo	// The PFO to be written to the ROOT tree.
int pfoId				// The next available PFO ID in the ROOT tree, the PFO itself will be written with this ID.
int parentPfoId				// The PFO may have a parent, this is its PFO ID. If the PFO has no parent (i.e. is a neutrino) it should be -1 to indicate this.
int hierarchyTier			// The hierarchy tier of the PFO, e.g. the neutrino is tier 0, its daughters are tier 1, daughters of its daughters are tier 2, etc.

What it does:
Writes the PFO to the ROOT tree.
If the PFO has descendants (say n descendants) these are also written to the ROOT tree, and are allocated the PFO IDs from (pfoId + 1) to (pfoId + n).

Returns:
int: the total number of PFOs written, i.e. returns the value n + 1.
*/

int MyTrackShowerIdAlgorithm::WritePfo(const ParticleFlowObject *const pPfo ,int pfoId, int parentPfoId, int hierarchyTier)
{
    IntVector daughterPfoIds;	// daughterPfoIds = [empty vector of integers].
    int pfosWritten(0);		// pfosWritten = 0.
    for (const ParticleFlowObject *const daughterPfo : pPfo->GetDaughterPfoList()) // for each daughterPfo in pPfo.daughterPfos:
    {
        int daughterPfoId(pfoId + pfosWritten + 1); // daughterPfoId = pfoId + pfosWritten + 1
        daughterPfoIds.push_back(daughterPfoId); // put daughterPfoId into daughterPfoIds
        pfosWritten += this->WritePfo(daughterPfo, daughterPfoId, pfoId, hierarchyTier + 1); // pfosWritten += WritePfo(daughterPfoId, pfoId, daughterPfo)
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
    this->GetCaloHitInfo(pPfo, TPC_VIEW_U, &m_UViewHits);
    this->GetCaloHitInfo(pPfo, TPC_VIEW_V, &m_VViewHits);
    this->GetCaloHitInfo(pPfo, TPC_VIEW_W, &m_WViewHits);
    //this->GetCaloHitInfo(pPfo, TPC_3D, &m_ThreeDViewHits);
    const Vertex *vertex(LArPfoHelper::GetVertex(pPfo));
    const CartesianVector &vertexPosition(vertex->GetPosition());
    m_Vertex[0] = vertexPosition.GetX();
    m_Vertex[1] = vertexPosition.GetY();
    m_Vertex[2] = vertexPosition.GetZ();

    m_pPfoTree->Fill(); // Fill the tree
    pfosWritten += 1;

    // Delete any locally created objects
    //delete m_pDaughterPfoIds;
    return pfosWritten;	// return pfosWritten += 1.
}

void MyTrackShowerIdAlgorithm::GetCaloHitInfo(
    const ParticleFlowObject *const pPfo,
    HitType hitType,
    ViewHits *viewHits)
{
    viewHits->pXCoord->clear();
    viewHits->pYCoord->clear();
    viewHits->pZCoord->clear();
    viewHits->pXCoordError->clear();
    viewHits->pEnergy->clear();

    CaloHitList caloHitList;
    LArPfoHelper::GetCaloHits(pPfo, hitType, caloHitList);

    try
    {
        const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(&caloHitList));
        viewHits->mcPdgCode = pMCParticle->GetParticleId();
        const bool isTrack((E_MINUS != std::abs(viewHits->mcPdgCode)) && (PHOTON != std::abs(viewHits->mcPdgCode)));
        std::cout << "MyTrackShowerIdAlgorithm: Got a MCParticle (for this view), PDG code " << viewHits->mcPdgCode << ", isTrack " << isTrack << std::endl;
    }
    catch (const StatusCodeException &)
    {
    }

    for (const CaloHit *const caloHit : caloHitList)
    {
        const CartesianVector &positionVector(caloHit->GetPositionVector());

        viewHits->pXCoord->push_back(positionVector.GetX());
        //if (hitType == TPC_3D) // If hits are 3D we get the Y coordinate
        //{
        //    viewHits->pMCoord->push_back(positionVector.GetY());
        //}
        viewHits->pZCoord->push_back(positionVector.GetZ());
        viewHits->pEnergy->push_back(caloHit->GetInputEnergy());
        viewHits->pXCoordError->push_back(caloHit->GetCellSize1());
    }
}

MyTrackShowerIdAlgorithm::MyTrackShowerIdAlgorithm() :
    m_EventId(0),
    m_UViewHits{{},{},{},{},{},0},
    m_VViewHits{{},{},{},{},{},0},
    m_WViewHits{{},{},{},{},{},0},
    m_ThreeDViewHits{{},{},{},{},{},0}
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
    delete m_UViewHits.pXCoord;
    //delete m_UViewHits.pYCoord;
    delete m_UViewHits.pZCoord;
    delete m_UViewHits.pEnergy;
    delete m_UViewHits.pXCoordError;
    delete m_VViewHits.pXCoord;
    //delete m_VViewHits.pYCoord;
    delete m_VViewHits.pZCoord;
    delete m_VViewHits.pEnergy;
    delete m_VViewHits.pXCoordError;
    delete m_WViewHits.pXCoord;
    //delete m_WViewHits.pYCoord;
    delete m_WViewHits.pZCoord;
    delete m_WViewHits.pEnergy;
    delete m_WViewHits.pXCoordError;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode MyTrackShowerIdAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    // Read settings from xml file here
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "OutputTree", m_treeName));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "OutputFile", m_fileName));

    // Open/create tree file
    std::cout <<  "MyTrackShowerIdAlgorithm: Opening tree file." << std::endl;
    m_pTFile = new TFile(m_fileName.c_str(), "RECREATE");
    //m_pPfoTree = (TTree*)m_pTFile->Get(m_treeName.c_str());
    //if (!m_pPfoTree)
    //{
    //    std::cout <<  "MyTrackShowerIdAlgorithm: PFO tree not found, creating a new one." << std::endl;
        m_pPfoTree = new TTree(m_treeName.c_str(), "A tree of PFOs.");
        m_pPfoTree->Branch("EventId", &m_EventId);

        // PFO identification + relations
        m_pPfoTree->Branch("PfoId", &m_PfoId);
	m_pPfoTree->Branch("ParentPfoId", &m_ParentPfoId);
        m_pPfoTree->Branch("DaughterPfoIds", &m_pDaughterPfoIds);
        m_pPfoTree->Branch("HierarchyTier",  &m_HierarchyTier);

        // U view
        std::cout << "MyTrackShowerIdAlgorithm: X coord pointer before " << m_UViewHits.pXCoord << std::endl; // ROOT is initialising the pointers! Need to initialise it ourselves.
        m_pPfoTree->Branch("DriftCoordU",  &(m_UViewHits.pXCoord));
        std::cout << "MyTrackShowerIdAlgorithm: X coord pointer after " << m_UViewHits.pXCoord << std::endl;

        m_pPfoTree->Branch("DriftCoordErrorU",  &(m_UViewHits.pXCoordError));
        m_pPfoTree->Branch("WireCoordU",  &(m_UViewHits.pZCoord));
        m_pPfoTree->Branch("EnergyU",  &(m_UViewHits.pEnergy));
        m_pPfoTree->Branch("MCPdgCodeU",  &(m_UViewHits.mcPdgCode));

        // V view
        m_pPfoTree->Branch("DriftCoordV",  &(m_VViewHits.pXCoord));
        m_pPfoTree->Branch("DriftCoordErrorV",  &(m_VViewHits.pXCoordError));
        m_pPfoTree->Branch("WireCoordV",  &(m_VViewHits.pZCoord));
        m_pPfoTree->Branch("EnergyV",  &(m_VViewHits.pEnergy));
        m_pPfoTree->Branch("MCPdgCodeV",  &(m_VViewHits.mcPdgCode));

        // W view
        m_pPfoTree->Branch("DriftCoordW",  &(m_WViewHits.pXCoord));
        m_pPfoTree->Branch("DriftCoordErrorW",  &(m_WViewHits.pXCoordError));
        m_pPfoTree->Branch("WireCoordW",  &(m_WViewHits.pZCoord));
        m_pPfoTree->Branch("EnergyW",  &(m_WViewHits.pEnergy));
        m_pPfoTree->Branch("MCPdgCodeW",  &(m_WViewHits.mcPdgCode));

        // 3D view
        m_pPfoTree->Branch("XCoordThreeD",  &(m_ThreeDViewHits.pXCoord));
        m_pPfoTree->Branch("YCoordThreeD",  &(m_ThreeDViewHits.pYCoord));
        m_pPfoTree->Branch("ZCoordThreeD",  &(m_ThreeDViewHits.pZCoord));
        m_pPfoTree->Branch("EnergyThreeD",  &(m_ThreeDViewHits.pEnergy));
        m_pPfoTree->Branch("Vertex",  &m_Vertex, "m_Vertex[3]/F");

    //}
    /*else
    //{
       // TODO: Set m_EventId such that it is equal to next unused event number (won't be zero when events are already in the file)
	
        std::cout <<  "MyTrackShowerIdAlgorithm: Found an existing tree, already containing " << m_EventId << " events." << std::endl;
        m_pPfoTree->SetBranchAddress("EventId", &m_EventId);
        m_pPfoTree->SetBranchAddress("PfoId", &m_PfoId);
	m_pPfoTree->SetBranchAddress("ParentPfoId", &m_ParentPfoId);
        m_pPfoTree->SetBranchAddress("DaughterPfoIds", &m_pDaughterPfoIds);
        m_pPfoTree->SetBranchAddress("HierarchyTier",  &m_HierarchyTier);
        m_pPfoTree->SetBranchAddress("DriftCoordU",  &(m_UViewHits.pXCoord));
        m_pPfoTree->SetBranchAddress("DriftCoordV",  &(m_VViewHits.pXCoord));
        m_pPfoTree->SetBranchAddress("DriftCoordW",  &(m_WViewHits.pXCoord));
        m_pPfoTree->SetBranchAddress("WireCoordU",  &(m_UViewHits.pZCoord));
        m_pPfoTree->SetBranchAddress("WireCoordV",  &(m_VViewHits.pZCoord));
        m_pPfoTree->SetBranchAddress("WireCoordW",  &(m_WViewHits.pZCoord));
        m_pPfoTree->SetBranchAddress("EnergyU",  &(m_UViewHits.pEnergy));
        m_pPfoTree->SetBranchAddress("EnergyV",  &(m_VViewHits.pEnergy));
        m_pPfoTree->SetBranchAddress("EnergyW",  &(m_WViewHits.pEnergy));
        m_pPfoTree->SetBranchAddress("MCPdgCodeU",  &(m_UViewHits.mcPdgCode));
        m_pPfoTree->SetBranchAddress("MCPdgCodeV",  &(m_VViewHits.mcPdgCode));
        m_pPfoTree->SetBranchAddress("MCPdgCodeW",  &(m_WViewHits.mcPdgCode));
        m_pPfoTree->SetBranchAddress("DriftCoordErrorU",  &(m_UViewHits.pXCoordError));
        m_pPfoTree->SetBranchAddress("DriftCoordErrorV",  &(m_VViewHits.pXCoordError));
        m_pPfoTree->SetBranchAddress("DriftCoordErrorW",  &(m_WViewHits.pXCoordError));
        m_pPfoTree->SetBranchAddress("Vertex",  &m_Vertex);
    }
    */

    return STATUS_CODE_SUCCESS;
}
