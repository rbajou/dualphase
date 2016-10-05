////////////////////////////////////////////////////////////////////////
// Class:       Matt2
// Module Type: analyzer
// File:        Matt2_module.cc
//
// Generated at Mon Oct 26 13:34:36 2015 by Matt Robinson using artmod
// from cetpkgsupport v1_08_07.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art/Framework/Services/Optional/TFileService.h"

#include "larsim/Simulation/SimChannel.h"
#include "larcore/Geometry/Geometry.h"
#include "SimulationBase/MCTruth.h"

//#include "RawData/RawDigit.h"
//#include "RawData/raw.h"

#include <iostream>
#include <algorithm>
#include <fstream>

#include "TDatabasePDG.h"
#include "TTree.h"

struct IDEYLess
{
  bool operator()(const sim::IDE& first, const sim::IDE& second)
  {
    return first.y < second.y;
  }
};

#define MAXKAON 1000
#define MAXHISTORY 100

class Matt2;

class Matt2 : public art::EDAnalyzer {
public:
  explicit Matt2(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  Matt2(Matt2 const &) = delete;
  Matt2(Matt2 &&) = delete;
  Matt2 & operator = (Matt2 const &) = delete;
  Matt2 & operator = (Matt2 &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

  //optionals
  void beginJob() override;
  void endJob() override;

private:

  // Declare member data here.
  art::ServiceHandle<geo::Geometry> geom_;
  art::ServiceHandle<art::TFileService> tfs_;
  TDatabasePDG* pdg_;
  TTree* tree_;
  unsigned runid_;
  unsigned eventid_;
  double etotal_;
  int nkaon_;
  int pdgcode_[MAXKAON];
  int nparent_[MAXKAON];
  int totalparent_;
  int pdgparent_[MAXKAON*MAXHISTORY];
  double ekaon_[MAXKAON];
  double edaughters_[MAXKAON];
  double edecay_[MAXKAON];
  double edgedist_[3][2];
  double muonrange_;
  double minx_, maxx_, miny_, maxy_, minz_, maxz_;
};


Matt2::Matt2(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p),
  geom_(),
  pdg_(nullptr),
  minx_(99999999),
  maxx_(-99999999),
  miny_(99999999),
  maxy_(-99999999),
  minz_(99999999),
  maxz_(-99999999)  
{
  pdg_=new TDatabasePDG();
  pdg_->ReadPDGTable();


  for (unsigned int c=0; c<geom_->Ncryostats(); c++)
    {
      const geo::CryostatGeo& cryostat=geom_->Cryostat(c);
      for (unsigned int t=0; t<cryostat.NTPC(); t++)
	{
	  geo::TPCID id;
	  id.Cryostat=c;
	  id.TPC=t;
	  id.isValid=true;
	  const geo::TPCGeo& tpc=cryostat.TPC(id);
	  std::cout << t << "\t" << (tpc.Length()/2) << ", " << (tpc.ActiveLength()/2) << std::endl;
	  if (tpc.MinX()<minx_) minx_=tpc.MinX();
	  if (tpc.MaxX()>maxx_) maxx_=tpc.MaxX();
	  if (tpc.MinY()<miny_) miny_=tpc.MinY();
	  if (tpc.MaxY()>maxy_) maxy_=tpc.MaxY();
	  if (tpc.MinZ()<minz_) minz_=tpc.MinZ();
	  if (tpc.MaxZ()>maxz_) maxz_=tpc.MaxZ();
	  
	}
    }
    std::cout << minx_ << ", " << maxx_ << "\t" <<  miny_ << ", " << maxy_ << "\t" <<  minz_ << ", " << maxz_ << std::endl;
		  // unsigned cryo=geom_->FindCryostatAtPosition(pos);
		  // //		  std::cout << "cryo = " << cryo << std::endl;
		  // if (cryo)
		  //   {
		  //     std::cout << "hit outside cryostat\n";
		  //   }
		  // else
		  //   {
		  //     unsigned tpc=geom_->Cryostat(cryo).FindTPCAtPosition(pos, 1.);
		  //     if (tpc>299)
		  // 	std::cout << ide.x << ", " << ide.y << ", " << ide.z << "  tpc = " << tpc << std::endl;
		  //   }
}

void Matt2::beginJob()
{
  tree_=tfs_->make<TTree>("kaons", "kaon event information");
  tree_->Branch("run", &runid_, "run/i");
  tree_->Branch("event", &eventid_, "event/i");
  tree_->Branch("etotal", &etotal_, "etotal/D");
  tree_->Branch("nkaon", &nkaon_, "nkaon/I");
  tree_->Branch("pdgcode", pdgcode_, "pdgcode[nkaon]/I");
  tree_->Branch("nparent", nparent_, "nparent[nkaon]/I");
  tree_->Branch("totalparent", &totalparent_, "totalparent/I"); 
  tree_->Branch("pdgparent", pdgparent_, "pdgparent[totalparent]/I");
  tree_->Branch("ekaon", ekaon_, "ekaon[nkaon]/D");
  tree_->Branch("edaughters", edaughters_, "edaughters[nkaon]/D");
  tree_->Branch("edecay", edecay_, "edecay[nkaon]/D");
  tree_->Branch("edgedist", edgedist_, "edgedist[3][2]/D");
  tree_->Branch("muonrange", &muonrange_, "muonrange/D");

}
void Matt2::endJob()
{
}
void Matt2::analyze(art::Event const & event)
{
  // art::Handle<std::vector<raw::RawDigit> > rawdigits;
  // event.getByLabel("daq", rawdigits);
  // for (auto rd:*rawdigits)
  //   {
  //     raw::RawDigit::ADCvector_t digits(rd.Samples());
  //     raw::Uncompress(rd.ADCs(), digits, rd.Compression());
  //     std::cout << rd.NADC() << std::endl;
  //   }

  runid_=event.run();
  eventid_=event.event();

  art::Handle<std::vector<simb::MCParticle> > truth;
  event.getByLabel("largeant", truth);
  std::map<int, const simb::MCParticle*> truthmap;
  for (size_t i=0; i<truth->size(); i++)
    truthmap[truth->at(i).TrackId()]=&((*truth)[i]);

  //  std::cout << "number of truth " << truth->size() << " " << truthmap.size() << std::endl;
  // std::vector<int> kaontrackids;
  // for (std::vector<simb::MCParticle>::size_type i=0; i<truth->size(); i++)
  //   {
  //     const simb::MCParticle& part=truth->at(i);
  //     int pdgcode=part.PdgCode();
  //     TParticlePDG* rpart=pdg_->GetParticle(pdgcode);
  //     if (rpart && rpart->GetName()[0]=='K')
  //      	{
  // 	  kaontrackids
  //     // 	  std::cout << i << "\t" << part.PdgCode() << "\t\"" << (rpart?rpart->GetName():"Nucleus") << "\"" << std::endl;
  //     // 	  double startpos[3];
  //     // 	  startpos[0]=part.Position().X();
  //     // 	  startpos[1]=part.Position().Y();
  //     // 	  startpos[2]=part.Position().Z();
  //     // 	  std::cout << "\t" << startpos[0] << ", " << startpos[1] << ", " << startpos[2] << std::endl;
  //     // 	  geo::TPCID tpcid=geom_->FindTPCAtPosition(startpos);
  //     // 	  std::cout << "\t" << part.NumberDaughters() << " daughters, tpc " << (tpcid.isValid?"In TPC":"Not In TPC") << std::endl;
  //     // 	  for (int j=0; j<part.NumberDaughters(); j++)
  //     // 	    std::cout << "\t\t" << j << "\t" << part.Daughter(j) << std::endl;
  //     // 	}
  //     //      std::cout << "\t" << i << "\t" << part.PdgCode() << std::endl;
  //   }
  // //  art::Handle<std::vector<sim::SimChannel> > simchannels;
  // //  event.getByLabel("largeant", simchannels);

  art::Handle<std::vector<sim::SimChannel> > simchannels;
  event.getByLabel("largeant", simchannels);
  std::vector<sim::IDE> priides;
  nkaon_=0;
  etotal_=0;
  memset(pdgcode_, 0, sizeof(pdgcode_));
  memset(pdgparent_, 0, sizeof(pdgparent_));
  memset(ekaon_, 0, sizeof(ekaon_));
  memset(edaughters_, 0, sizeof(edaughters_));
  memset(edecay_, 0, sizeof(edecay_));
  memset(nparent_, 0, sizeof(nparent_));
  totalparent_=0;
  for (int i=0; i<3; i++)
    for (int j=0; j<2; j++)
      edgedist_[i][j]=99999999.;
  muonrange_=0;
  std::vector<int> kaonids;
  for (auto const& simchannel:*simchannels)
    {
      if (geom_->SignalType(simchannel.Channel()) == geo::kCollection)
	{
	  for (auto const& tdcide:simchannel.TDCIDEMap())
	    {
	      //unsigned int tdc=tdcide.first;
	      auto const& idevec=tdcide.second;
	      for (auto const& ide:idevec)
		{
		  etotal_+=ide.energy;
		  int trackID=ide.trackID;
		  if (trackID==1)
		    priides.push_back(ide);
		  //		  if (trackID<0)
		  //		    std::cout << "negative track id: " << trackID << std::endl;
		  double pos[3];
		  pos[0]=ide.x;
		  pos[1]=ide.y;
		  pos[2]=ide.z;

		  geo::TPCID tpcid=geom_->FindTPCAtPosition(pos);
		  
		  if (geom_->HasTPC(tpcid))
		    {
		      // if (false && tpclocs.find(tpcid)==tpclocs.end())
		      // 	{
		      // 	  const geo::TPCGeo& tpcgeo=geom_->GetElement(tpcid);
		      // 	  tpclocs[tpcid][0][0]=tpcgeo.MinX();
		      // 	  tpclocs[tpcid][0][1]=tpcgeo.MaxX();
		      // 	  tpclocs[tpcid][1][0]=tpcgeo.MinY();
		      // 	  tpclocs[tpcid][1][1]=tpcgeo.MaxY();
		      // 	  tpclocs[tpcid][2][0]=tpcgeo.MinZ();
		      // 	  tpclocs[tpcid][2][1]=tpcgeo.MaxZ();
		      // 	  for (auto const& it:tpclocs)
		      // 	    {
		      // 	      std::cout << it.first << "\t" << 
		      // 		it.second[0][0] << ", " << it.second[0][1] <<
		      // 		" (" << (it.second[0][1]-it.second[0][0]) << ")\t" <<	
		      // 		it.second[1][0] << ", " << it.second[1][1] << 
		      // 		" (" << (it.second[1][1]-it.second[1][0]) << ")\t" <<	
		      // 		it.second[2][0] << ", " << it.second[2][1] << 
		      // 		" (" << (it.second[2][1]-it.second[2][0]) << ")" <<	
		      // 		std::endl;
		      // 	    }
		      // 	}
		      // const double MAXX=745.744;
		      // const double MINX=-MAXX;
		      // const double MAXY=607.829;
		      // const double MINY=-MAXY;
		      // const double MINZ=-0.87625;
		      // const double MAXZ=5808.87;
		      // double dx1=ide.x-tpcgeo.MinX();
		      // double dx2=tpcgeo.MaxX()-ide.x;
		      // double dy1=ide.y-tpcgeo.MinY();
		      // double dy2=tpcgeo.MaxY()-ide.y;
		      // double dz1=ide.z-tpcgeo.MinZ();
		      // double dz2=tpcgeo.MaxZ()-ide.z;
		      double dx1=ide.x-minx_;
		      double dx2=maxx_-ide.x;
		      double dy1=ide.y-miny_;
		      double dy2=maxy_-ide.y;
		      double dz1=ide.z-minz_;
		      double dz2=maxz_-ide.z;
		      if (dx1<edgedist_[0][0]) edgedist_[0][0]=dx1;
		      if (dx2<edgedist_[0][1]) edgedist_[0][1]=dx2;
		      if (dy1<edgedist_[1][0]) edgedist_[1][0]=dy1;
		      if (dy2<edgedist_[1][1]) edgedist_[1][1]=dy2;
		      if (dz1<edgedist_[2][0]) edgedist_[2][0]=dz1;
		      if (dz2<edgedist_[2][1]) edgedist_[2][1]=dz2;

		  bool iskaon=false;
		  bool fromdecay=false;
		  while (trackID!=0 && !iskaon)
		    {
		      //		      std::cout << trackID << std::endl;
		      
		      const simb::MCParticle& part=*(truthmap[abs(trackID)]);

		      int pdgcode=part.PdgCode();                     
		      TParticlePDG* rpart=pdg_->GetParticle(pdgcode);
		      if (part.Process()!="KaonPlusInelastic" && part.Process()!="KaonMinusInelastic" && rpart && rpart->GetName()[0]=='K' && (strstr(rpart->GetName(), "+") || strstr(rpart->GetName(), "-")))
			{
			  int kaonnum;
			  std::vector<int>::iterator it=std::find(kaonids.begin(), kaonids.end(), abs(trackID));
			  if (it==kaonids.end())
			    {
			      kaonids.push_back(abs(trackID));
			      pdgcode_[nkaon_]=pdgcode;
			      int parent=part.Mother();
			      while (parent>0)
				{
				  int pdg=truthmap[parent]->PdgCode();
				  if (nparent_[nkaon_]==0 || pdg!=pdgparent_[totalparent_-1])
				    {
				      pdgparent_[totalparent_]=pdg;
				      nparent_[nkaon_]++;
				      totalparent_++;
				    }
				  parent=truthmap[parent]->Mother();
				}
			      kaonnum=nkaon_;
			      nkaon_++;
			    }
			  else
			    kaonnum=it-kaonids.begin();
			  if (trackID==ide.trackID)
			    {
			      ekaon_[kaonnum]+=ide.energy;
			    }
			  else if (fromdecay)
			    edecay_[kaonnum]+=ide.energy;
			  else
			    edaughters_[kaonnum]+=ide.energy;
			  iskaon=true;
			}
		      else
			{
			  trackID=part.Mother();
			  // if (trackID==329237 && event.event()==431829565)
			  //   {
			  //     std::cout << (rpart?rpart->GetName():"UNKNOWN") << " created from K- by " << part.Process() << std::endl;
			  if (trackID>0)
			    rpart=pdg_->GetParticle((truthmap[abs(trackID)])->PdgCode());
			  //   }
			  if (trackID>0 && rpart && rpart->GetName()[0]=='K' && 
			      (strstr(rpart->GetName(), "+") || strstr(rpart->GetName(), "-") ) && 
			      (part.Process()=="Decay" || part.Process()=="hBertiniCaptureAtRest"))
			    {
			      fromdecay=true;
			      //			      std::cout << "*****DECAY in event " << event.event() << std::endl;
			    }
			  // else if (part.Process()!="primary" &&
			  // 	   part.Process()!="Decay" &&
			  // 	   part.Process()!="muMinusCaptureAtRest" &&
			  // 	   part.Process()!="hBertiniCaptureAtRest" &&
			  // 	   part.Process()!="hadElastic" &&
			  // 	   part.Process()!="neutronInelastic" &&
			  // 	   part.Process()!="protonInelastic" &&
			  // 	   part.Process()!="pi-Inelastic" &&
			  // 	   part.Process()!="pi+Inelastic" &&
			  // 	   part.Process()!="muonNuclear" &&
			  // 	   part.Process()!="nCapture" &&
			  // 	   part.Process()!="kaon0LInelastic" &&
			  // 	   part.Process()!="anti_protonInelastic" &&
			  // 	   part.Process()!="kaon-Inelastic" &&
			  // 	   part.Process()!="kaon+Inelastic" &&
			  // 	   part.Process()!="CoulombScat")
			  //   std::cout << "******* PROCESS: " << part.Process() 
			  //				      << std::endl;
			}
		    }
		    }
		  else
		    std::cout << "hit outside all tpcs: " << ide.x << ", " << ide.y << ", " << ide.z << std::endl;
		}
	    }
	}
    }
  if (nkaon_)
    {
      std::cout << "Charged Kaon(s) found" << std::endl;
      std::cout << "Total Energy " << etotal_ << " MeV\n";
      std::cout << "Distance to edge of active volume: " << edgedist_[0][0] << "," << edgedist_[0][1] << " " << edgedist_[1][0] << "," << edgedist_[1][1] << " " << edgedist_[2][0] << "," << edgedist_[2][1] << std::endl;
      if (priides.size())
	{
	  std::sort(priides.begin(), priides.end(), IDEYLess());
	  double dx=priides.front().x-priides.back().x;
	  double dy=priides.front().y-priides.back().y;
	  double dz=priides.front().z-priides.back().z;
	  muonrange_=sqrt(dx*dx+dy*dy+dz*dz);
	  std::cout << "muon track length = " << muonrange_ << " cm" << std::endl;
	}
      else
	std::cout << "no muon hits\n";
      std::cout << "Number of Kaons " << nkaon_ << std::endl;
      int parentcount=0;
      for (int i=0; i<nkaon_; i++)
	{
	  std::cout << "Kaon " << i << ": " << pdg_->GetParticle(pdgcode_[i])->GetName() << std::endl;
	  std::cout << "Energy from Kaon " << i << ": " << ekaon_[i] << std::endl;
	  std::cout << "Energy from Kaon " << i << " secondaries: " << edaughters_[i] << std::endl;
	  std::cout << "Energy from Kaon " << i << " decay products: " << edecay_[i] << std::endl;
	  std::cout << "Ancestry:";
	  for (int j=0; j<nparent_[i]; j++)
	    {
	      TParticlePDG* p=pdg_->GetParticle(pdgparent_[parentcount]);
	      std::cout << " <- " << (p?p->GetName():"UNKNOWN");
	      parentcount++;
	    }
	  std::cout << std::endl;
	}
      tree_->Fill();
      std::cout << totalparent_ << ":" << std::endl;
      for (int i=0; i<totalparent_; i++)
	std::cout << "\t" << pdgparent_[i] << std::endl;
    }
}

DEFINE_ART_MODULE(Matt2)
