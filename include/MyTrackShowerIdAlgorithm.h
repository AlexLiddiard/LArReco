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


struct ViewHits
{
    pandora::FloatVector		*pXCoord;			///< hit x coord (drift coord)
    pandora::FloatVector		*pYCoord;			///< hit y coord
    pandora::FloatVector		*pZCoord;			///< hit z (for U/V/W view, equivalent to the wire plane coord)
    pandora::FloatVector		*pXCoordError;			///< hit x coord error
    pandora::FloatVector		*pEnergy;			///< hit electromagnetic energy
    int					mcPdgCode;			///< truth particle for these hits
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
    void GetCaloHitInfo(const pandora::ParticleFlowObject *const pPfo, pandora::HitType hitType, ViewHits *vHits);

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
    ViewHits				m_UViewHits;		///< U view calo hits
    ViewHits				m_VViewHits;		///< V view calo hits
    ViewHits				m_WViewHits;		///< W view calo hits
    ViewHits				m_ThreeDViewHits;	///< 3D view calo hits
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *MyTrackShowerIdAlgorithm::Factory::CreateAlgorithm() const
{
    return new MyTrackShowerIdAlgorithm();
}

#endif // #ifndef MY_TRACK_SHOWER_ID_ALGORITHM_H
