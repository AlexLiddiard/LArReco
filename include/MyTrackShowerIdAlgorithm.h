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

    int WritePfo(const pandora::ParticleFlowObject *const pPfoint, int pfoId = 0, int parentPfoId = -1, int hierarchyTier = 0);

    // Member variables here
    std::string				m_treeName; 		///< Name of output tree
    std::string				m_fileName; 		///< Name of output file
    TFile				*m_pTFile;		///< ROOT tree file
    TTree				*m_pPfoTree;		///< PFO tree
    int					cEventId;		///< Current event id
    int					cParentPfoId;		///< Parent pfo id
    int					cPfoId;			///< Current pfo id
    int					cHierarchyTier;		///< Hierarchy tier of the pfo
    pandora::IntVector			*m_pDaughterPfoIds;	///< Daughter pfo ids
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *MyTrackShowerIdAlgorithm::Factory::CreateAlgorithm() const
{
    return new MyTrackShowerIdAlgorithm();
}

#endif // #ifndef MY_TRACK_SHOWER_ID_ALGORITHM_H
