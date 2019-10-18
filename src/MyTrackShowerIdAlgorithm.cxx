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
    
    //const PfoList *pNeutrinoPfoList(nullptr);
    PfoList neutrinoPfos;
    LArPfoHelper::GetRecoNeutrinos(pPfoList, neutrinoPfos);
    if (neutrinoPfos.size() == 1)
    {
        WritePfo(0, -1, neutrinoPfos.front());
        cEventId++;
    }

    /*
    int nMCParticles(0);
    FloatVector MCParticleEnergies;

    int nClusters(0);
    FloatVector clusterHadronicEnergies;
    FloatVector clusterElectromagneticEnergies;

    int nParticleFlowObjects(0);
    FloatVector parcicleFlowObjectEnergies;

    // Try writing a value to the root tree
    PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "testVariable", 6258465));

    const MCParticleList *pMCParticleList(nullptr);
    const ClusterList *pClusterList(nullptr);
    const PfoList *pPfoList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pMCParticleList));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pPfoList));

    PANDORA_MONITORING_API(SetEveDisplayParameters(this->GetPandora(), true, DETECTOR_VIEW_XZ, -1.f, -1.f, 1.f));
    PANDORA_MONITORING_API(VisualizeMCParticles(this->GetPandora(), pMCParticleList, "MyMCParticles", BLUE));
    PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));

    for (const MCParticle *const pMCParticle : *pMCParticleList)
    {
        MCParticleEnergies.push_back(pMCParticle->GetEnergy());
        nMCParticles++;
    }

    for (const Cluster *const pCluster : *pClusterList)
    {
        clusterHadronicEnergies.push_back(pCluster->GetHadronicEnergy());
        clusterElectromagneticEnergies.push_back(pCluster->GetElectromagneticEnergy());
        nClusters++;
    }

    for (const ParticleFlowObject *const pParticleFlowObject : *pPfoList)
    {
        parcicleFlowObjectEnergies.push_back(pParticleFlowObject->GetEnergy());
        nParticleFlowObjects++;
    }

    // Try writing values to the root tree
    PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "testVariable", 62584652));
    PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName, "nMCParticles", nMCParticles));
    PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName, "MCParticleEnergies", &MCParticleEnergies));
    PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName, "nClusters", nClusters));
    PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName, "clusterHadronicEnergies", &clusterHadronicEnergies));
    PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName, "clusterElectromagneticEnergies", &clusterElectromagneticEnergies));
    PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName, "nParticleFlowObjects", nParticleFlowObjects));
    PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName, "particleFlowObjectEnergies", &parcicleFlowObjectEnergies));

    // Fill the root tree
    PANDORA_MONITORING_API(FillTree(this->GetPandora(), m_treeName.c_str()));
    */

    return STATUS_CODE_SUCCESS;
}

int MyTrackShowerIdAlgorithm::WritePfo(int pfoId, int parentPfoId, const ParticleFlowObject *const pPfo)
{
    IntVector daughterPfoIds;	// daughterPfoIds = [empty vector of integers].
    int pfosWritten(0);		// pfosWritten = 0.
    for (const ParticleFlowObject *const daughterPfo : pPfo->GetDaughterPfoList()) // for each daughterPfo in pPfo.daughterPfos:
    {
        int daughterPfoId(pfoId + pfosWritten + 1); // daughterPfoId = pfoId + pfosWritten + 1
        daughterPfoIds.push_back(daughterPfoId); // put daughterPfoId into daughterPfoIds
        pfosWritten += WritePfo(daughterPfoId, pfoId, daughterPfo); // pfosWritten += WritePfo(daughterPfoId, pfoId, daughterPfo)
    }
    std::cout << "MyTrackShowerIdAlgorithm: Writing PFO to the tree!" << std::endl;
    std::cout << "MyTrackShowerIdAlgorithm: The parent PFO ID is " << parentPfoId << std::endl;
    // Write parentPfoId to ROOT tree
    std::cout << "MyTrackShowerIdAlgorithm: The PFO ID is " << pfoId << std::endl;
    // Write pfoId to ROOT tree
    std::cout << "MyTrackShowerIdAlgorithm: The daughter PFO IDs are ";
    for (int daughterPfoId : daughterPfoIds)
    {
        std::cout << daughterPfoId << " ";
    }
    std::cout << std::endl;

    // Write daughterPfoIds to ROOT tree
/*
Write all other properties of pPFO to ROOT tree
Fill the tree.
    */
    pfosWritten += 1;
    return pfosWritten;	// return pfosWritten += 1.
}

MyTrackShowerIdAlgorithm::MyTrackShowerIdAlgorithm() :
    cEventId(0),
    cPfoId(0)
{
}

MyTrackShowerIdAlgorithm::~MyTrackShowerIdAlgorithm()
{
    // Save the root tree
    std::cout << "MyTrackShowerIdAlgorithm: Saving ROOT tree " << m_treeName << " to file " << m_fileName << std::endl;
    try
    {
        m_pTFile->Write();
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
        m_pPfoTree->Branch("EventId", &cEventId,"EventId/I");
        m_pPfoTree->Branch("PfoId", &cPfoId,"PfoId/I");
    }
    else
    {
       // To do: set cEventId such that it is equal to next unused event number (won't be zero when events are already in the file)
    }

    return STATUS_CODE_SUCCESS;
}
