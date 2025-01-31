////////////////////////////////////////////////////////////////////////
// Class:       MyAnalyzer
// Module Type: analyzer
// File:        MyPDDPTestAna_module.cc
//
// Generated at Tue Oct  8 14:37:54 2019 by Raphaël Bajou,,, using artmod
// from cetpkgsupport v1_14_01.
////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art_root_io/TFileService.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Utilities/InputTag.h"

#include "lardataobj/RecoBase/PFParticle.h"
#include "lardataobj/AnalysisBase/Calorimetry.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/Track.h"
#include "lardataobj/RecoBase/SpacePoint.h"
#include "lardataobj/RecoBase/TrackHitMeta.h"

#include "TTree.h"
#include "TH1D.h"

namespace test {
  class MyPDDPTestAna;
}

class test::MyPDDPTestAna : public art::EDAnalyzer {
public:
  explicit MyPDDPTestAna(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  MyPDDPTestAna(MyPDDPTestAna const &) = delete;
  MyPDDPTestAna(MyPDDPTestAna &&) = delete;
  MyPDDPTestAna & operator = (MyPDDPTestAna const &) = delete;
  MyPDDPTestAna & operator = (MyPDDPTestAna &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;
  void beginJob();
  void endJob();

private:
  
  // Declare member data here.
  TTree *fOutputTree;

  unsigned int fEventID;
  unsigned int fNPFParticles;
  unsigned int fNPrimaries;
  int fNPrimaryDaughters;
  int fNTracks;
  std::vector< double > fTrackLength;
  std::vector< int > fNHits;
  std::vector< int > fView;
  std::vector< double > fX, fStartX, fEndX; 
  std::vector< double > fY, fStartY, fEndY;
  std::vector< double > fZ, fStartZ, fEndZ;
  std::vector< double > fStartTick;
  std::vector< double > fPeakTime;
  std::vector< double > fHitIntegral;
  std::vector< double > fdQdx;
  //  std::vector< double > fPeakTime0;
  //  std::vector< double > fPeakTime1;
  //std::vector< double > fHitIntegral0; 
  //std::vector< double > fHitIntegral1; 
  std::vector< float > fdQdx0;
  std::vector< float > fdQdx1;
  std::vector< int > fPlanenum;
  TH1D *fdQdxhist;

  std::string fPFParticleLabel;
  std::string fTrackModuleLabel;
  std::string fSpacePointModuleLabel;
  std::string fCalorimetryLabel;
  std::string fHitModuleLabel;
  
  //Constantes
  float C = 89.1; //[ADC/fC] : calibration constante
  
};


test::MyPDDPTestAna::MyPDDPTestAna(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p) 
// Initialize member data here.
{
  
  // Call appropriate produces<>() functions here.
  fPFParticleLabel       = p.get<std::string>("PFParticleLabel");
  fTrackModuleLabel      = p.get<std::string>("TrackModuleLabel");
  fSpacePointModuleLabel = p.get<std::string>("SpacePointModuleLabel");
  fCalorimetryLabel      = p.get<std::string>("CalorimetryLabel");
  fHitModuleLabel        = p.get<std::string>("HitModuleLabel");

}



void test::MyPDDPTestAna::analyze(art::Event const & e)                                       
{  
  // Implementation of required member function here.
  fEventID = e.id().event();

  fNPFParticles = 0;
  fNPrimaries   = 0;
  fNTracks      = 0;
  fTrackLength.clear();
  fNHits.clear();
  fX.clear(); fY.clear(); fZ.clear();
  fStartX.clear(); fStartY.clear(); fStartZ.clear(); 
  fEndX.clear(); fEndY.clear(); fEndZ.clear(); 
  fStartTick.clear();
  fView.clear();
  fPeakTime.clear();//fPeakTime0.clear(); fPeakTime1.clear();
  fHitIntegral.clear();//fHitIntegral0.clear(); fHitIntegral1.clear();
  fdQdx.clear();
  fdQdx0.clear(); fdQdx1.clear();
  fPlanenum.clear(); 
  
  art::Handle< std::vector<recob::PFParticle> > pfparticleListHandle;
  std::vector<art::Ptr<recob::PFParticle> > pfparticlelist;
  art::Handle< std::vector<recob::Track> > trackListHandle;
  std::vector<art::Ptr<recob::Track> > tracklist;
  art::Handle< std::vector<recob::SpacePoint> > spacepointListHandle;              
  std::vector<art::Ptr<recob::SpacePoint> > spacepointlist;
  art::Handle< std::vector<recob::Hit> > hitListHandle;       
  std::vector<art::Ptr<recob::Hit> > hitlist;
  if(e.getByLabel("pandora", pfparticleListHandle)) {
    art::fill_ptr_vector(pfparticlelist, pfparticleListHandle);
  }
  if(e.getByLabel("pandoraTrack", trackListHandle)) { //make sure the Handle is valid
    art::fill_ptr_vector(tracklist, trackListHandle);
  }
  if(e.getByLabel("pandora", spacepointListHandle)) {
    art::fill_ptr_vector(spacepointlist, spacepointListHandle);                      
  }
  if(e.getByLabel("dprawhit", hitListHandle)) {
    art::fill_ptr_vector(hitlist, hitListHandle);                
  }

  if(!pfparticlelist.size()) return;
  fNPFParticles = pfparticlelist.size();

  art::FindManyP<recob::Track> trackAssoc(pfparticlelist, e, fTrackModuleLabel); //accessing the recob::Track objects associated with everything in the pfparticlelist vector
  art::FindManyP<recob::SpacePoint> spacepointAssoc(pfparticlelist, e, fSpacePointModuleLabel);
  art::FindManyP<recob::Hit> hittrackAssoc(tracklist, e, fTrackModuleLabel);
  art::FindManyP<recob::Hit> hitspAssoc(spacepointlist, e, fHitModuleLabel);
  art::FindManyP<anab::Calorimetry> calorimetryAssoc(tracklist, e, fCalorimetryLabel);
  art::FindManyP<recob::Hit, recob::TrackHitMeta> fmthm(tracklist, e, fTrackModuleLabel);  
  
  for(const art::Ptr<recob::PFParticle> &pfp : pfparticlelist){
    
    
    if( !(pfp->IsPrimary() && std::abs(pfp->PdgCode()) == 13) ) continue; 
    fNPrimaries++;
    std::vector< art::Ptr<recob::SpacePoint> > pfpspacepoint = spacepointAssoc.at(pfp.key());
    std::vector< art::Ptr<recob::Track> > pfptrack = trackAssoc.at(pfp.key());
    if(!pfptrack.empty() && !pfpspacepoint.empty()){
      //fNTracks++;
      // for(const art::Ptr<recob::SpacePoint> &sp : pfpspacepoint){
	// fX.push_back(sp->XYZ()[0]); fY.push_back(sp->XYZ()[1]); fZ.push_back(sp->XYZ()[2]); */
  //}
      for(const art::Ptr<recob::Track> &trk: pfptrack){
	fNTracks++;
	std::vector< art::Ptr<recob::Hit> > trackhit = hittrackAssoc.at(trk.key());
	if(!(trackhit[trk->FirstValidPoint()]->PeakTime() > 100) ) continue;
	//std::vector< recob::TrackHitMeta const* > & data
	fTrackLength.push_back(trk->Length());
        if(!trackhit.empty()){
	  fNHits.push_back(trackhit.size());
	  fStartTick.push_back(trackhit[trk->FirstValidPoint()]->PeakTime());
          for(const art::Ptr<recob::Hit>  &hit : trackhit){
	    int view = hit->WireID().Plane;
	    fView.push_back(view);
	    fPeakTime.push_back(hit->PeakTime());
	    double dq = hit->Integral();
	    fHitIntegral.push_back(dq);
	    
	  }  
        } 
	
	fStartX.push_back(trk->Start().X()); fStartY.push_back(trk->Start().Y()); fStartZ.push_back(trk->Start().Z());
        fEndX.push_back(trk->End().X()); fEndY.push_back(trk->End().Y()); fEndZ.push_back(trk->End().Z()); 

	std::vector< art::Ptr<anab::Calorimetry> > trackcalo = calorimetryAssoc.at(trk.key());
        if (!trackcalo.empty()){

	  for (const art::Ptr <anab::Calorimetry> &cal : trackcalo){
	    if(!cal->PlaneID().isValid) continue; 
	    int planenum = cal->PlaneID().Plane;
	    fPlanenum.push_back(planenum);  
	    int calsize = cal->dQdx().size();
	    for(int i = 0; i < calsize ; i++){
	      fX.push_back(cal->XYZ()[i].X()); fY.push_back(cal->XYZ()[i].Y()); fZ.push_back(cal->XYZ()[i].Z());
	    }
	    if (planenum == 0){
	      for(float dqdx : cal->dQdx()){
		fdQdx0.push_back( dqdx / C );  // C = 89.1 [ADC/fC]
		fdQdxhist->Fill( dqdx / C);
	      }  
	    }
	    if (planenum == 1){
	      for(float dqdx : cal->dQdx()){
		fdQdx1.push_back( dqdx / C );  // C = 89.1 [ADC/fC]
		fdQdxhist->Fill( dqdx / C);
	      } 
	    }
	  }
	  
        }//end if(!trackcalo.empty())	
      }//end for loop on pfptracks
    }//end if(!pfptrack.empty())
  }//end for loop on pfparticles
  
  fOutputTree->Fill(); 

}

void test::MyPDDPTestAna::beginJob()
{
  // Implementation of optional member function here.
  art::ServiceHandle<art::TFileService> tfs;
  fOutputTree = tfs->make<TTree >("mytree", "My Tree");
  fOutputTree->Branch("eventID", &fEventID, "eventID/i");
  fOutputTree->Branch("nPFParticles", &fNPFParticles, "nPFParticles/i");
  fOutputTree->Branch("nPrimaries", &fNPrimaries, "nPrimaries/i");
  fOutputTree->Branch("nTracks", &fNTracks, "nTracks/i");
  fOutputTree->Branch("nPrimaryDaughters", &fNPrimaryDaughters, "nPrimaryDaughters/i");
  fOutputTree->Branch("TrackLength", &fTrackLength);
  fOutputTree->Branch("nHits", &fNHits);
  fOutputTree->Branch("X", &fX);//, "X/D" ); 
  fOutputTree->Branch("Y", &fY);//, "Y/D" );
  fOutputTree->Branch("Z", &fZ);//, "Z/D" );
  fOutputTree->Branch("StartX", &fStartX);
  fOutputTree->Branch("StartY", &fStartY);
  fOutputTree->Branch("StartZ", &fStartZ);
  fOutputTree->Branch("EndX", &fEndX);
  fOutputTree->Branch("EndY", &fEndY);
  fOutputTree->Branch("EndZ", &fEndZ);
  fOutputTree->Branch("StartTick", &fStartTick);
  fOutputTree->Branch("View", &fView);
  fOutputTree->Branch("PeakTime", &fPeakTime);
  //fOutputTree->Branch("PeakTime0", &fPeakTime0);
  //fOutputTree->Branch("PeakTime1", &fPeakTime1);
  fOutputTree->Branch("HitIntegral", &fHitIntegral);
  //fOutputTree->Branch("HitIntegral0", &fHitIntegral0);
  //fOutputTree->Branch("HitIntegral1", &fHitIntegral1);
  fOutputTree->Branch("dQdx0", &fdQdx0);
  fOutputTree->Branch("dQdx1", &fdQdx1);
  fOutputTree->Branch("Planenum", &fPlanenum); //, "");
  
  fdQdxhist = tfs->make<TH1D>("hdQdx", ";dQdx [fC/cm]", 50, 0, 50);
}

void test::MyPDDPTestAna::endJob()
{
}


DEFINE_ART_MODULE(test::MyPDDPTestAna)




