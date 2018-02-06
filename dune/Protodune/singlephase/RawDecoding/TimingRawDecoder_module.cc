////////////////////////////////////////////////////////////////////////
// Class:       TimingRawDecoder
// Module Type: producer
// File:        TimingRawDecoder_module.cc
//
// Generated at Thu Jul  6 18:31:48 2017 by Antonino Sergi,32 1-A14,+41227678738, using artmod
// from cetpkgsupport v1_11_00.
////////////////////////////////////////////////////////////////////////

// art includes
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"

// artdaq and dune-raw-data includes
#include "dune-raw-data/Overlays/TimingFragment.hh"

// larsoft includes
#include "lardataobj/RawData/RawDigit.h"

// ROOT includes
#include "TH1.h"

// C++ Includes
#include <memory>

namespace dune {
  class TimingRawDecoder;
}

class dune::TimingRawDecoder : public art::EDProducer {
public:
  explicit TimingRawDecoder(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  TimingRawDecoder(TimingRawDecoder const &) = delete;
  TimingRawDecoder(TimingRawDecoder &&) = delete;
  TimingRawDecoder & operator = (TimingRawDecoder const &) = delete;
  TimingRawDecoder & operator = (TimingRawDecoder &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;
  void reconfigure(const fhicl::ParameterSet &pset);
  void printParameterSet();
  void beginJob();

  void setRootObjects();

private:

  // Declare member data here.
  std::string fRawDataLabel;
  std::string fOutputDataLabel;
  bool fUseChannelMap;
  bool fDebug;
  bool fMakeTree;
  raw::Compress_t        fCompression;      ///< compression type to use
  unsigned int           fZeroThreshold;    ///< Zero suppression threshold

  TH1I * fHTimestamp;
  TH1I * fHTrigType;
  TH1I * fHTimestampDelta;

  uint64_t fPrevTimestamp;
};


dune::TimingRawDecoder::TimingRawDecoder(fhicl::ParameterSet const & pset)
// :
// Initialize member data here.
{
  art::ServiceHandle<art::TFileService> fs;
  //fs->registerFileSwitchCallback(this, &TimingRawDecoder::setRootObjects);
  setRootObjects();

  reconfigure(pset);
  // Call appropriate produces<>() functions here.
  produces< std::vector<raw::RawDigit> > (fOutputDataLabel);  
}

void dune::TimingRawDecoder::reconfigure(fhicl::ParameterSet const& pset) {

  fRawDataLabel = pset.get<std::string>("RawDataLabel");
  fOutputDataLabel = pset.get<std::string>("OutputDataLabel");
  fUseChannelMap = pset.get<bool>("UseChannelMap");
  fDebug = pset.get<bool>("Debug");
  fMakeTree = pset.get<bool>("MakeTree");
  fZeroThreshold=0;
  fCompression=raw::kNone;
  fPrevTimestamp=0;
  if(fDebug) printParameterSet();

}

void dune::TimingRawDecoder::printParameterSet(){

  for(int i=0;i<20;i++) std::cout << "=";
  std::cout << std::endl;
  std::cout << "Parameter Set" << std::endl;
  for(int i=0;i<20;i++) std::cout << "=";
  std::cout << std::endl;

  std::cout << "fRawDataLabel: " << fRawDataLabel << std::endl;
  std::cout << "fOutputDataLabel: " << fOutputDataLabel << std::endl;
  std::cout << "fDebug: ";
  if(fDebug) std::cout << "true" << std::endl;
  else std::cout << "false" << std::endl;

  for(int i=0;i<20;i++) std::cout << "=";
  std::cout << std::endl;    
}

void dune::TimingRawDecoder::setRootObjects(){
  art::ServiceHandle<art::TFileService> tFileService;

  fHTimestamp = tFileService->make<TH1I>("Timestamp","Timing Timestamp",  1e3, 0, 1e9);
  fHTimestamp->GetXaxis()->SetTitle("Timestamp (ms)");

  fHTimestampDelta = tFileService->make<TH1I>("TimestampDelta","Timing Timestamp Delta", 100, 0, 1e3);
  fHTimestampDelta->GetXaxis()->SetTitle("Delta Timestamp (ms)");

  fHTrigType = tFileService->make<TH1I>("TrigType","Timing trigger type", 10, 0, 10);
  fHTrigType->GetXaxis()->SetTitle("Trigger type");


}
void dune::TimingRawDecoder::beginJob(){
}

void dune::TimingRawDecoder::produce(art::Event & evt){
  std::cout<<"-------------------- Timing RawDecoder -------------------"<<std::endl;
  // Implementation of required member function here.
  art::Handle<artdaq::Fragments> rawFragments;
  evt.getByLabel(fRawDataLabel, "TIMING", rawFragments);

  art::EventNumber_t eventNumber = evt.event();

  // Check if there is Timing data in this event
  // Don't crash code if not present, just don't save anything
  try { rawFragments->size(); }
  catch(std::exception e) {
    std::cout << " WARNING: Raw Timing data not found in event " << eventNumber << std::endl;
    std::vector<raw::RawDigit> digits;
    evt.put(std::make_unique<std::vector<raw::RawDigit>>(std::move(digits)), fOutputDataLabel);
    std::cout<<std::endl;
    return;
  }

  //Check that the data is valid
  if(!rawFragments.isValid()){
    std::cerr << "Run: " << evt.run()
	      << ", SubRun: " << evt.subRun()
	      << ", Event: " << eventNumber
	      << " is NOT VALID" << std::endl;
    throw cet::exception("rawFragments NOT VALID");
  }
  std::vector<raw::RawDigit> rawDigitVector;
  for(auto const& rawFrag : *rawFragments){
      dune::TimingFragment frag(rawFrag);
//      std::cout << "[TimingRawDecoder] Event number: " << eventNumber << ", TStamp: " << frag.get_tstamp() << std::endl; 
      std::cout << " ArtDaq Fragment Timestamp: "  << std::dec << rawFrag.timestamp() << std::endl;
      uint64_t currentTimestamp=frag.get_tstamp();
      fHTimestamp->Fill(currentTimestamp/1e6);
      fHTrigType->Fill(frag.get_scmd());

      if(fPrevTimestamp!=0) fHTimestampDelta->Fill((currentTimestamp-fPrevTimestamp)/1e6);

      fPrevTimestamp=currentTimestamp;
//      fHTimestamp->Fill(rawFrag.timestamp());
  }
  evt.put(std::make_unique<decltype(rawDigitVector)>(std::move(rawDigitVector)), fOutputDataLabel);
  std::cout<<std::endl;
}

DEFINE_ART_MODULE(dune::TimingRawDecoder)
