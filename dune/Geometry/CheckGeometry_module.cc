////////////////////////////////////////////////////////////////////////
// Class:       CheckGeometry
// Module Type: analyzer
// File:        CheckGeometry_module.cc
//
// Generated at Tue Jan  6 22:27:12 2015 by Tingjun Yang using artmod
// from cetpkgsupport v1_07_00.
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art_root_io/TFileService.h" 
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "larcore/Geometry/Geometry.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "TCanvas.h"
#include "TBox.h"
#include "TH2F.h"
#include "TLine.h"
#include "TLatex.h"
#include "TLegend.h"

#include <iostream>

//constexpr unsigned short kMaxAuxDets = 100; // unused
//constexpr unsigned short kMaxTkIDs = 100; // unused

namespace dune {
  class CheckGeometry;
}

class dune::CheckGeometry : public art::EDAnalyzer {
public:
  explicit CheckGeometry(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  CheckGeometry(CheckGeometry const &) = delete;
  CheckGeometry(CheckGeometry &&) = delete;
  CheckGeometry & operator = (CheckGeometry const &) = delete;
  CheckGeometry & operator = (CheckGeometry &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

  // Selected optional functions.
  void reconfigure(fhicl::ParameterSet const & p) ;

private:

  // Declare member data here.

};


dune::CheckGeometry::CheckGeometry(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p)
{
  reconfigure(p);
}

void dune::CheckGeometry::analyze(art::Event const & evt)
{

  art::ServiceHandle<geo::Geometry> geo;

  std::cout<<"channel = "<<geo->PlaneWireToChannel(0,600,1)<<std::endl;

  TCanvas *can = new TCanvas("c1","c1");
  can->cd();
  std::vector<TBox*> TPCBox;
  std::vector<TLine*> Wires;

  double minx = 1e9;
  double maxx = -1e9;
  double miny = 1e9;
  double maxy = -1e9;
  double minz = 1e9;
  double maxz = -1e9;
  
  int nwires = 0;
  std::vector<int> nwires_tpc(geo->NTPC());
  for (size_t i = 0; i<geo->NTPC(); ++i) nwires_tpc[i] = 0;
  for (size_t t = 0; t<geo->NTPC(); ++t){
    //if (t%2==0) continue;
    double local[3] = {0.,0.,0.};
    double world[3] = {0.,0.,0.};
    const geo::TPCGeo &tpc = geo->TPC(t);
    tpc.LocalToWorld(local,world);
    if (minx>world[0]-tpc.ActiveHalfWidth())
      minx = world[0]-tpc.ActiveHalfWidth();
    if (maxx<world[0]+tpc.ActiveHalfWidth())
      maxx = world[0]+tpc.ActiveHalfWidth();
    if (miny>world[1]-tpc.ActiveHalfHeight())
      miny = world[1]-tpc.ActiveHalfHeight();
    if (maxy<world[1]+tpc.ActiveHalfHeight())
      maxy = world[1]+tpc.ActiveHalfHeight();
    if (minz>world[2]-tpc.ActiveLength()/2.)
      minz = world[2]-tpc.ActiveLength()/2.;
    if (maxz<world[2]+tpc.ActiveLength()/2.)
      maxz = world[2]+tpc.ActiveLength()/2.;
    std::cout<<t<<" "<<world[0]-tpc.ActiveHalfWidth()
	     <<" "<<world[0]+tpc.ActiveHalfWidth()
	     <<" "<<world[1]-tpc.ActiveHalfHeight()
	     <<" "<<world[1]+tpc.ActiveHalfHeight()
	     <<" "<<world[2]-tpc.ActiveLength()/2.
	     <<" "<<world[2]+tpc.ActiveLength()/2.<<std::endl;
 
    TPCBox.push_back(new TBox(world[2]-tpc.ActiveLength()/2.,
			      world[1]-tpc.ActiveHalfHeight(),
			      world[2]+tpc.ActiveLength()/2.,
			      world[1]+tpc.ActiveHalfHeight()));
    TPCBox.back()->SetFillStyle(0);
    TPCBox.back()->SetLineStyle(2);
    TPCBox.back()->SetLineWidth(2);
    TPCBox.back()->SetLineColor(16);
    
    for (size_t p = 0; p<geo->Nplanes(t);++p){
      for (size_t w = 0; w<geo->Nwires(p,t); ++w){
	++nwires;
	++nwires_tpc[t];
	double xyz0[3];
	double xyz1[3];
	unsigned int c = 0;
//	if ((t==7&&p==0&&w==192)||
//	    (t==7&&p==1&&w==112)||
//	    (t==7&&p==2&&w==0)){
//	if (true){
        if ((t==2||t==6||t==10)&&p==0&&w%10==0){
	  geo->WireEndPoints(c,t,p,w,xyz0,xyz1);
	  Wires.push_back(new TLine(xyz0[2],xyz0[1],xyz1[2],xyz1[1]));
	}
	//std::cout<<t<<" "<<p<<" "<<w<<" "<<xyz0[0]<<" "<<xyz0[1]<<" "<<xyz0[2]<<std::endl;
      }
    }
  }

  TH2F *frame = new TH2F("frame",";z (cm);y (cm)",100,minz,maxz,100,miny,maxy);
  frame->SetStats(0);
  frame->Draw();
  for (auto box: TPCBox) box->Draw();
  for (auto wire: Wires) wire->Draw();
  can->Print("wires.pdf");
  std::cout<<"N wires = "<<nwires<<std::endl;
  for (size_t i = 0; i<geo->NTPC(); ++i){
    std::cout<<"TPC "<<i<<" has "<<nwires_tpc[i]<<" wires"<<std::endl;
  }

  //CRT
  std::vector<TBox*> CRTBox0;
  std::vector<TBox*> CRTBox1;
  std::vector<std::vector<TBox*>> CRTStrips(2);
  std::vector<int> modules0;
  std::vector<int> modules1;
  std::vector<TText *> text0;
  std::vector<TText *> text1;
  std::vector<double> module_x(32);
  std::vector<double> module_y(32);
  std::vector<double> module_z(32);
  std::vector<double> pixel_x(32);
  std::vector<double> pixel_y(32);
  std::vector<double> pixel_z(32);
  for (unsigned int i = 0; i < geo->NAuxDets(); ++i){
    auto& auxdet = geo->AuxDet(i);
    double xyz[3];
    auxdet.GetCenter(xyz);
    std::cout<<"Aux "<<i<<" "<<xyz[0]<<" "<<xyz[1]<<" "<<xyz[2]<<" "<<auxdet.HalfWidth1()<<" "<<auxdet.HalfHeight()<<" "<<auxdet.Length()/2<<std::endl;
    double auxdet0[3] = {-auxdet.HalfWidth1(), auxdet.HalfHeight(), -auxdet.Length()/2};
    double world0[3];
    auxdet.LocalToWorld(auxdet0, world0);
    double auxdet1[3] = {auxdet.HalfWidth1(), auxdet.HalfHeight(), auxdet.Length()/2};
    double world1[3];
    auxdet.LocalToWorld(auxdet1, world1);
    module_x[i] = (world0[0]+world1[0])/2;
    module_y[i] = (world0[1]+world1[1])/2;
    module_z[i] = (world0[2]+world1[2])/2;
    if (xyz[2] < 0){ //front
      CRTBox0.push_back(new TBox(world0[0],world0[1],world1[0],world1[1]));
      CRTBox0.back()->SetFillStyle(0);
      //CRTBox0.back()->SetLineStyle(i/8+1);
      CRTBox0.back()->SetLineColor(i%8+1);
      CRTBox0.back()->SetLineWidth(2);
      modules0.push_back(i);
      text0.push_back(new TText((world0[0]+world1[0])/2,(world0[1]+world1[1])/2,Form("%d",i)));
      text0.back()->SetTextColor(i%8+1);
    }
    else{
      CRTBox1.push_back(new TBox(world0[0],world0[1],world1[0],world1[1]));
      CRTBox1.back()->SetFillStyle(0);
      //CRTBox1.back()->SetLineStyle(i/8+1);
      CRTBox1.back()->SetLineColor(i%8+1);
      CRTBox1.back()->SetLineWidth(2);
      modules1.push_back(i);
      text1.push_back(new TText((world0[0]+world1[0])/2,(world0[1]+world1[1])/2,Form("%d",i)));
      text1.back()->SetTextColor(i%8+1);
    }
    if (i==0){
      std::cout<<"Aux "<<i<<" has "<<auxdet.NSensitiveVolume()<<std::endl;
    }
  }

  pixel_x[0] = module_x[23];
  pixel_y[0] = module_y[29];
  pixel_z[0] = module_z[29];

  pixel_x[1] = module_x[22];
  pixel_y[1] = module_y[29];
  pixel_z[1] = module_z[29];

  pixel_x[2] = module_x[21];
  pixel_y[2] = module_y[28];
  pixel_z[2] = module_z[28];

  pixel_x[3] = module_x[20];
  pixel_y[3] = module_y[28];
  pixel_z[3] = module_z[28];

  pixel_x[4] = module_x[20];
  pixel_y[4] = module_y[16];
  pixel_z[4] = module_z[16];

  pixel_x[5] = module_x[4];
  pixel_y[5] = module_y[12];
  pixel_z[5] = module_z[12];

  pixel_x[6] = module_x[4];
  pixel_y[6] = module_y[0];
  pixel_z[6] = module_z[0];

  pixel_x[7] = module_x[5];
  pixel_y[7] = module_y[0];
  pixel_z[7] = module_z[0];

  pixel_x[8] = module_x[6];
  pixel_y[8] = module_y[1];
  pixel_z[8] = module_z[1];

  pixel_x[9] = module_x[7];
  pixel_y[9] = module_y[1];
  pixel_z[9] = module_z[1];

  pixel_x[10] = module_x[7];
  pixel_y[10] = module_y[13];
  pixel_z[10] = module_z[13];

  pixel_x[11] = module_x[23];
  pixel_y[11] = module_y[17];
  pixel_z[11] = module_z[17];

  pixel_x[12] = module_x[22];
  pixel_y[12] = module_y[17];
  pixel_z[12] = module_z[17];

  pixel_x[13] = module_x[21];
  pixel_y[13] = module_y[16];
  pixel_z[13] = module_z[16];

  pixel_x[14] = module_x[5];
  pixel_y[14] = module_y[12];
  pixel_z[14] = module_z[12];

  pixel_x[15] = module_x[6];
  pixel_y[15] = module_y[13];
  pixel_z[15] = module_z[13];

  pixel_x[16] = module_x[26];
  pixel_y[16] = module_y[30];
  pixel_z[16] = module_z[30];

  pixel_x[17] = module_x[27];
  pixel_y[17] = module_y[30];
  pixel_z[17] = module_z[30];

  pixel_x[18] = module_x[24];
  pixel_y[18] = module_y[31];
  pixel_z[18] = module_z[31];

  pixel_x[19] = module_x[25];
  pixel_y[19] = module_y[31];
  pixel_z[19] = module_z[31];

  pixel_x[20] = module_x[25];
  pixel_y[20] = module_y[19];
  pixel_z[20] = module_z[19];
  
  pixel_x[21] = module_x[11];
  pixel_y[21] = module_y[14];
  pixel_z[21] = module_z[14];
  
  pixel_x[22] = module_x[11];
  pixel_y[22] = module_y[2];
  pixel_z[22] = module_z[2];
  
  pixel_x[23] = module_x[10];
  pixel_y[23] = module_y[2];
  pixel_z[23] = module_z[2];
  
  pixel_x[24] = module_x[9];
  pixel_y[24] = module_y[3];
  pixel_z[24] = module_z[3];
  
  pixel_x[25] = module_x[8];
  pixel_y[25] = module_y[3];
  pixel_z[25] = module_z[3];
  
  pixel_x[26] = module_x[8];
  pixel_y[26] = module_y[15];
  pixel_z[26] = module_z[15];
  
  pixel_x[27] = module_x[26];
  pixel_y[27] = module_y[18];
  pixel_z[27] = module_z[18];
  
  pixel_x[28] = module_x[27];
  pixel_y[28] = module_y[18];
  pixel_z[28] = module_z[18];
  
  pixel_x[29] = module_x[24];
  pixel_y[29] = module_y[19];
  pixel_z[29] = module_z[19];
  
  pixel_x[30] = module_x[10];
  pixel_y[30] = module_y[14];
  pixel_z[30] = module_z[14];
  
  pixel_x[31] = module_x[9];
  pixel_y[31] = module_y[15];
  pixel_z[31] = module_z[15];
  
  TLatex tex;
  tex.SetTextSize(0.02);

  TH2F *frcrt0 = new TH2F("frcrt0","Front",100,-300, 600, 100, -100, 650);
  TH2F *frcrt1 = new TH2F("frcrt1","Back",100,-400, 450, 100, -200, 600);
  frcrt0->SetStats(0);
  frcrt1->SetStats(0);
  TCanvas *cancrt = new TCanvas("cancrt","cancrt",1600,800);
  cancrt->Divide(2,1);
  cancrt->cd(1);
  frcrt0->DrawCopy();
  TLegend *leg0 = new TLegend(0.7,0.1,0.9,0.9);
  leg0->SetFillStyle(0);
  for (size_t i = 0; i< CRTBox0.size(); ++i){
    CRTBox0[i]->Draw();
    text0[i]->Draw();
    leg0->AddEntry(CRTBox0[i], Form("Module %d",modules0[i]), "l");
  }
  //leg0->Draw();
  for (int i = 0; i<16; ++i){
    //tex.DrawLatex(pixel_x[i], pixel_y[i], Form("CH %d",i));
  }
  TLatex tt;
  tt.SetTextSize(0.02);
  for (unsigned int i = 0; i < geo->NAuxDets(); ++i){
    auto& auxdet = geo->AuxDet(i);
    double xyz[3];
    auxdet.GetCenter(xyz);
    if (xyz[2] < 0){ //front
      if (auxdet.HalfWidth1()>auxdet.HalfHeight()){//horizontal
        for (int j = 0; j<1; ++j){
          auto& auxdetsen = auxdet.SensitiveVolume(j);
          std::cout<<i<<" "<<j<<" "<<auxdetsen.HalfWidth1()<<" "<<auxdetsen.HalfHeight()<<" "<<auxdetsen.Length()/2<<std::endl;
          double auxdetsen0[3] = {0,0,auxdetsen.Length()/2-20};
          double world0[3];
          auxdetsen.LocalToWorld(auxdetsen0,world0);
          std::cout<<i<<" "<<j<<" "<<world0[0]<<" "<<world0[1]<<std::endl;
          tt.SetTextColor(i%8+1);
          tt.DrawLatex(world0[0], world0[1], Form("%d",j));
        }
      }
    }
  }

  cancrt->cd(2);
  frcrt1->DrawCopy();
  TLegend *leg1 = new TLegend(0.7,0.1,0.9,0.9);
  leg1->SetFillStyle(0);
  for (size_t i = 0; i< CRTBox1.size(); ++i){
    CRTBox1[i]->Draw();
    text1[i]->Draw();
    leg1->AddEntry(CRTBox1[i], Form("Module %d",modules1[i]), "l");
  }
  //leg1->Draw();
  for (int i = 16; i<32; ++i){
    //tex.DrawLatex(pixel_x[i], pixel_y[i], Form("CH %d",i));
  }

  for (unsigned int i = 0; i < geo->NAuxDets(); ++i){
    auto& auxdet = geo->AuxDet(i);
    double xyz[3];
    auxdet.GetCenter(xyz);
    if (xyz[2] > 0){ //front
      for (int j = 0; j<1; ++j){
        auto& auxdetsen = auxdet.SensitiveVolume(j);
        std::cout<<i<<" "<<j<<" "<<auxdetsen.HalfWidth1()<<" "<<auxdetsen.HalfHeight()<<" "<<auxdetsen.Length()/2<<std::endl;
        double auxdetsen0[3] = {0,0,auxdetsen.Length()/2-20};
        double world0[3];
        auxdetsen.LocalToWorld(auxdetsen0,world0);
        std::cout<<i<<" "<<j<<" "<<world0[0]<<" "<<world0[1]<<std::endl;
        tt.SetTextColor(i%8+1);
        tt.DrawLatex(world0[0], world0[1], Form("%d",j));
      }
    }
  }

  tt.SetTextColor(1);
  for (int i = 0; i<2; ++i){
    auto& auxdet = geo->AuxDet(i*2);
    for (size_t j = 0; j<auxdet.NSensitiveVolume(); ++j){
      auto& auxdetsen = auxdet.SensitiveVolume(j);
      double world0[3] = {0};
      double world1[3] = {0};
      double auxdet0[3] = {-auxdetsen.HalfWidth1(),
                           -auxdetsen.HalfHeight(),
                           0};
      double auxdet1[3] = {auxdetsen.HalfWidth1(),
                           auxdetsen.HalfHeight(),
                           0};
      auxdetsen.LocalToWorld(auxdet0, world0);
      auxdetsen.LocalToWorld(auxdet1, world1);
      std::cout<<world0[0]<<" "<<world0[1]<<" "<<world0[2]<<std::endl;
      std::cout<<world1[0]<<" "<<world1[1]<<" "<<world1[2]<<std::endl;
      if (i==0){
        CRTStrips[i].push_back(new TBox(world0[1], world0[2], world1[1], world1[2]));
      }
      else {
        CRTStrips[i].push_back(new TBox(world0[0], world0[2], world1[0], world1[2]));
      }
      CRTStrips[i].back()->SetFillStyle(0);
      CRTStrips[i].back()->SetLineWidth(2);
    }
  }
  std::cout<<CRTStrips[0].size()<<" "<<CRTStrips[1].size()<<std::endl;
  TH2D *frstp[2];
  TCanvas *cstp[2];
  for (int i = 0; i<2; ++i){
    cstp[i] = new TCanvas(Form("cstp_%d",i), Form("cstp_%d",i), 1000,500);
    double minx = 1e10;
    double maxx = -1e10;
    double miny = 1e10;
    double maxy = -1e10;
    for (size_t j = 0; j<CRTStrips[i].size(); ++j){
      if (minx>CRTStrips[i][j]->GetX1()) minx = CRTStrips[i][j]->GetX1();
      if (minx>CRTStrips[i][j]->GetX2()) minx = CRTStrips[i][j]->GetX2();
      if (maxx<CRTStrips[i][j]->GetX1()) maxx = CRTStrips[i][j]->GetX1();
      if (maxx<CRTStrips[i][j]->GetX2()) maxx = CRTStrips[i][j]->GetX2();
      if (miny>CRTStrips[i][j]->GetY1()) miny = CRTStrips[i][j]->GetY1();
      if (miny>CRTStrips[i][j]->GetY2()) miny = CRTStrips[i][j]->GetY2();
      if (maxy<CRTStrips[i][j]->GetY1()) maxy = CRTStrips[i][j]->GetY1();
      if (maxy<CRTStrips[i][j]->GetY2()) maxy = CRTStrips[i][j]->GetY2();
    }
    frstp[i] = new TH2D(Form("frstp_%d",i),Form("frstp_%d",i),100,minx-10,maxx+10,100,miny-1,maxy+1);
    frstp[i]->SetStats(0);
    if (i==0) frstp[i]->SetTitle("Horizontal module (offline module 0);y(cm);z(cm)");
    else frstp[i]->SetTitle("Vertical module (offline module 2);x(cm);z(cm)");
    frstp[i]->Draw();
    for (size_t j = 0; j<CRTStrips[i].size(); ++j){
      CRTStrips[i][j]->Draw();
    }
    auto& auxdet = geo->AuxDet(i*2);
    for (size_t j = 0; j<auxdet.NSensitiveVolume(); ++j){
      auto& auxdetsen = auxdet.SensitiveVolume(j);
      double xyz[3];
      auxdetsen.GetCenter(xyz);
      if (i==0) tt.DrawLatex(xyz[1]-0.5, xyz[2], Form("%d",int(j)));
      else tt.DrawLatex(xyz[0]-0.5, xyz[2], Form("%d",int(j)));
    }
    cstp[i]->Print(Form("cstp_%d.png",i));
  }
  for (int i = 0; i<32; ++i){
    std::cout<<pixel_x[i]<<" "<<pixel_y[i]<<" "<<pixel_z[i]<<std::endl;
  }
  cancrt->Print("crt.pdf");
  cancrt->Print("crt.png");

}

void dune::CheckGeometry::reconfigure(fhicl::ParameterSet const & p)
{
  // Implementation of optional member function here.
}

DEFINE_ART_MODULE(dune::CheckGeometry)
