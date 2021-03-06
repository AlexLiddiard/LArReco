/**
 *  @file   LArReco/include/MyTrackShowerIdAlgorithm.h
 * 
 *  @brief  Header file for my track shower ID algorithm.
 * 
 *  $Log: $
 */
#ifndef MY_TRACK_SHOWER_ID_ALGORITHM_H
#define MY_TRACK_SHOWER_ID_ALGORITHM_H 1

#include "Pandora/ExternallyConfiguredAlgorithm.h"
#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"
#include "TFile.h"
#include "TTree.h"


struct ViewHits
{
    pandora::FloatVector		*pXCoord;			///< hit x coord (drift coord)
    pandora::FloatVector		*pYCoord;			///< hit y coord
    pandora::FloatVector		*pZCoord;			///< hit z (for U/V/W view, equivalent to the wire plane coord)
    pandora::FloatVector		*pXCoordError;			///< hit x coord error
    pandora::FloatVector		*pEnergy;			///< hit electromagnetic energy
    int					        nHitsPfo;			///< total number of pfo hits
    int					        nHitsMatch;			///< number of hits matched with (best matched) Monte Carlo particle
    int					        nHitsMcp;			///< total number of hits for the (best matched) Monte Carlo particle
};

/**
 *  @brief  MyTrackShowerIdAlgorithm class
 */
class MyTrackShowerIdAlgorithm : public pandora::ExternallyConfiguredAlgorithm
{
public:
    /**
     *  @brief  Factory class for instantiating algorithm
     */
    class Factory : public pandora::AlgorithmFactory
    {
    public:
        pandora::Algorithm *CreateAlgorithm() const;
    };

    /**
     *  @brief  Constructor
     */
    MyTrackShowerIdAlgorithm();
    /**
     *  @brief  Destructor
     */
    ~MyTrackShowerIdAlgorithm();

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    unsigned int WritePfo(const pandora::ParticleFlowObject *const pPfo, const unsigned int pfoId = 0, const int parentPfoId = -1, const unsigned int hierarchyTier = 0);
    void GetCaloHitInfo(const pandora::ParticleFlowObject *const pPfo, pandora::HitType hitType, ViewHits &viewHits);
    std::string GetFileName(const std::string& filePath);
    void GetIncidentMCPs(const pandora::MCParticleList *const pMCParticleList, pandora::MCParticleList &parentMCNuList);
    void Mapper(
        const lar_content::LArMCParticleHelper::MCContributionMap &basicMap, 
        const pandora::MCParticle *const pMCParticle, 
        const bool isShowerProduct, 
        const unsigned int hierarchyTier, 
        pandora::CaloHitList &caloHitsToMerge, 
        lar_content::LArMCParticleHelper::MCContributionMap &selectiveMap);
    void GetBestMatchedMCParticleInfo(const pandora::ParticleFlowObject *const pPfo, ViewHits &UView, ViewHits &VView, ViewHits &WView);
    void PrintMCParticles(const lar_content::LArMCParticleHelper::MCContributionMap &mcContributionMap, const unsigned int minHits = 1) const;
    void PrintMCParticle(const pandora::MCParticle *const pMCParticle, const lar_content::LArMCParticleHelper::MCContributionMap &mcToTrueHitListMap, const unsigned int depth = 0, const unsigned int minHits = 1, bool printDaughters = true) const;
    void GetPfoToHitsMap(const pandora::PfoList &pPfoList, lar_content::LArMCParticleHelper::PfoContributionMap &pfoToHitsMap);

    // Member variables here
    std::string     m_caloHitListName;          ///< Name of input calo hit list
    std::string     m_mcParticleListName;       ///< Name of input MC particle list
    std::string     m_pfoListName;              ///< Name of input PFO list
    unsigned int    m_mcMappingMinHits;         ///< Minimum number of hits for a MC particle to be mapped

    std::string		m_treeName; 		        ///< Name of output tree
    std::string		m_fileName; 		        ///< Name of output file
    TFile			*m_pTFile;                  ///< ROOT tree file
    TTree			*m_pPfoTree;                ///< PFO tree

    lar_content::LArMCParticleHelper::MCContributionMap m_selectiveMap;                     ///< Bespoke mapping of MCParticles to associated Calohits
    lar_content::LArMCParticleHelper::PfoToMCParticleHitSharingMap m_pfoToMCHitSharingMap;  ///< Mapping from PFOs to associated MCParticles and their shared hits
    const pandora::MCParticle *m_incidentMcp;

    // PFO tree variables
    unsigned int        m_EventId;              ///< Current event id
    unsigned int        m_HierarchyTier;        ///< PFO hierarchy tier
    unsigned int        m_PfoId;                ///< Current PFO id
    int                 m_ParentPfoId;          ///< Parent PFO id
    pandora::IntVector  *m_pDaughterPfoIds;     ///< Daughter PFO ids
    float               m_Vertex[3];            ///< PFO interaction vertex
    ViewHits            m_UViewHits;            ///< U view calo hits
    ViewHits            m_VViewHits;            ///< V view calo hits
    ViewHits            m_WViewHits;            ///< W view calo hits
    ViewHits            m_ThreeDViewHits;       ///< 3D view calo hits
    unsigned int        m_mcNuanceCode;         ///< Interaction type
    int                 m_mcPdgCode;            ///< truth particle for this PFO
    float               m_mcpMomentum;          ///< truth particle momentum
    unsigned int        m_mcHierarchyTier;      ///< truth particle hierarchy tier
    int                 m_mcParentPdgCode;      ///< truth parent particle for this PFO
    pandora::IntVector  *m_pMcDaughterPdgCodes; ///< truth daughter particles for this PFO
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *MyTrackShowerIdAlgorithm::Factory::CreateAlgorithm() const
{
    return new MyTrackShowerIdAlgorithm();
}

#endif // #ifndef MY_TRACK_SHOWER_ID_ALGORITHM_H
