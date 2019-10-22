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
    void GetMyCaloHits(const pandora::ParticleFlowObject *const pPfo, const pandora::HitType &hitType, std::vector<MyCaloHit>* myCaloHits);

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
    int					m_Vertex[3];		///< PFO interaction vertex

    // U plane CaloHits
    pandora::FloatVector		*m_pUHitx;		///< U plane hit x
    pandora::FloatVector		*m_pUHitu;		///< U plane hit u
    pandora::FloatVector		*m_pUHitEnergyE;	///< U plane hit electromagnetic energy
    pandora::FloatVector		*m_pUHitEnergyH;	///< U plane hit hadronic energy
    int					m_nUHits;		///< Number of U plane calo hits

    // V plane CaloHits
    pandora::FloatVector		*m_pVHitx;		///< V plane hit x
    pandora::FloatVector		*m_pVHitv;		///< V plane hit v
    pandora::FloatVector		*m_pVHitEnergyE;	///< V plane hit electromagnetic energy
    pandora::FloatVector		*m_pVHitEnergyH;	///< V plane hit hadronic energy
    int					m_nVHits;		///< Number of V plane calo hits

    // W plane CaloHits
    pandora::FloatVector		*m_pWHitx;		///< W plane hit x
    pandora::FloatVector		*m_pWHitw;		///< W plane hit v
    pandora::FloatVector		*m_pWHitEnergyE;	///< W plane hit electromagnetic energy
    pandora::FloatVector		*m_pWHitEnergyH;	///< W plane hit hadronic energy
    int					m_nWHits;		///< Number of W plane calo hits
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *MyTrackShowerIdAlgorithm::Factory::CreateAlgorithm() const
{
    return new MyTrackShowerIdAlgorithm();
}

#endif // #ifndef MY_TRACK_SHOWER_ID_ALGORITHM_H
