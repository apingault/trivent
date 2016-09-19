#ifndef _TriventProc_hh_
#define _TriventProc_hh_

// -- Asics and channels mapping for sdhcal
#include "Mapping.h"

// -- std includes
#include <string>
#include <map>

// -- marlin includes
#include <marlin/Processor.h>
#include "marlin/VerbosityLevels.h"
#include "marlin/tinyxml.h"

// -- lcio includes
#include <lcio.h>
#include "IO/LCWriter.h"
#include <EVENT/RawCalorimeterHit.h>
#include <EVENT/LCCollection.h>
#include <IMPL/CalorimeterHitImpl.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/LCEventImpl.h>
#include <UTIL/CellIDEncoder.h>

// -- ROOT includes
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2D.h>

class TriventProc  : public marlin::Processor
{
public:

  Processor*  newProcessor() { return new TriventProc ; }

  TriventProc();

  ~TriventProc() {};

  void init();


  void    processEvent( LCEvent * evtP );
  void    processRunHeader( LCRunHeader * runH);// added by me
  void    XMLReader(std::string xmlfile);
  void    readDifGeomFile(std::string geomfile);
  void    printDifGeom();
  void    defineColors();

  uint    getCellDif_id(int cell_id);
  uint    getCellAsic_id(int cell_id);
  uint    getCellChan_id(int cell_id);

  void    getMaxTime();
  std::vector<int> getTimeSpectrum();
  std::vector<unsigned int> getPadIndex(uint dif_id, uint asic_id, uint chan_id);
  void    eventBuilder(LCCollection* col_event, int time_peak, int prev_time_peak);
  void    end();

  TTree*  getOrCreateTree(std::string treeName, std::string treeDescription); 
  void    findCerenkovHits(int timePeak);

protected:
  TH1F *noise_dist;
  TH1F *gain_chan;
  TH1F *mean_hit_dif;
  TH1F *time_hit_dif;
  // xml test
  std::map<std::string, std::string> m_parameters;

  std::vector<EVENT::RawCalorimeterHit*> _trigger_raw_hit;
  std::vector<EVENT::RawCalorimeterHit*> _cerenkov_raw_hit;

  bool GAIN_CORRECTION_MODE;
  std::string _outFileName;
  std::string _noiseFileName;
  std::string _treeName;
  std::string _rootFileName;
  std::string m_outputCollectionName;
  std::string _fileName;
  std::string _mappingfile;
  std::string _geomXML;
  std::vector<std::string> _hcalCollections;
  int _overwrite;
  TTree *_outputTree;
  unsigned int _trigNbr;
  Int_t _nHit;
  Int_t _elecNoiseCut;

  std::map<int, LayerID  > _mapping;
  std::map<int, double  > _chamberPos;//camber , pos

  // Cut parameters
  int _noiseCut;
  int _timeWin;
  int _layerCut;
  int _time2prevEventCut;
  double _layerGap;
  float _beamEnergy;
  int _trigCount;
  int _maxTime;
  int evtnum;
  int _selectedNum;
  int _rejectedNum;
  
  // Flags for selected/rejected events
  
  
  std::set<unsigned int> _firedLayersSet;
  LCWriter* _lcWriter;
  int _bcid1;
  int _bcid2;

  // Geometry paramaters
  float m_cellSizeI;
  float m_cellSizeJ;
  float m_layerThickness;

  // Cerenkov
  int m_cerenkovLength;
  bool m_hasCherenkov;
  int m_cerenkovDifId;
  int m_cerenkovTimeWindow;
  bool m_cerenkovFlag[3];
  int m_cerenkovCount[3];
  int m_cerenkovCountTotal[3];

  // Color for streamlog ouptut 
  std::string normal  ;
  std::string red     ;
  std::string green   ;
  std::string yellow  ;
  std::string blue    ;
  std::string magenta ;
  std::string white   ;

  bool m_isEvent;
  //ROOT histograms
  TFile *m_rootFile;
  std::vector<TH2D*> m_vHitMapPerLayer; // HitMap of selected evt for each Layer
  Int_t m_runNumber;
  std::string m_plotFolder;
  // Trees
  TTree *m_triggerTree; 
  TTree *m_eventTree; 
  
  // Trigger branches
  int m_trigNbr;                    // Current trigger number
  // int m_nEvt;                       // Number of evt in trigger
  std::vector<int> m_vTimeSpectrum; // number of hits per time clock
  
  // Event branches
  int m_evtTrigNbr;          // Current trigger Number
  int m_evtNbr;              // Current Evt number
  int m_nHit;                // Number of hits
  int m_nFiredLayers;        // Number of Layers touched in evt
  int m_nCerenkov1;          // Number of hit in first Cerenkov
  int m_nCerenkov2;          // Number of hit in second Cerenkov
  int m_timeCerenkov;        // Timing between peak and Cerenkov signal
  bool m_isSelected;         // Event is selected/rejected
  bool m_isNoise;            // If rejected, is it noise
  bool m_isTooCloseInTime;   // If rejected, is it too close from previous evt
  bool m_hasNotEnoughLayers; // If rejected, has not touched sufficient layers
  bool m_hasFullAsic;        // If rejected, has full asics

};



#endif


