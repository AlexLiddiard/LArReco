/**
 *  @file   LArReco/src/MyTrackShowerIdAlgorithm.cc
 * 
 *  @brief  Implementation of my track shower ID algorithm class.
 * 
 *  $Log: $
 */

#include "MyTrackShowerIdAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"

#include "larpandoracontent/LArHelpers/LArMonitoringHelper.h"
#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArPersistency/EventReadingAlgorithm.h"

using namespace lar_content;
using namespace pandora;

StatusCode MyTrackShowerIdAlgorithm::Run()
{
    std::cout << "\n---MyTrackShowerIdAlgorithm-----------------------------------------------------------------------" << std::endl;
    std::cout << "Processing next event, eventId " << m_EventId << std::endl;

    // Make sure the maps are empty for the next event
    m_selectiveMap.clear();
    m_pfoToMCHitSharingMap.clear();

    const PfoList *pPfoList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pPfoList));

    // Input lists
    const MCParticleList *pMCParticleList = nullptr;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_mcParticleListName, pMCParticleList));

    const CaloHitList *pCaloHitList = nullptr;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_caloHitListName, pCaloHitList));

    // Mapping target MCParticles -> truth associated Hits
    LArMCParticleHelper::MCContributionMap basicMCParticleToHitsMap;
    LArMCParticleHelper::CaloHitToMCMap caloHitToMCMap;
    LArMCParticleHelper::GetMCParticleToCaloHitMatches(pCaloHitList, LArMCParticleHelper::MCRelationMap(), caloHitToMCMap, basicMCParticleToHitsMap);

    // Mapping PFOs -> reconstructed calohit lists
    LArMCParticleHelper::PfoContributionMap pfoToHitsMap;
    this->GetPfoToHitsMap(*pPfoList, pfoToHitsMap);

    // Get Neutrino MCParticle
    MCParticleList parentMCNuList;
    this->GetParentNeutrino(pMCParticleList, parentMCNuList);
    if (parentMCNuList.size()) // Check that there is a MC parent neutrino
    {
        CaloHitList rejectedCaloHitList;
        std::cout << "\nGenerating MCParticle->CaloHit map..." << std::endl;
        this->Mapper(basicMCParticleToHitsMap, parentMCNuList.front(), false, rejectedCaloHitList, m_selectiveMap);
        std::cout << "Mapping complete, results:" << std::endl;
        this->PrintMCParticles(m_selectiveMap);
    }
    else
    {
        std::cout << "The event has no MC parent neutrinos!" << std::endl;
    }

    // Create hit sharing map
    LArMCParticleHelper::MCParticleToPfoHitSharingMap mcToPfoHitSharingMap;
    LArMCParticleHelper::GetPfoMCParticleHitSharingMaps(pfoToHitsMap, {m_selectiveMap}, m_pfoToMCHitSharingMap, mcToPfoHitSharingMap);

    // Get Neutrino PFO
    PfoList neutrinoPfos;
    LArPfoHelper::GetRecoNeutrinos(pPfoList, neutrinoPfos);
    if (neutrinoPfos.size()) // Write this event if there is a neutrino PFO
    {
        std::cout << "\nBegin collecting PFO data..." << std::endl;
        this->WritePfo(neutrinoPfos.front()); // there should be only one PFO in the list
    }
    else
    {
        std::cout << "The event has no reconstructed neutrinos!" << std::endl;
    }

    m_EventId++;
    return STATUS_CODE_SUCCESS;
}

// The original MCParticle helper version collects+filters all downstream hits. We just need a very simplistic map without any merging/filtering.
void MyTrackShowerIdAlgorithm::GetPfoToHitsMap(const PfoList &pPfoList, LArMCParticleHelper::PfoContributionMap &pfoToHitsMap)
{
    for (const ParticleFlowObject *const pPfo : pPfoList)
    {
        CaloHitList &caloHitList2D(pfoToHitsMap[pPfo]);
        LArPfoHelper::GetCaloHits(pPfo, TPC_VIEW_U, caloHitList2D);
        LArPfoHelper::GetCaloHits(pPfo, TPC_VIEW_V, caloHitList2D);
        LArPfoHelper::GetCaloHits(pPfo, TPC_VIEW_W, caloHitList2D);
        LArPfoHelper::GetIsolatedCaloHits(pPfo, TPC_VIEW_U, caloHitList2D);
        LArPfoHelper::GetIsolatedCaloHits(pPfo, TPC_VIEW_V, caloHitList2D);
        LArPfoHelper::GetIsolatedCaloHits(pPfo, TPC_VIEW_W, caloHitList2D);
    }
}



// Function which gets event parent neutrino -------------------------------------------------------------------------m-----------------------------------------------------------------------------------------------------------------------
int MyTrackShowerIdAlgorithm::GetParentNeutrino(const MCParticleList *const pMCParticleList, MCParticleList &parentMCNuList)
{
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
void MyTrackShowerIdAlgorithm::Mapper(const LArMCParticleHelper::MCContributionMap &basicMap, const MCParticle *const pMCParticle, const bool isShowerProduct, CaloHitList &caloHitsToMerge, LArMCParticleHelper::MCContributionMap &selectiveMap)
{
    // Get MCParticle PDGCode.
    int PDGCode = std::abs(pMCParticle->GetParticleId());

    // Looping over every daughterMCParticle.
    CaloHitList returnedCaloHits;
    for (const MCParticle *const pMCDaughter : pMCParticle->GetDaughterList())
    {
        // Run mapper on daughterMCParticles, setting isShowerProduct to true if the parent is a shower particle.
        Mapper(basicMap, pMCDaughter, (PDGCode == E_MINUS || PDGCode == PHOTON || isShowerProduct), returnedCaloHits, selectiveMap);
    }

    // Get the direct MCParticle calohits.
    CaloHitList mCPCaloHits;
    if (basicMap.count(pMCParticle) == 1)
    {
        mCPCaloHits = basicMap.at(pMCParticle);
    }

    std::back_insert_iterator<CaloHitList> caloHits_back_inserter = std::back_inserter(caloHitsToMerge);
    if (returnedCaloHits.size() + mCPCaloHits.size() > 20 && !isShowerProduct)
    {
        // Add this MCParticle and its hits to the map (instead of adding to a list of hits to be merged)
        caloHits_back_inserter = std::back_inserter(selectiveMap[pMCParticle]);
    }
    // Copy the direct hits if available
    std::copy(mCPCaloHits.begin(), mCPCaloHits.end(), caloHits_back_inserter);
    // Copy the hits that were returned from daughters (i.e. merge them with this MCParticle)
    std::copy(returnedCaloHits.begin(), returnedCaloHits.end(), caloHits_back_inserter);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void MyTrackShowerIdAlgorithm::GetBestMatchedMCParticleInfo(const ParticleFlowObject *const pPfo, ViewHits &UView, ViewHits &VView, ViewHits &WView)
{
    int bestMCParticlePdgCode(0);
    int nHitsSharedWithBestMCParticleTotal(0);
    const LArMCParticleHelper::MCParticleToSharedHitsVector &mcParticleToSharedHitsVector(m_pfoToMCHitSharingMap.at(pPfo));

    for (const LArMCParticleHelper::MCParticleCaloHitListPair pMCParticleCaloHitListPair : mcParticleToSharedHitsVector)
    {
        const LArMCParticle *const pLArMCParticle = dynamic_cast<const LArMCParticle*>(pMCParticleCaloHitListPair.first);
        const CaloHitList &allMCHits(m_selectiveMap.at(pMCParticleCaloHitListPair.first));
        const CaloHitList &associatedMCHits(pMCParticleCaloHitListPair.second); 
        if (associatedMCHits.size() > nHitsSharedWithBestMCParticleTotal)
        {
            // This is the current best matched MCParticle, to be stored.
            nHitsSharedWithBestMCParticleTotal = associatedMCHits.size();
            bestMCParticlePdgCode = pLArMCParticle->GetParticleId();

            m_mcPdgCode = bestMCParticlePdgCode;
            m_mcpEnergy = pLArMCParticle->GetEnergy();
            m_mcNuanceCode = pLArMCParticle->GetNuanceCode();
            UView.nHitsMatch = LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, associatedMCHits);
            UView.nHitsMcp = LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allMCHits);
            VView.nHitsMatch = LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, associatedMCHits);
            VView.nHitsMcp = LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allMCHits);  
            WView.nHitsMatch = LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, associatedMCHits);
            WView.nHitsMcp = LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allMCHits);
        }
    }

    if (bestMCParticlePdgCode)
    {
        std::cout << "Got best matching MC Particle, bestMCParticlePdgCode " << bestMCParticlePdgCode
                  << ", nHitsShared U: " << UView.nHitsMatch << " V: " <<  VView.nHitsMatch << " W: " << WView.nHitsMatch
                  << ", nHitsBestMatchMCP U: " << UView.nHitsMcp << " V: " << VView.nHitsMcp << " W: " << WView.nHitsMcp << std::endl;
    }
    else
    {
        std::cout << "Could not find a matching MC particle for this PFO!" << std::endl;
        UView.nHitsMatch = 0;
        UView.nHitsMcp = 0;    
        VView.nHitsMatch = 0;
        VView.nHitsMcp = 0;
        WView.nHitsMatch = 0;
        WView.nHitsMcp = 0;
    }
}

//Copied from MCParticle Monitoring Algorithm----------------------------------------------------------------------------------------------------------------------------------------------------------------
void MyTrackShowerIdAlgorithm::PrintMCParticles(const LArMCParticleHelper::MCContributionMap &mcContributionMap) const
{
    MCParticleVector mcPrimaryVector;
    LArMonitoringHelper::GetOrderedMCParticleVector({mcContributionMap}, mcPrimaryVector);

    unsigned int index(0);

    for (const MCParticle *const pMCPrimary : mcPrimaryVector)
    {
        const CaloHitList &caloHitList(mcContributionMap.at(pMCPrimary));

        if (caloHitList.size() >= 1)
        {
            std::cout << std::endl << "--Primary " << index << ", MCPDG " << pMCPrimary->GetParticleId() << ", Energy " << pMCPrimary->GetEnergy()
                      << ", Dist. " << (pMCPrimary->GetEndpoint() - pMCPrimary->GetVertex()).GetMagnitude() << ", nMCHits " << caloHitList.size()
                      << " (" << LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, caloHitList)
                      << ", " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, caloHitList)
                      << ", " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, caloHitList) << ")" << std::endl;

            LArMCParticleHelper::MCRelationMap mcToPrimaryMCMap;
            LArMCParticleHelper::CaloHitToMCMap caloHitToPrimaryMCMap;
            LArMCParticleHelper::MCContributionMap mcToTrueHitListMap;
            LArMCParticleHelper::GetMCParticleToCaloHitMatches(&caloHitList, mcToPrimaryMCMap, caloHitToPrimaryMCMap, mcToTrueHitListMap);
            this->PrintMCParticle(pMCPrimary, mcToTrueHitListMap, 1);
        }

        ++index;
    }
}
//Copied from MCParticle Monitoring Algorithm----------------------------------------------------------------------------------------------------------------------------------------------------------------
void MyTrackShowerIdAlgorithm::PrintMCParticle(const MCParticle *const pMCParticle, const LArMCParticleHelper::MCContributionMap &mcToTrueHitListMap,
    const int depth) const
{
    const CaloHitList &caloHitList(mcToTrueHitListMap.count(pMCParticle) ? mcToTrueHitListMap.at(pMCParticle) : CaloHitList());

    if (caloHitList.size() >= 1)
    {
        if (depth > 1)
        {
            for (int iDepth = 1; iDepth < depth - 1; ++iDepth) std::cout << "   ";
            std::cout << "\\_ ";
        }

        std::cout << "MCPDG " << pMCParticle->GetParticleId() << ", Energy " << pMCParticle->GetEnergy()
                  << ", Dist. " << (pMCParticle->GetEndpoint() - pMCParticle->GetVertex()).GetMagnitude() << ", nMCHits " << caloHitList.size()
                  << " (" << LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, caloHitList)
                  << ", " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, caloHitList)
                  << ", " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, caloHitList) << ")" << std::endl;
    }

    for (const MCParticle *const pDaughterParticle : pMCParticle->GetDaughterList())
        this->PrintMCParticle(pDaughterParticle, mcToTrueHitListMap, depth + 1);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

int MyTrackShowerIdAlgorithm::WritePfo(const ParticleFlowObject *const pPfo ,const int pfoId, const int parentPfoId, const int hierarchyTier)
{
    IntVector daughterPfoIds;	// daughterPfoIds = [empty vector of integers].
    int pfosWritten(0);		// pfosWritten = 0.
    for (const ParticleFlowObject *const daughterPfo : pPfo->GetDaughterPfoList()) // for each daughterPfo in pPfo.daughterPfos:
    {
        int daughterPfoId(pfoId + pfosWritten + 1); // daughterPfoId = pfoId + pfosWritten + 1
        daughterPfoIds.push_back(daughterPfoId); // put daughterPfoId into daughterPfoIds
        pfosWritten += this->WritePfo(daughterPfo, daughterPfoId, pfoId, hierarchyTier + 1); // pfosWritten += WritePfo(daughterPfoId, pfoId, daughterPfo)
    }

    std::cout << "Writing a PFO to the tree, pfoId " << pfoId << ", hierarchyTier " << hierarchyTier << ", parentPfoId " << parentPfoId;
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

    this->GetCaloHitInfo(pPfo, TPC_VIEW_U, m_UViewHits);
    this->GetCaloHitInfo(pPfo, TPC_VIEW_V, m_VViewHits);
    this->GetCaloHitInfo(pPfo, TPC_VIEW_W, m_WViewHits);
    this->GetCaloHitInfo(pPfo, TPC_3D, m_ThreeDViewHits);
    
    this->GetBestMatchedMCParticleInfo(pPfo, m_UViewHits, m_VViewHits, m_WViewHits);

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
        std::cout << "A vertex was not found for this PFO!" << std::endl;
    }

    m_pPfoTree->Fill(); // Fill the tree
    pfosWritten += 1;
    return pfosWritten;	// return pfosWritten += 1.
}

void MyTrackShowerIdAlgorithm::GetCaloHitInfo(
    const ParticleFlowObject *const pPfo,
    HitType hitType,
    ViewHits &viewHits)
{
    viewHits.pXCoord->clear();
    viewHits.pYCoord->clear();
    viewHits.pZCoord->clear();
    viewHits.pXCoordError->clear();
    viewHits.pEnergy->clear();

    CaloHitList caloHitList;
    LArPfoHelper::GetCaloHits(pPfo, hitType, caloHitList);
    LArPfoHelper::GetIsolatedCaloHits(pPfo, hitType, caloHitList);
    viewHits.nHitsPfo = caloHitList.size();
    for (const CaloHit *const caloHit : caloHitList)
    {
        const CartesianVector &positionVector(caloHit->GetPositionVector());

        viewHits.pXCoord->push_back(positionVector.GetX());
        if (hitType == TPC_3D) // If hits are 3D we get the Y coordinate
        {
            viewHits.pYCoord->push_back(positionVector.GetY());
        }
        viewHits.pZCoord->push_back(positionVector.GetZ());
        viewHits.pEnergy->push_back(caloHit->GetInputEnergy());
        viewHits.pXCoordError->push_back(caloHit->GetCellSize1());
    }

    std::string s;
    switch (hitType)
    {
        case TPC_VIEW_U: s = "U"; break;
        case TPC_VIEW_V: s = "V"; break;
        case TPC_VIEW_W: s = "W"; break;
        case TPC_3D: s = "3D"; break;
        default: s = ""; break;
    }
    std::cout << "Got " << viewHits.nHitsPfo << " calohits for this PFO in the " << s << " view." << std::endl;
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
    m_UViewHits{new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),0,0,0},
    m_VViewHits{new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),0,0,0},
    m_WViewHits{new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),0,0,0},
    m_ThreeDViewHits{new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),new FloatVector(),0,0,0}
{
}

MyTrackShowerIdAlgorithm::~MyTrackShowerIdAlgorithm()
{
    // Build index for PFOs
    m_pPfoTree->BuildIndex("eventId", "pfoId");

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
    m_pPfoTree->Branch("eventId", &m_EventId);

    // PFO identification + relations
    m_pPfoTree->Branch("pfoId", &m_PfoId);
    m_pPfoTree->Branch("parentPfoId", &m_ParentPfoId);
    m_pPfoTree->Branch("daughterPfoIds", &m_pDaughterPfoIds);
    m_pPfoTree->Branch("hierarchyTier", &m_HierarchyTier);

    // Simulation info
    m_pPfoTree->Branch("mcNuanceCode", &m_mcNuanceCode);
    m_pPfoTree->Branch("mcPdgCode", &m_mcPdgCode);
    m_pPfoTree->Branch("mcpEnergy", &m_mcpEnergy);

    // U view
    m_pPfoTree->Branch("driftCoordU", &(m_UViewHits.pXCoord));
    m_pPfoTree->Branch("driftCoordErrorU", &(m_UViewHits.pXCoordError));
    m_pPfoTree->Branch("wireCoordU", &(m_UViewHits.pZCoord));
    m_pPfoTree->Branch("energyU", &(m_UViewHits.pEnergy));
    m_pPfoTree->Branch("nHitsPfoU", &(m_UViewHits.nHitsPfo));
    m_pPfoTree->Branch("nHitsMcpU", &(m_UViewHits.nHitsMcp));
    m_pPfoTree->Branch("nHitsMatchU", &(m_UViewHits.nHitsMatch));

    // V view
    m_pPfoTree->Branch("driftCoordV", &(m_VViewHits.pXCoord));
    m_pPfoTree->Branch("driftCoordErrorV", &(m_VViewHits.pXCoordError));
    m_pPfoTree->Branch("wireCoordV", &(m_VViewHits.pZCoord));
    m_pPfoTree->Branch("energyV", &(m_VViewHits.pEnergy));
    m_pPfoTree->Branch("nHitsPfoV", &(m_VViewHits.nHitsPfo));
    m_pPfoTree->Branch("nHitsMcpV", &(m_VViewHits.nHitsMcp));
    m_pPfoTree->Branch("nHitsMatchV", &(m_VViewHits.nHitsMatch));

    // W view
    m_pPfoTree->Branch("driftCoordW", &(m_WViewHits.pXCoord));
    m_pPfoTree->Branch("driftCoordErrorW", &(m_WViewHits.pXCoordError));
    m_pPfoTree->Branch("wireCoordW", &(m_WViewHits.pZCoord));
    m_pPfoTree->Branch("energyW", &(m_WViewHits.pEnergy));
    m_pPfoTree->Branch("nHitsPfoW", &(m_WViewHits.nHitsPfo));
    m_pPfoTree->Branch("nHitsMcpW", &(m_WViewHits.nHitsMcp));
    m_pPfoTree->Branch("nHitsMatchW", &(m_WViewHits.nHitsMatch));

    // 3D view
    m_pPfoTree->Branch("xCoordThreeD", &(m_ThreeDViewHits.pXCoord));
    m_pPfoTree->Branch("yCoordThreeD", &(m_ThreeDViewHits.pYCoord));
    m_pPfoTree->Branch("zCoordThreeD", &(m_ThreeDViewHits.pZCoord));
    m_pPfoTree->Branch("energyThreeD", &(m_ThreeDViewHits.pEnergy));
    m_pPfoTree->Branch("vertex", &m_Vertex, "m_Vertex[3]/F");

    return STATUS_CODE_SUCCESS;
}
