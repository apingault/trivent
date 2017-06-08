/**
 * Yacine HADDAD
 * LLR Ecole polytechnique
 * avril 2012
 * Trivent v0.3
 */

#include <NoiseProc.hh>
#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/LCEventImpl.h>
#include <limits.h>
#include <cmath>
#include <EVENT/LCFloatVec.h>
#include <EVENT/LCParameters.h>
#include <stdexcept>  
#include <Rtypes.h> 
#include <sstream>		
#include <UTIL/CellIDEncoder.h>
#include "Mapping.h"
#include "TObject.h"
#include "TRefArray.h"
#include "TRef.h"
#include <fstream>
#include <algorithm>
#include <lcio.h>
#include "marlin/VerbosityLevels.h"
#include "marlin/tinyxml.h"

NoiseProc a_NoiseProc_instance;

//=========================================================
NoiseProc::NoiseProc()
  : Processor("NoiseProc"),
    _output(0),
    _outputTree(0)//,
			//_hitArray(0)
{
  
  streamlog_out( MESSAGE )<< "Trivent ... begin " << endl;
  _rejectedNum = 0;
  
  // collection 
  std::vector<std::string> hcalCollections;    
  hcalCollections.push_back(std::string("DHCALRawHits"));
  registerInputCollections( LCIO::RAWCALORIMETERHIT , 
			    "HCALCollections"       ,  
			    "HCAL Collection Names" ,  
			    _hcalCollections        , 
			    hcalCollections         ); 
  
  //maping on XML file
  _geomXML = "setup_geometry.xml";
  registerProcessorParameter("setup_geometry" ,
                             "Dif geometry and position on the detector XML",
                             _geomXML,
                             _geomXML);
  
  //maping on txt file
  _mappingfile = "mapping_ps.txt";
  registerProcessorParameter("DIFMapping" ,
                             "dif's mapping file ",
                             _mappingfile,
                             _mappingfile);
  // electronic noise cut
  _elec_noise_cut = 5000;
  registerProcessorParameter("electronic_noise_cut" ,
                             "number of hit max on time stamp",
                             _elec_noise_cut,
                             _elec_noise_cut);  
} 

void NoiseProc::XMLReader(std::string xmlfile){
  TiXmlDocument doc(xmlfile.c_str());
  bool load_key = doc.LoadFile();  
  if(load_key){
    streamlog_out( MESSAGE ) << green << "File : " << xmlfile.c_str() << normal <<std::endl;
    // tout ici 
    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);
    // name block
    {
      pElem=hDoc.FirstChildElement().Element();
      // should always have a valid root but handle gracefully if it does
      if (!pElem) streamlog_out( WARNING ) << red << "error elem" << normal << std::endl;
      streamlog_out( MESSAGE ) << green << pElem->Value() << normal << std::endl;
      
      // save this for later
      hRoot=TiXmlHandle(pElem);
    }
    // parameters block
    {
      m_parameters.clear();
      pElem=hRoot.FirstChild("parameter").Element();
      std::string key = pElem->Attribute("name");
      streamlog_out( MESSAGE ) << green << key.c_str() << normal << std::endl; 
      streamlog_out( DEBUG1 ) << green
			      <<"parameter : " 
			      << pElem->Attribute("name") 
			      << normal 
			      << std::endl;
    
      std::vector<std::string> lines;
      {
	std::string value = pElem->GetText() ;
	std::vector<std::string> lines;
	istringstream iss(value);
	copy(istream_iterator<string>(iss),
             istream_iterator<string>(),
             back_inserter<vector<string> >(lines));
	for(unsigned int iline = 0; iline < lines.size(); iline++){
	  std::string line = lines.at(iline);
	  streamlog_out( MESSAGE ) << red << line << normal << std::endl;
	  
	  stringstream ss( line.c_str() );
	  vector<string> result;
	  
	  LayerID mapp;
	  int Dif_id;
	  while( ss.good() )
	    {
	      string substr;
	      getline( ss, substr, ',' );
	      result.push_back( substr );
	    }
	  istringstream ( result.at(0) ) >> Dif_id;
	  istringstream ( result.at(1) ) >> mapp.K;
	  istringstream ( result.at(2) ) >> mapp.DifX;
	  istringstream ( result.at(3) ) >> mapp.DifY;
	  istringstream ( result.at(4) ) >> mapp.IncX;
	  istringstream ( result.at(5) ) >> mapp.IncY;
	  _mapping[Dif_id] = mapp;
	}
      }
      pElem = pElem->NextSiblingElement();
      // ChamberGeom  Node.
      {
	streamlog_out( DEBUG1 ) << green
				<<"parameter : " 
				<< pElem->Attribute("name") 
				<< normal 
				<< std::endl;
	std::vector<std::string> lines;
	{
	  std::string value = pElem->GetText() ;
	  std::vector<std::string> lines;
	  istringstream iss(value);
	  copy(istream_iterator<string>(iss),
	       istream_iterator<string>(),
	       back_inserter<vector<string> >(lines));
	  for(unsigned int iline = 0; iline < lines.size(); iline++){
	    std::string line = lines.at(iline);
	    streamlog_out( MESSAGE ) << red << line << normal << std::endl;
	    
	    stringstream ss( line.c_str() );
	    vector<string> result;
	    
	    double position;
	    int Dif_id;
	    while( ss.good() )
	      {
		string substr;
		getline( ss, substr, ',' );
		result.push_back( substr );
	      }
	    istringstream ( result.at(0) ) >> Dif_id;
	    istringstream ( result.at(3) ) >> position;
	    
	    _chamber_pos[Dif_id] = position;
	  }
	}
      }
    }
  }else{
    streamlog_out( WARNING ) << red << "Failed to load file : " << xmlfile.c_str() << normal <<std::endl;
  }
}

void NoiseProc::readDifGeomFile(std::string geomfile){
  
  cout << "read the mapping file .."<< endl;
  
  LayerID contenu;
  ifstream file(geomfile.c_str(), ios::in);
  if(file){ 
    while(!file.eof()){
      int Dif_id;
      char co;
      file >> Dif_id >> co 
	   >> contenu.K >> co 
	   >> contenu.DifX >> co 
	   >> contenu.DifY >> co  
	   >> contenu.IncX >> co  
	   >> contenu.IncY ;
      _mapping [Dif_id] = contenu;
    }
    file.close();
  }
  else
    cerr << "ERROR ... maping file not correct !" << endl;
}

void NoiseProc::printDifGeom(){
  
  for(std::map<int,LayerID>::iterator itt = _mapping.begin();itt!=_mapping.end();itt++)     {
    streamlog_out( MESSAGE ) << itt->first << "\t" << itt->second.K 
			     <<"\t"<<itt->second.DifX 
			     <<"\t"<<itt->second.DifY
			     <<"\t"<<itt->second.IncX
			     <<"\t"<<itt->second.IncY
			     << std::endl;
  }
}

// ============ decode the cell ids =============
uint NoiseProc::getCellDif_id(int cell_id){
  return cell_id & 0xFF;
}
uint NoiseProc::getCellAsic_id(int cell_id){
  return (cell_id & 0xFF00)>>8;
}
uint NoiseProc::getCellChan_id(int cell_id){
  return (cell_id & 0x3F0000)>>16;
}

uint* NoiseProc::getPadIndex(uint dif_id, uint asic_id, uint chan_id){
  _index[0]=_index[1]=_index[2]=0;
  double DifY = -1.,DifZ = -1.;
  DifZ = _mapping.find(dif_id)->second.K;
  DifY = _mapping.find(dif_id)->second.DifY;
  _index[0] = (1+MapILargeHR2[chan_id]+AsicShiftI[asic_id]);
  _index[1] = (32-(MapJLargeHR2[chan_id]+AsicShiftJ[asic_id]))+int(DifY);
  _index[2] = abs(int(DifZ));
  streamlog_out( DEBUG0 ) << " Dif_id == " << dif_id
			  << " Asic_id ==" << asic_id
			  << " Chan_id ==" << chan_id
			  << " I == " << _index[0]
			  << " J == " << _index[1]
			  << " K == " << _index[2]
			  << std::endl;
  return _index;
}

//===============================================
void NoiseProc::init() {
  trig_count = 0;
  //========================
  //readDifGeomFile(_mappingfile.c_str());
  
  // ========================
  
  printParameters();
  // new process
  
  char cnormal[8] =  {0x1b,'[','0',';','3','9','m',0};
  char cred[8]     = {0x1b,'[','1',';','3','1','m',0};
  char cgreen[8]   = {0x1b,'[','1',';','3','2','m',0};
  char cyellow[8]  = {0x1b,'[','1',';','3','3','m',0};
  char cblue[8]    = {0x1b,'[','1',';','3','4','m',0};
  char cmagenta[8] = {0x1b,'[','1',';','3','5','m',0};
  char cwhite[8]   = {0x1b,'[','1',';','3','9','m',0};

  normal   = cnormal;
  red      = cred;
  green    = cgreen;
  yellow   = cyellow;
  blue     = cblue;
  magenta  = cmagenta;
  white    = cwhite;

  //XMLReader(_geomXML.c_str());
  //printDifGeom();
  evtnum=0;// event number

  gainCorrection = new MyGainCorrection();
}
//==================================================================================
//void NoiseProc::setTriggerRawHit() {
  
//}
//==================================================================================
void NoiseProc::processRunHeader( LCRunHeader * runHd ) {
  
}

//==================================================================================
void NoiseProc::processEvent( LCEvent * evtP ) 
{	
  if (evtP != NULL){
    try{
      
      _eventNr=evtP->getEventNumber();
      for(unsigned int i=0; i< _hcalCollections.size(); i++){//!loop over collection
	try{
	  
	  LCCollection * col = NULL;
	  col = evtP ->getCollection(_hcalCollections[i].c_str());
	  int numElements = col->getNumberOfElements();// hit number 
	  
	  
	  streamlog_out( MESSAGE ) << yellow << "Trigger number == " << trig_count++ << normal << std::endl;
	  
	  if(col == NULL )  {
	    streamlog_out( WARNING )<< red << "TRIGGER SKIPED ..."<< normal <<std::endl;
	    break;
	  }
	  
	  if(numElements > _elec_noise_cut)  {
	    streamlog_out( MESSAGE ) << red << "TRIGGER SKIPED ..."<< normal <<std::endl;
	    break;
	  }
	  
	  // set raw hits 
	  _trigger_raw_hit.clear();
	  std::vector<int> vTrigger;
	  for (int ihit(0); ihit < col->getNumberOfElements(); ++ihit) {// loop over the hits
	    RawCalorimeterHit *raw_hit = 
	      dynamic_cast<RawCalorimeterHit*>( col->getElementAt(ihit)) ;
	    if (NULL != raw_hit){
	      _trigger_raw_hit.push_back(raw_hit);
	      //extract abolute bcid information: 
	      unsigned int difid = getCellDif_id(raw_hit->getCellID0());
	      unsigned int asicid = getCellAsic_id(raw_hit->getCellID0());
	      unsigned int chanid = getCellChan_id(raw_hit->getCellID0());
	      if( asicid>48 || chanid>64 )
		std::cout << "WHAT THE FUCK DIF_ID = " << difid << "\t ASIC_ID  = " << asicid<< "\t CHAN_ID = " << chanid << std::endl;
	      if(ihit==0){
		if (difid==0) return;
		std::stringstream pname("");
		pname <<"DIF"<<difid<<"_Triggers";
		col->getParameters().getIntVals(pname.str(),vTrigger);
		if (vTrigger.size()!=0){
		  _bcid1=vTrigger[4] ;
		  _bcid2=vTrigger[3] ;
		  unsigned long long Shift=16777216ULL;//to shift the value from the 24 first bits
		  unsigned long long theBCID_=_bcid1*Shift+_bcid2; 
		  streamlog_out( DEBUG1 ) << "trigger time : " << theBCID_ << std::endl;
		}
	      }
	      if(raw_hit->getTimeStamp()!=0){
		//std::cout << "1st : " 
		//	  << raw_hit->getCellID0() << "\t" 
		//	  << difid << "\t" 
		//	  << asicid << "\t" 
		//	  << chanid << std::endl;
		gainCorrection->fillHitsInMap(raw_hit);
	      }
	    }
	  }
	}catch (lcio::DataNotAvailableException zero) {}
      }
    }catch (lcio::DataNotAvailableException err) {}
  }
}	
//==============================================================
void NoiseProc::end()
{       
  streamlog_out( MESSAGE )<< "Trivent Select "<<_rejectedNum <<" Condidate event"<<std::endl;
  streamlog_out( MESSAGE )<< "Trivent end"<<std::endl;
  //cc.StoreHistos("test.root");
  //_lcWriter->close();
  
  gainCorrection->analyseNoise();
  gainCorrection->writeChannelGainCorrectionFile();
  delete gainCorrection;
}
//==============================================================


 
