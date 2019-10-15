/**
 *  @file   LArReco/src/MyTrackShowerIdAlgorithm.cc
 * 
 *  @brief  Implementation of my track shower ID algorithm class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"
#include "MyTrackShowerIdAlgorithm.h"

using namespace pandora;

StatusCode MyTrackShowerIdAlgorithm::Run()
{
    // Algorithm code here
    std::cout <<  "MyTrackShowerIdAlgorithm: Hello World!" << std::endl;

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

    return STATUS_CODE_SUCCESS;
}

MyTrackShowerIdAlgorithm::~MyTrackShowerIdAlgorithm()
{
    // Save the root tree
    std::cout << "MyTrackShowerIdAlgorithm: Saving ROOT tree " << m_treeName << " to file " << m_fileName << std::endl;
    try
    {
        PANDORA_MONITORING_API(SaveTree(this->GetPandora(), m_treeName, m_fileName, "UPDATE"));
    }
    catch (const StatusCodeException &)
    {
        std::cout << "MyTrackShowerIdAlgorithm: Unable to write tree!" << std::endl;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode MyTrackShowerIdAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    // Read settings from xml file here
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "OutputTree", m_treeName));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "OutputFile", m_fileName));
    return STATUS_CODE_SUCCESS;
}
