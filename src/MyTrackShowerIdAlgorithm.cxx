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

    const PfoList *pPfoList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pPfoList));

    // Mapping reconstructed particles -> reconstruction associated Hits
    // TODO take care when ensuring Pfos accessed here are same as in your loop below - any selection here may mean info not available below
    PfoList allConnectedPfos;
    LArPfoHelper::GetAllConnectedPfos(*pPfoList, allConnectedPfos);

    PfoList myPfoList;
    for (const ParticleFlowObject *const pPfo : allConnectedPfos)
    {
        if (LArPfoHelper::IsFinalState(pPfo))
            myPfoList.push_back(pPfo);
    }

    // Input lists
    const MCParticleList *pMCParticleList = nullptr;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_mcParticleListName, pMCParticleList));

    const CaloHitList *pCaloHitList = nullptr;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_caloHitListName, pCaloHitList));

    // Mapping target MCParticles -> truth associated Hits
    LArMCParticleHelper::MCContributionMap targetMCParticleToHitsMap;
    LArMCParticleHelper::SelectReconstructableMCParticles(pMCParticleList, pCaloHitList, LArMCParticleHelper::PrimaryParameters(), LArMCParticleHelper::IsBeamNeutrinoFinalState, targetMCParticleToHitsMap);

    LArMCParticleHelper::PfoContributionMap pfoToHitsMap;
    LArMCParticleHelper::GetPfoToReconstructable2DHitsMap(myPfoList, targetMCParticleToHitsMap, pfoToHitsMap);

    // Last step
    LArMCParticleHelper::PfoToMCParticleHitSharingMap pfoToMCHitSharingMap;
    LArMCParticleHelper::MCParticleToPfoHitSharingMap mcToPfoHitSharingMap;
    LArMCParticleHelper::GetPfoMCParticleHitSharingMaps(pfoToHitsMap, {targetMCParticleToHitsMap}, pfoToMCHitSharingMap, mcToPfoHitSharingMap);

    //PandoraMonitoringApi::SetEveDisplayParameters(this->GetPandora(), true, DETECTOR_VIEW_XZ, -1.f, -1.f, 1.f);
    //PandoraMonitoringApi::VisualizeParticleFlowObjects(this->GetPandora(), &myPfoList, “MyPfoList", RED);
    //PandoraMonitoringApi::ViewEvent(this->GetPandora());

    for (const Pfo *const pPfo : myPfoList)
    {
        const CaloHitList &allHitsInPfo(pfoToHitsMap.at(pPfo));
        std::cout << "We got a pfo, isNeutrinoFinalState " << LArPfoHelper::IsNeutrinoFinalState(pPfo) << ", nHits " << allHitsInPfo.size()
                  << " (U: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allHitsInPfo) << ", V: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allHitsInPfo) << ", W: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allHitsInPfo) << ") " << std::endl;
        //const int nHitsInPfoTotal(allHitsInPfo.size()), nHitsInPfoU(LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allHitsInPfo)), nHitsInPfoV(LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allHitsInPfo)), nHitsInPfoW(LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allHitsInPfo));

        //CaloHitList wHitsInPfo;
        //LArPfoHelper::GetCaloHits(pPfo, TPC_VIEW_W, wHitsInPfo);

        //PandoraMonitoringApi::VisualizeCaloHits(this->GetPandora(), &wHitsInPfo, "WHitsInThisPfo", CYAN);
        //PandoraMonitoringApi::ViewEvent(this->GetPandora());

        int nHitsInBestMCParticleTotal(-1), nHitsInBestMCParticleU(-1), nHitsInBestMCParticleV(-1), nHitsInBestMCParticleW(-1), bestMCParticlePdgCode(0), bestMCParticleIsTrack(-1);
        int nHitsSharedWithBestMCParticleTotal(-1), nHitsSharedWithBestMCParticleU(-1), nHitsSharedWithBestMCParticleV(-1), nHitsSharedWithBestMCParticleW(-1);

        const LArMCParticleHelper::MCParticleToSharedHitsVector &mcParticleToSharedHitsVector(pfoToMCHitSharingMap.at(pPfo));

        for (const LArMCParticleHelper::MCParticleCaloHitListPair &mcParticleCaloHitListPair : mcParticleToSharedHitsVector)
        {
            const pandora::MCParticle *const pAssociatedMCParticle(mcParticleCaloHitListPair.first);

            const CaloHitList &allMCHits(targetMCParticleToHitsMap.at(pAssociatedMCParticle));
            std::cout << "Associated MCParticle: " << pAssociatedMCParticle->GetParticleId() << ", nTotalHits " << allMCHits.size()
                      << " (U: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allMCHits) << ", V: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allMCHits) << ", W: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allMCHits) << ") " << std::endl;

            const CaloHitList &associatedMCHits(mcParticleCaloHitListPair.second);
            
/*
            CaloHitList associatedMCHitsW;
            for (const CaloHit *const pCaloHit : associatedMCHits)
            {
                if (TPC_VIEW_W == pCaloHit->GetHitType())
                    associatedMCHitsW.push_back(pCaloHit);
            }
*/

            std::cout << "Shared with MCParticle: " << pAssociatedMCParticle->GetParticleId() << ", nSharedHits " << associatedMCHits.size()
                      << " (U: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, associatedMCHits) << ", V: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, associatedMCHits) << ", W: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, associatedMCHits) << ") " << std::endl;

            // This is the current best matched MCParticle, to be stored
            std::cout << "associatedMCHits.size() " << associatedMCHits.size() << ", nHitsSharedWithBestMCParticleTotal " << nHitsSharedWithBestMCParticleTotal << ", check " << associatedMCHits.size() > nHitsSharedWithBestMCParticleTotal) << std::endl;

            if (associatedMCHits.size() > nHitsSharedWithBestMCParticleTotal)
            {
                 nHitsSharedWithBestMCParticleTotal = associatedMCHits.size();
                 nHitsSharedWithBestMCParticleU = LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, associatedMCHits);
                 nHitsSharedWithBestMCParticleV = LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, associatedMCHits);
                 nHitsSharedWithBestMCParticleW = LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, associatedMCHits);

                 nHitsInBestMCParticleTotal = allMCHits.size();
                 nHitsInBestMCParticleU = LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allMCHits);
                 nHitsInBestMCParticleV = LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allMCHits);
                 nHitsInBestMCParticleW = LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allMCHits);

                 bestMCParticlePdgCode = pAssociatedMCParticle->GetParticleId();
                 bestMCParticleIsTrack = ((PHOTON != pAssociatedMCParticle->GetParticleId()) && (E_MINUS != std::abs(pAssociatedMCParticle->GetParticleId())) ? 1 : 0);
            }

            //PandoraMonitoringApi::VisualizeCaloHits(this->GetPandora(), &associatedMCHitsW, "associatedMCHitsW", GREEN);
            //PandoraMonitoringApi::ViewEvent(this->GetPandora());
        }

		// TODO remove
		if (nHitsInBestMCParticleTotal > 0) {nHitsInBestMCParticleTotal = 0;}
		if (nHitsInBestMCParticleU > 0) {nHitsInBestMCParticleU = 0;}
		if (nHitsInBestMCParticleV > 0) {nHitsInBestMCParticleV = 0;}
		if (nHitsInBestMCParticleW > 0) {nHitsInBestMCParticleW = 0;}
		if (bestMCParticlePdgCode > 0) {bestMCParticlePdgCode = 0;}
		if (bestMCParticleIsTrack > 0) {bestMCParticleIsTrack = 0;}
		if (nHitsSharedWithBestMCParticleU > 0) {nHitsSharedWithBestMCParticleU = 0;}
		if (nHitsSharedWithBestMCParticleV > 0) {nHitsSharedWithBestMCParticleV = 0;}
		if (nHitsSharedWithBestMCParticleW > 0) {nHitsSharedWithBestMCParticleW = 0;}
    }

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
