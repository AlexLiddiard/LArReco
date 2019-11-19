/**
 *  @file   LArReco/src/MyTrackShowerIdAlgorithm.cc
 * 
 *  @brief  Implementation of my track shower ID algorithm class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "MyTrackShowerIdAlgorithm.h"

#include "larpandoracontent/LArHelpers/LArMonitoringHelper.h"
#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArPersistency/EventReadingAlgorithm.h"

using namespace lar_content;

using namespace pandora;

StatusCode MyTrackShowerIdAlgorithm::Run()
{
    std::cout <<  "MyTrackShowerIdAlgorithm: Processing next event, eventId " << m_EventId << std::endl;

    // Make sure the maps are empty for the next event
    m_selectiveMap.clear();
    m_pfoToMCHitSharingMap.clear();

    const PfoList *pPfoList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pPfoList));

    // Mapping reconstructed particles -> reconstruction associated Hits
    // TODO take care when ensuring Pfos accessed here are same as in your loop below - any selection here may mean info not available below
    std::cout <<  "Mapping reconstructed particles -> reconstruction associated Hits" << std::endl;
    PfoList allConnectedPfos;
    LArPfoHelper::GetAllConnectedPfos(*pPfoList, allConnectedPfos);

    // Input lists
    std::cout <<  "Input lists" << std::endl;
    const MCParticleList *pMCParticleList = nullptr;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_mcParticleListName, pMCParticleList));

    const CaloHitList *pCaloHitList = nullptr;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_caloHitListName, pCaloHitList));

    // Mapping target MCParticles -> truth associated Hits
    std::cout <<  "Mapping target MCParticles -> truth associated Hits" << std::endl;
    LArMCParticleHelper::MCContributionMap basicMCParticleToHitsMap;
    
    //this->GetCompleteMCParticleMap(pCaloHitList, basicMCParticleToHitsMap);
    const LArMCParticleHelper::MCRelationMap emptyMCPrimaryMap;
    LArMCParticleHelper::CaloHitToMCMap caloHitToMCMap;
    LArMCParticleHelper::GetMCParticleToCaloHitMatches(pCaloHitList, emptyMCPrimaryMap, caloHitToMCMap, basicMCParticleToHitsMap);

    // Get Neutrino MCParticle
    std::cout << "Get Neutrino MCParticle " << std::endl;
    MCParticleList parentMCNuList; 
    this->GetParentNeutrino(pMCParticleList, parentMCNuList);
    std::cout << "Found this many parent neutrinos: " << parentMCNuList.size() << std::endl;

    CaloHitList rejectedCaloHitList;
    // Assuming there is one incident neutrino in the MCParticleList (also being very lazy and only mapping for the first event).
    this->Mapper(basicMCParticleToHitsMap, parentMCNuList.front(), false, rejectedCaloHitList, m_selectiveMap);
    std::cout << rejectedCaloHitList.size() << std::endl;

    LArMCParticleHelper::PfoContributionMap pfoToHitsMap;
    LArMCParticleHelper::GetPfoToReconstructable2DHitsMap(allConnectedPfos, m_selectiveMap, pfoToHitsMap);

    // Last step
    LArMCParticleHelper::MCParticleToPfoHitSharingMap mcToPfoHitSharingMap;
    LArMCParticleHelper::GetPfoMCParticleHitSharingMaps(pfoToHitsMap, {m_selectiveMap}, m_pfoToMCHitSharingMap, mcToPfoHitSharingMap);

    PfoList neutrinoPfos;
    LArPfoHelper::GetRecoNeutrinos(pPfoList, neutrinoPfos);
    if (neutrinoPfos.size()) // Write this event if there is a neutrino PFO
    {
        this->WritePfo(neutrinoPfos.front()); // there should be only one PFO in the list
    }
    else
    {
        std::cout << "MyTrackShowerIdAlgorithm: The event has no reconstructed neutrinos!" << std::endl;
    }

    m_EventId++;
    return STATUS_CODE_SUCCESS;
}

// Function which gets event parent neutrino.
int MyTrackShowerIdAlgorithm::GetParentNeutrino(const MCParticleList *const pMCParticleList, MCParticleList &parentMCNuList){
    for (const MCParticle *const mCParticle : *pMCParticleList)
    {
        if (mCParticle->GetParentList().empty() && LArMCParticleHelper::IsNeutrino(mCParticle))
        {
            parentMCNuList.push_back(mCParticle);
        }
    }
    return STATUS_CODE_SUCCESS;
}

// Mapper Function ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MyTrackShowerIdAlgorithm::Mapper(const LArMCParticleHelper::MCContributionMap &basicMap, const MCParticle *const pMCParticle, bool isShowerProduct, CaloHitList &caloHitsToMerge, LArMCParticleHelper::MCContributionMap &selectiveMap){
    // Create new list which holds calohits to be added to map if necessary.
    //std::cout << "Create new list which holds calohits to be added to map if necessary." << std::endl;
    CaloHitList newMCPCaloHits;

    // Get direct MCParticle calohits.
    //std::cout << "Get direct MCParticle calohits." << std::endl;
    if (basicMap.count(pMCParticle) == 1)
    {
        const CaloHitList &mCPCaloHits(basicMap.at(pMCParticle));
        // Copy the direct hits into the newCaloHitsList as they would definitely be added to the map.
        //std::cout << "Copy the direct hits into the newCaloHitsList as they would definitely be added to the map." << std::endl;
        std::copy(mCPCaloHits.begin(), mCPCaloHits.end(), std::back_inserter(selectiveMap[pMCParticle]));
    }
    // Get MCParticle PDGCode.
    //std::cout << "Get MCParticle PDGCode." << std::endl;
    int PDGCode = std::abs(pMCParticle->GetParticleId());
 
    // Looping over every daughterMCParticle.
    //std::cout << "Looping over every daughterMCParticle." << std::endl;
    CaloHitList returnedCaloHits;
    for (const MCParticle *const pMCDaughter : pMCParticle->GetDaughterList())
    {
        // Run mapper on daughterMCParticles, setting isShowerProduct to true if the daughter is a shower particle.
        //std::cout << "Run mapper on daughterMCParticles, setting isShowerProduct to true if the daughter is a shower particle." << std::endl;
        Mapper(basicMap, pMCDaughter, (PDGCode == E_MINUS || PDGCode == PHOTON), returnedCaloHits, selectiveMap);
        // Merge the returned caloHits into the new MCParticle caloHitList.
        //std::cout << "Merge the returned caloHits into the new MCParticle caloHitList." << std::endl;
    }
    std::copy(returnedCaloHits.end(), returnedCaloHits.begin(), std::back_inserter(newMCPCaloHits));

    if (newMCPCaloHits.size() > 20 && isShowerProduct == false)
    {
        //selectiveMap.insert ( std::pair<const MCParticle*, CaloHitList>(pMCParticle,newMCPCaloHits) );
        std::copy(newMCPCaloHits.begin(), newMCPCaloHits.end(), std::back_inserter(selectiveMap[pMCParticle]));
    }
    else
    {
        std::copy(newMCPCaloHits.begin(), newMCPCaloHits.end(), std::back_inserter(caloHitsToMerge));
    }
    std::cout << "Hits in newMCPCaloHits: " << newMCPCaloHits.size() << std::endl;
    std::cout << "Hits in caloHitsToMerge: " << caloHitsToMerge.size() << std::endl;
    LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, newMCPCaloHits);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void MyTrackShowerIdAlgorithm::GetBestMatchedMCParticleInfo(const ParticleFlowObject *const pPfo, HitType hitType, int &pBestMCParticlePdgCode, int &nHitsShared, int &nHitsBestMCParticle){
    std::cout << "Getting best matched MCParticle" << std::endl;
    int nHitsSharedWithBestMCParticleTotal(-1);

    const LArMCParticleHelper::MCParticleToSharedHitsVector &mcParticleToSharedHitsVector(m_pfoToMCHitSharingMap.at(pPfo));

    for (const LArMCParticleHelper::MCParticleCaloHitListPair pMCParticleCaloHitListPair : mcParticleToSharedHitsVector)
    {
        const MCParticle *const mcParticle(pMCParticleCaloHitListPair.first);
        const CaloHitList &allMCHits(m_selectiveMap.at(pMCParticleCaloHitListPair.first));
        const CaloHitList &associatedMCHits(pMCParticleCaloHitListPair.second);
        if (associatedMCHits.size() > nHitsSharedWithBestMCParticleTotal)
        {
            // This is the current best matched MCParticle, to be stored.
            nHitsSharedWithBestMCParticleTotal = associatedMCHits.size();
            pBestMCParticlePdgCode = mcParticle->GetParticleId();
            nHitsShared = LArMonitoringHelper::CountHitsByType(hitType, associatedMCHits);
            nHitsBestMCParticle = LArMonitoringHelper::CountHitsByType(hitType, allMCHits);
        }
    }
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

    std::cout << "MyTrackShowerIdAlgorithm: Writing a PFO to the tree, pfoId " << pfoId << ", hierarchyTier " << hierarchyTier << ", parentPfoId " << parentPfoId;
    if (daughterPfoIds.size() > 0)
    {
        std::cout << ", daughterPfoIds ";
        for (int daughterPfoId : daughterPfoIds)
        {
            std::cout << daughterPfoId << " ";
        }
    }
    std::cout << std::endl;

    m_ParentPfoId = parentPfoId; // Write parentPfoId to ROOT tree
    m_PfoId = pfoId; // Write pfoId to ROOT tree
    m_pDaughterPfoIds = &daughterPfoIds; // Write daughterPfoIds to ROOT tree
    m_HierarchyTier = hierarchyTier; // Write hierarchyTier to ROOT tree
    
    // Write all other properties of pPFO to ROOT tree

    this->GetCaloHitInfo(pPfo, TPC_VIEW_U, &m_UViewHits);
    this->GetCaloHitInfo(pPfo, TPC_VIEW_V, &m_VViewHits);
    this->GetCaloHitInfo(pPfo, TPC_VIEW_W, &m_WViewHits);
    this->GetCaloHitInfo(pPfo, TPC_3D, &m_ThreeDViewHits);

    try
    {
        const Vertex *vertex(LArPfoHelper::GetVertex(pPfo));
        const CartesianVector &vertexPosition(vertex->GetPosition());
        m_Vertex[0] = vertexPosition.GetX();
        m_Vertex[1] = vertexPosition.GetY();
        m_Vertex[2] = vertexPosition.GetZ();
    }
    catch (const StatusCodeException &)
    {
        std::cout << "MyTrackShowerIdAlgorithm: A vertex was not found for this PFO!" << std::endl;
    }

    m_pPfoTree->Fill(); // Fill the tree
    pfosWritten += 1;
    return pfosWritten;	// return pfosWritten += 1.
}

void MyTrackShowerIdAlgorithm::GetCaloHitInfo(
    const ParticleFlowObject *const pPfo,
    HitType hitType,
    ViewHits *viewHits)
{
    std::string s;
    switch (hitType)
    {
        case TPC_VIEW_U: s = "U"; break;
        case TPC_VIEW_V: s = "V"; break;
        case TPC_VIEW_W: s = "W"; break;
        case TPC_3D: s = "3D"; break;
        default: s = ""; break;
    }
    std::cout << "MyTrackShowerIdAlgorithm: Getting calohit info for  " << s << " view." << std::endl;

    viewHits->pXCoord->clear();
    viewHits->pYCoord->clear();
    viewHits->pZCoord->clear();
    viewHits->pXCoordError->clear();
    viewHits->pEnergy->clear();
    viewHits->nHitsPfo = 0;
    viewHits->nHitsMcp = 0;
    viewHits->nHitsMatch = 0;
    viewHits->nHitsMatch = 0;
    viewHits->nHitsMatch = 0;
    viewHits->nHitsMcp = 0;
    viewHits->mcPdgCode = 0;

    CaloHitList caloHitList;
    LArPfoHelper::GetCaloHits(pPfo, hitType, caloHitList);


    GetBestMatchedMCParticleInfo(pPfo, hitType, viewHits->mcPdgCode, viewHits->nHitsMatch, viewHits->nHitsMcp);

    for (const CaloHit *const caloHit : caloHitList)
    {
        const CartesianVector &positionVector(caloHit->GetPositionVector());

        viewHits->pXCoord->push_back(positionVector.GetX());
        if (hitType == TPC_3D) // If hits are 3D we get the Y coordinate
        {
            viewHits->pYCoord->push_back(positionVector.GetY());
        }
        viewHits->pZCoord->push_back(positionVector.GetZ());
        viewHits->pEnergy->push_back(caloHit->GetInputEnergy());
        viewHits->pXCoordError->push_back(caloHit->GetCellSize1());
        viewHits->nHitsPfo++;
    }
}

// Gets a file name (without extension) from a file path 
std::string MyTrackShowerIdAlgorithm::GetFileName(const std::string& filePath)
{
    std::size_t start = filePath.find_last_of("/\\") + 1;
    std::size_t end = filePath.find_last_of(".");
    return filePath.substr(start, end - start);
}

MyTrackShowerIdAlgorithm::MyTrackShowerIdAlgorithm() :
    m_EventId(0),
    m_UViewHits{new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),0,0,0,0},
    m_VViewHits{new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),0,0,0,0},
    m_WViewHits{new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),0,0,0,0},
    m_ThreeDViewHits{new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),0,0,0,0}
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
    
    // Clean up
    delete m_pTFile;
    delete m_UViewHits.pXCoord;
    delete m_UViewHits.pYCoord;
    delete m_UViewHits.pZCoord;
    delete m_UViewHits.pEnergy;
    delete m_UViewHits.pXCoordError;
    delete m_VViewHits.pXCoord;
    delete m_VViewHits.pYCoord;
    delete m_VViewHits.pZCoord;
    delete m_VViewHits.pEnergy;
    delete m_VViewHits.pXCoordError;
    delete m_WViewHits.pXCoord;
    delete m_WViewHits.pYCoord;
    delete m_WViewHits.pZCoord;
    delete m_WViewHits.pEnergy;
    delete m_WViewHits.pXCoordError;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode MyTrackShowerIdAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    // Read settings from xml file here
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "CaloHitListName", m_caloHitListName));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "MCParticleListName", m_mcParticleListName));

    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "OutputTree", m_treeName));
    const StatusCode statusCode(XmlHelper::ReadValue(xmlHandle, "OutputFile", m_fileName));
    if (STATUS_CODE_SUCCESS != statusCode) // If there is no name given, use the same name as the input file (with extension changed to .root)
    {
        EventReadingAlgorithm::ExternalEventReadingParameters *pExternalParameters(nullptr);
        pExternalParameters = dynamic_cast<EventReadingAlgorithm::ExternalEventReadingParameters*>(this->GetExternalParameters());
        m_fileName = this->GetFileName(pExternalParameters->m_eventFileNameList).append(".root"); // Assumes there is a single event file being processed (otherwise it will use the name of the last file)
        std::cout << "File name: " << m_fileName << std::endl;
    }

    // Open/create tree file
    std::cout <<  "MyTrackShowerIdAlgorithm: Creating tree file." << std::endl;
    m_pTFile = new TFile(m_fileName.c_str(), "RECREATE");
    m_pPfoTree = new TTree(m_treeName.c_str(), "A tree of PFOs.");
    m_pPfoTree->Branch("EventId", &m_EventId);

    // PFO identification + relations
    m_pPfoTree->Branch("PfoId", &m_PfoId);
    m_pPfoTree->Branch("ParentPfoId", &m_ParentPfoId);
    m_pPfoTree->Branch("DaughterPfoIds", &m_pDaughterPfoIds);
    m_pPfoTree->Branch("HierarchyTier",  &m_HierarchyTier);

    // U view
    m_pPfoTree->Branch("DriftCoordU",  &(m_UViewHits.pXCoord));
    m_pPfoTree->Branch("DriftCoordErrorU",  &(m_UViewHits.pXCoordError));
    m_pPfoTree->Branch("WireCoordU",  &(m_UViewHits.pZCoord));
    m_pPfoTree->Branch("EnergyU",  &(m_UViewHits.pEnergy));
    m_pPfoTree->Branch("MCPdgCodeU",  &(m_UViewHits.mcPdgCode));
    m_pPfoTree->Branch("nHitsPfoU",  &(m_UViewHits.nHitsPfo));
    m_pPfoTree->Branch("nHitsMcpU",  &(m_UViewHits.nHitsMcp));
    m_pPfoTree->Branch("nHitsMatchU",  &(m_UViewHits.nHitsMatch));

    // V view
    m_pPfoTree->Branch("DriftCoordV",  &(m_VViewHits.pXCoord));
    m_pPfoTree->Branch("DriftCoordErrorV",  &(m_VViewHits.pXCoordError));
    m_pPfoTree->Branch("WireCoordV",  &(m_VViewHits.pZCoord));
    m_pPfoTree->Branch("EnergyV",  &(m_VViewHits.pEnergy));
    m_pPfoTree->Branch("MCPdgCodeV",  &(m_VViewHits.mcPdgCode));
    m_pPfoTree->Branch("nHitsPfoV",  &(m_VViewHits.nHitsPfo));
    m_pPfoTree->Branch("nHitsMcpV",  &(m_VViewHits.nHitsMcp));
    m_pPfoTree->Branch("nHitsMatchV",  &(m_VViewHits.nHitsMatch));

    // W view
    m_pPfoTree->Branch("DriftCoordW",  &(m_WViewHits.pXCoord));
    m_pPfoTree->Branch("DriftCoordErrorW",  &(m_WViewHits.pXCoordError));
    m_pPfoTree->Branch("WireCoordW",  &(m_WViewHits.pZCoord));
    m_pPfoTree->Branch("EnergyW",  &(m_WViewHits.pEnergy));
    m_pPfoTree->Branch("MCPdgCodeW",  &(m_WViewHits.mcPdgCode));
    m_pPfoTree->Branch("nHitsPfoW",  &(m_WViewHits.nHitsPfo));
    m_pPfoTree->Branch("nHitsMcpW",  &(m_WViewHits.nHitsMcp));
    m_pPfoTree->Branch("nHitsMatchW",  &(m_WViewHits.nHitsMatch));

    // 3D view
    m_pPfoTree->Branch("XCoordThreeD",  &(m_ThreeDViewHits.pXCoord));
    m_pPfoTree->Branch("YCoordThreeD",  &(m_ThreeDViewHits.pYCoord));
    m_pPfoTree->Branch("ZCoordThreeD",  &(m_ThreeDViewHits.pZCoord));
    m_pPfoTree->Branch("EnergyThreeD",  &(m_ThreeDViewHits.pEnergy));
    m_pPfoTree->Branch("Vertex",  &m_Vertex, "m_Vertex[3]/F");

    return STATUS_CODE_SUCCESS;
}
