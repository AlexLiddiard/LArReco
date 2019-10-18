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
        WritePfo(neutrinoPfos.front());
        cEventId++;
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

    cParentPfoId = parentPfoId; // Write parentPfoId to ROOT tree
    cPfoId = pfoId; // Write pfoId to ROOT tree
    m_pDaughterPfoIds = &daughterPfoIds; // Write daughterPfoIds to ROOT tree
    cHierarchyTier = hierarchyTier; // Write hierarchyTier to ROOT tree
/*
Write all other properties of pPFO to ROOT tree
*/
    m_pPfoTree->Fill(); // Fill the tree.
    pfosWritten += 1;
    return pfosWritten;	// return pfosWritten += 1.
}

MyTrackShowerIdAlgorithm::MyTrackShowerIdAlgorithm() :
    cEventId(0)
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
        m_pPfoTree->Branch("EventId", &cEventId);
        m_pPfoTree->Branch("PfoId", &cPfoId);
	m_pPfoTree->Branch("ParentPfoId", &cParentPfoId);
        m_pPfoTree->Branch("DaughterPfoIds", &m_pDaughterPfoIds);
        m_pPfoTree->Branch("HierarchyTier",  &cHierarchyTier);
    }
    else
    {
       // TODO: Set cEventId such that it is equal to next unused event number (won't be zero when events are already in the file)
	
       std::cout <<  "MyTrackShowerIdAlgorithm: Found an existing tree, already containing " << cEventId << " events." << std::endl;
       m_pPfoTree->SetBranchAddress("EventId", &cEventId);
       m_pPfoTree->SetBranchAddress("PfoId", &cPfoId);
       m_pPfoTree->SetBranchAddress("ParentPfoId", &cParentPfoId);
       m_pPfoTree->SetBranchAddress("DaughterPfoIds", &m_pDaughterPfoIds);
       m_pPfoTree->SetBranchAddress("HierarchyTier",  &cHierarchyTier);
    }

    return STATUS_CODE_SUCCESS;
}
