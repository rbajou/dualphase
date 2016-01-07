// -*- mode: c++; c-basic-offset: 2; -*-
////////////////////////////////////////////////////////////////////////
// Class:       SSPToOffline
// Module Type: producer
// File:        SSPToOffline_module.cc
//
// Repackage raw SSP data into OpDetPulse data products
//
// Alex Himmel ahimme@phy.duke.edu
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <iostream>
#include <map>

//lbne-artdaq includes
#include "lbne-raw-data/Overlays/SSPFragment.hh"
#include "lbne-raw-data/Overlays/anlTypes.hh"
#include "artdaq-core/Data/Fragments.hh"

//larsoft includes
#include "lardata/RawData/raw.h"
#include "lardata/RawData/OpDetWaveform.h"
#include "larcore/Geometry/Geometry.h"

//daqinput35t includes

#include "utilities/UnpackFragment.h"
#include "SSPFragmentToOpDetWaveform.h"

namespace DAQToOffline {
  class SSPToOffline;
}

class DAQToOffline::SSPToOffline : public art::EDProducer {
public:
  explicit SSPToOffline(fhicl::ParameterSet const & pset);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  SSPToOffline(SSPToOffline const &) = delete;
  SSPToOffline(SSPToOffline &&) = delete;
  SSPToOffline & operator = (SSPToOffline const &) = delete;
  SSPToOffline & operator = (SSPToOffline &&) = delete;
  void produce(art::Event & evt) override;
  void reconfigure(fhicl::ParameterSet const& pset);
  void printParameterSet();

private:

  std::string fFragType;
  std::string fRawDataLabel;
  std::string fOutputDataLabel;
  double      fNOvAClockFrequency; //MHz
  std::string fChannelMapFile;
  
  std::map<int,int> theChannelMap;
    
  //long        first_FirstSample;
  //double      first_TimeStamp;
  //long        first_InternalSample;
  //double      first_InternalTimeStamp;

  //bool fDebug;
  //raw::Compress_t        fCompression;      ///< compression type to use
  //unsigned int           fZeroThreshold;    ///< Zero suppression threshold


};


DAQToOffline::SSPToOffline::SSPToOffline(fhicl::ParameterSet const & pset)
{

  this->reconfigure(pset);

  produces< std::vector<raw::OpDetWaveform> > (fOutputDataLabel);  

  //first_FirstSample = -1;
  //first_TimeStamp = -1;
}

void DAQToOffline::SSPToOffline::reconfigure(fhicl::ParameterSet const& pset){

  fFragType           = pset.get<std::string>("FragType");
  fRawDataLabel       = pset.get<std::string>("RawDataLabel");
  fOutputDataLabel    = pset.get<std::string>("OutputDataLabel");
  fNOvAClockFrequency = pset.get<double>("NOvAClockFrequency"); // in MHz
  fChannelMapFile     = pset.get<std::string>("OpDetChannelMapFile");

  //fDebug = pset.get<bool>("Debug");
  //fZeroThreshold=0;
  //fCompression=raw::kNone;

  printParameterSet();
  BuildOpDetChannelMap(fChannelMapFile, theChannelMap);
  
}

void DAQToOffline::SSPToOffline::printParameterSet(){

  mf::LogDebug("SSPToOffline") << "===================================="   << "\n"
			       << "Parameter Set"                          << "\n"
			       << "===================================="   << "\n"
			       << "fFragType:        " << fFragType        << "\n"
			       << "fRawDataLabel:    " << fRawDataLabel    << "\n"
			       << "fOutputDataLabel: " << fOutputDataLabel << "\n"
			       << "fChannelMapFile:  " << fChannelMapFile  << "\n"
			       << "===================================="   << "\n";
}


void DAQToOffline::SSPToOffline::produce(art::Event & evt)
{

  art::Handle<artdaq::Fragments> rawFragments;
  evt.getByLabel(fRawDataLabel, fFragType, rawFragments);

  // Check if there is SSP data in this event
  // Don't crash code if not present, just don't save anything
  try { rawFragments->size(); }
  catch(std::exception e) {
    mf::LogWarning("SSPToOffline") << "WARNING: Raw SSP data not found in event " << evt.event();
    std::vector<raw::OpDetWaveform> waveforms;
    evt.put(std::make_unique<std::vector<raw::OpDetWaveform>>(std::move(waveforms)), fOutputDataLabel);
    return;
  }

  // Check that the data is valid
  if(!rawFragments.isValid()){
    mf::LogError("SSPToOffline") << "Run: " << evt.run()
				 << ", SubRun: " << evt.subRun()
				 << ", Event: " << evt.event()
				 << " is NOT VALID";
    throw cet::exception("raw NOT VALID");
    return;
  }

  auto waveforms = SSPFragmentToOpDetWaveform(*rawFragments, fNOvAClockFrequency, theChannelMap);

  evt.put(std::make_unique<decltype(waveforms)>(std::move(waveforms)), fOutputDataLabel);


}

DEFINE_ART_MODULE(DAQToOffline::SSPToOffline)


