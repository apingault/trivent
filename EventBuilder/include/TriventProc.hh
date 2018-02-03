#ifndef TRIVENTPROC_HH
#define TRIVENTPROC_HH

// -- Asics and channels mapping for sdhcal
#include "Mapping.hh"

// -- std includes
#include <map>
#include <string>

// -- marlin includes
#include "marlin/VerbosityLevels.h"
#include "marlin/tinyxml.h"
#include <marlin/Processor.h>

// -- lcio includes
#include "IO/LCWriter.h"
#include <EVENT/LCCollection.h>
#include <EVENT/RawCalorimeterHit.h>
#include <IMPL/CalorimeterHitImpl.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/LCEventImpl.h>
#include <UTIL/CellIDEncoder.h>
#include <lcio.h>

// -- ROOT includes
#include <TFile.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TTree.h>

class TriventProc : public marlin::Processor {
public:
  Processor *newProcessor() override { return new TriventProc; }

  TriventProc();
  TriventProc(const TriventProc & /*proc*/)  = delete;
  TriventProc(const TriventProc && /*proc*/) = delete;
  ~TriventProc() override                    = default;

  TriventProc &operator=(const TriventProc & /*proc*/) = delete;
  TriventProc &operator=(const TriventProc && /*proc*/) = delete;

  void init() override;
  void initRootTree();
  void processEvent(LCEvent *evtP) override;
  void processRunHeader(LCRunHeader * /*runH*/) override{};
  void XMLReader(const std::string &xmlfile);
  void printDifGeom() const;

  int getCellDif_id(const int &cellId) const;
  int getCellAsic_id(const int &cellId) const;
  int getCellChan_id(const int &cellId) const;

  int getMaxTime() const;

  std::vector<int> getTimeSpectrum(const int &maxTime) const;

  /**
   * @brief
   *
   * @param inputLCCol
   */
  void fillRawHitTrigger(const LCCollection &inputLCCol);

  /**
   * @brief Return **inclusive** time boundaries for eventBuilding
   *
   * @param beginTime
   * @param endTime
   * @param candidateTime
   * @return std::vector<const std::vector<int>::iterator>
   */
  std::vector<const std::vector<int>::iterator>
  getCandidateTimeBoundaries(const std::vector<int>::iterator &beginTime, const std::vector<int>::iterator &endTime,
                             const std::vector<int>::iterator &candidateTime) const;

  bool checkPadLimits(const std::vector<int> &padIndex, const std::vector<int> &padLimits) const;

  /**
   * @brief return I, J, K for hit in given difId, asicId, chanId
   * return empty vector if dif not found in geometry
   *
   * @param difId
   * @param asicId
   * @param chanId
   * @return std::vector<int>
   */
  std::vector<int> getPadIndex(const int &difId, const int &asicId, const int &chanId) const;

  /**
   * @brief
   *
   */
  void resetEventParameters();

  /**
   * @brief
   *
   */
  void resetTriggerParameters();

  void eventBuilder(std::unique_ptr<IMPL::LCCollectionVec> &evtCol, const int &timePeak, const int &lowTimeBoundary,
                    const int &highTimeBoundary);

  void end() override;

  TH2 *makeTH2(const std::string &title, const std::string &xTitle, const std::string &yTitle) const;

  // std::unique_ptr<TTree> getOrCreateTree(const std::string &treeName, const std::string &treeDescription);
  TTree *getOrCreateTree(const std::string &treeName, const std::string &treeDescription) const;
  void   findCerenkovHits(std::unique_ptr<IMPL::LCCollectionVec> &cerCol, const int &timePeak);
  int    getAsicKey(const std::vector<int> &padIndex) const;
  int    IJKToKey(const std::vector<int> &padIndex) const;

private:
  std::unique_ptr<LCWriter> m_lcWriter;
  // map of <hitTimeStamp, rawHit>
  std::map<int, std::vector<EVENT::RawCalorimeterHit *>> m_triggerRawHitMap;
  std::map<int, std::vector<EVENT::RawCalorimeterHit *>> m_cerenkovRawHitMap;
  // std::vector<std::shared_ptr<EVENT::RawCalorimeterHit>> m_trigger_raw_hit;
  // std::vector<std::shared_ptr<EVENT::RawCalorimeterHit>> m_cerenkov_raw_hit;

  std::string              m_outputCollectionName;
  std::string              m_outFileName;
  std::string              m_noiseFileName;
  std::string              m_rootFileName;
  std::string              m_treeName;
  std::string              m_treeDescription;
  std::string              m_geomXMLFile;
  std::vector<std::string> m_hcalCollections;
  float                    m_beamEnergy{0};
  std::map<int, LayerID>   m_mDifMapping;

  // Cut parameters
  int m_elecNoiseCut{5000};
  int m_noiseCut{10};
  int m_layerCut{10};
  int m_timeWin{2};
  int m_time2prevEventCut{0};

  // Geometry paramaters
  float          m_cellSizeI{10.408};
  float          m_cellSizeJ{10.408};
  float          m_layerThickness{26.131};
  std::set<uint> m_layerSet{};

  // Cerenkov
  std::string      m_cerenkovCollectionName;
  bool             m_hasCherenkov{true};
  int              m_cerenkovDifId{3};
  int              m_cerenkovLayerId{};
  int              m_cerenkovTimeWindow{10};
  std::vector<int> m_cerAsic;
  std::vector<int> m_cerChan;
  std::vector<int> m_cerThreshold;
  unsigned int     m_nCerenkov1{0};             // Number of hit in first Cerenkov
  unsigned int     m_nCerenkov2{0};             // Number of hit in second Cerenkov
  unsigned int     m_nCerenkov3{0};             // Number of hit in first + second Cerenkov
  unsigned int     m_nCerenkovTrigger{0};       // Tot number of hit in cerenkov for current trigger
  bool             m_hasTooManyCerenkov{false}; // if m_nCerenkovTrigger > bifHit in trigger
  std::vector<int> m_timeCerenkov;              // Timing between peak and Cerenkov signal
  unsigned int     m_nCerenkovEvts{0};          // Number of events tagged with the cerenkov

  unsigned int m_trigNbr{0};
  unsigned int m_trigCount{0};
  unsigned int m_evtNum{0};
  unsigned int m_selectedNum{0};
  unsigned int m_rejectedNum{0};
  int          m_bcid1{0};
  int          m_bcid2{0};

  // Color for streamlog output
  std::string normal{0x1b, '[', '0', ';', '3', '9', 'm', 0};
  std::string red{0x1b, '[', '1', ';', '3', '1', 'm', 0};
  std::string green{0x1b, '[', '1', ';', '3', '2', 'm', 0};
  std::string yellow{0x1b, '[', '1', ';', '3', '3', 'm', 0};
  std::string blue{0x1b, '[', '1', ';', '3', '4', 'm', 0};
  std::string magenta{0x1b, '[', '1', ';', '3', '5', 'm', 0};
  std::string white{0x1b, '[', '1', ';', '3', '9', 'm', 0};

  // ROOT histograms
  // std::unique_ptr<TFile> m_rootFile;
  TFile *m_rootFile{};
  // std::vector<std::unique_ptr<TH2>> m_vHitMapPerLayer; // HitMap of selected evt for each Layer
  std::vector<TH2 *> m_vHitMapPerLayer; // HitMap of selected evt for each Layer
  unsigned int       m_runNumber{0};
  std::string        m_plotFolder;

  // Trees
  // TTree *m_triggerTree;
  // std::unique_ptr<TTree> m_triggerTree;
  TTree *m_eventTree{};
  // std::unique_ptr<TTree> m_eventTree;

  // Trigger branches
  // unsigned int m_trigNbr;              // Current trigger number
  // int m_nEvt;                       // Number of evt in trigger
  // std::vector<unsigned int> m_vTimeSpectrum; // number of hits per time clock

  // Event branches
  unsigned int m_evtTrigNbr{0}; // Current trigger Number
  unsigned int m_evtNbr{0};     // Current Evt number
  unsigned int m_nHit{0};       // Number of hits

  // std::vector<unsigned long int> m_hitBCID;             // Hit time
  std::vector<int> m_hitI; // Hit position
  std::vector<int> m_hitJ; // Hit position
  std::vector<int> m_hitK; // Hit position
  std::vector<int> m_hitThreshold;

  std::set<unsigned int> m_firedLayersSet;            // set of Layers touched in evt
  unsigned int           m_nFiredLayers{0};           // Number of Layers touched in evt = m_firedLayersSet.size()
  bool                   m_isSelected{true};          // Event is selected/rejected
  bool                   m_isNoise{false};            // If rejected, is it noise
  bool                   m_hasNotEnoughLayers{false}; // If rejected, has not touched sufficient layers
  bool                   m_hasFullAsic{false};        // If rejected, has full asics
  bool                   m_hasRamFull{false};         // If rejected, has ram full
  bool                   m_keepRejected{false};       // Wether or not to keep rejected data in the output
};

#endif
