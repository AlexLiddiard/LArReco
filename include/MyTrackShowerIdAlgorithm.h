/**
 *  @file   LArReco/include/MyTrackShowerIdAlgorithm.h
 * 
 *  @brief  Header file for my track shower ID algorithm.
 * 
 *  $Log: $
 */
#ifndef MY_TRACK_SHOWER_ID_ALGORITHM_H
#define MY_TRACK_SHOWER_ID_ALGORITHM_H 1

#include "Pandora/Algorithm.h"
#include "TFile.h"
#include "TTree.h"


struct PlaneCaloHits 
{
    int					nHits;				///< Number of calo hits
    pandora::FloatVector		*pDriftCoord;			///< hit x
    pandora::FloatVector		*pWireCoord;			///< hit u/v/w
    pandora::FloatVector		*pElectromagneticEnergy;	///< hit electromagnetic energy
    pandora::FloatVector		*pHadronicEnergy;		///< hit hadronic energy
};

/**
 *  @brief  MyTrackShowerIdAlgorithm class
 */
class MyTrackShowerIdAlgorithm : public pandora::Algorithm
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

    int WritePfo(const pandora::ParticleFlowObject *const pPfo, int pfoId = 0, int parentPfoId = -1, int hierarchyTier = 0);
    void GetCaloHitInfo(const pandora::ParticleFlowObject *const pPfo, const pandora::HitType &hitType, PlaneCaloHits *planeCaloHits);

    // Member variables here
    std::string				m_treeName; 		///< Name of output tree
    std::string				m_fileName; 		///< Name of output file
    TFile				*m_pTFile;		///< ROOT tree file
    TTree				*m_pPfoTree;		///< PFO tree
    
    // PFO tree variables
    int					m_EventId;		///< Current event id
    int					m_HierarchyTier;	///< PFO hierarchy tier
    int					m_PfoId;		///< Current PFO id
    int					m_ParentPfoId;		///< Parent PFO id
    pandora::IntVector			*m_pDaughterPfoIds;	///< Daughter PFO ids
    float				m_Vertex[3];		///< PFO interaction vertex

    PlaneCaloHits			m_UCaloHits;		///< U plane calo hits
    PlaneCaloHits			m_VCaloHits;		///< V plane calo hits
    PlaneCaloHits			m_WCaloHits;		///< W plane calo hits
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *MyTrackShowerIdAlgorithm::Factory::CreateAlgorithm() const
{
    return new MyTrackShowerIdAlgorithm();
}

#endif // #ifndef MY_TRACK_SHOWER_ID_ALGORITHM_H
