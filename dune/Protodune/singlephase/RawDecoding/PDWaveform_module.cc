//December 2017 Reconstruction oF Histograms (jiowang@ucdavis.edu)

#ifndef PDWaveform_module
#define PDWaveform_module

// LArSoft includes
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "lardataobj/Simulation/SimChannel.h"
#include "larsim/Simulation/LArG4Parameters.h"
#include "larcore/Geometry/Geometry.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCTruth.h"
#include "larcoreobj/SimpleTypesAndConstants/geo_types.h"

// Data type includes
#include "lardataobj/RawData/raw.h"
#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RawData/OpDetPulse.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/Cluster.h"
#include "lardataobj/RecoBase/OpHit.h"
#include "lardataobj/RawData/OpDetWaveform.h"
#include "larana/OpticalDetector/OpDigiProperties.h"

// Framework includes
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/ParameterSet.h"

// ROOT includes.
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TFile.h"
#include "TNtuple.h"
#include "TPaveLabel.h"
#include "TPaveText.h"
#include "TFrame.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include "TGraph.h"
#include "TString.h"
#include "TH1I.h"
#include "TH1D.h"
#include "TAxis.h"

// C++ Includes
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

#ifdef __MAKECINT__
#pragma link C++ class vector<vector<int> >+;
#endif

namespace pd_monitor {
  
  class PDWaveform : public art::EDAnalyzer {
    
  public:
    
    explicit PDWaveform(fhicl::ParameterSet const& pset);
    virtual ~PDWaveform();
    
    void setRootObjects();
    
    void beginJob();
    
    void endJob();
    
    void beginRun(const art::Run& run);
    
    void reconfigure(fhicl::ParameterSet const& pset);
    
    void analyze(const art::Event& evt); 
    
  private:
    
    std::string fSSPInput;
    std::string fSSPInstance;
    unsigned int fSSP_m1,fSSP_m2,fSSP_i1,fSSP_i2,fSSP_readout_pretrigger,fSSP_disc_width;
    unsigned int fSSP_wfm_verbose,fPDwaveform_fft;
    unsigned int fSSP_corrchan1, fSSP_corrchan2, fSSP_intwin;


    bool fIsSSP;
    
    art::ServiceHandle<geo::Geometry> fGeom;
    
    TH2F* PDchanMean; // histogram of means of each waveform
    TH2F* PDchanRMS; //  histogram of RMS of each waveform
    TH2F* PDchanRMSwide; // histogram of RMS of each waveform on large scale
    TH2F* PDchanFFT; // cumulative histogram of FFT power from each channel waveform
    TH2F* PDchanPED; // Distribution of ALL ADC convertions in a waveform for each channel
    TH2F* PDchanMax; // Distribution of the max ADC count from each waveform
    TH2F* PDchanMaxPed; // Distribution of max ADC count from each waveform minus the pedestal
    TH2D* PDCalib; // Channel integral calibration at a glance
    TH2D* PDchanCorr; //Correlattion between two channels selected in the fhicl file
    TH2F* PDchanCorPerTrace[288]; //OpDetWaveForms after pedestal subtraction per channel
    TH2F* PDchanRawPerTrace[288]; //OpDetWaveForm raw persistence traces per channel
    TH2F* PDchanPEDRough[288]; //Pedastal value histogram per channel
    TH1D* PDchanWaveInt[288]; //Waveform integrations per channel
    TH1D* PDchanWaveIntPre[288]; //Waveform integrations prebeam per channel
    TH1D* PDchanWaveIntPost[288]; //Waveform integrations postbeam per channel
    TH1I* PDtrigs; // Number of triggers
    TH1F* PDPEDhist; //Calculated Pedestal of Each Channel
    TH1F* PDchanThres; // Calculated Threshold of Each Channel
    // Add PDchanMax,PDchanMin?
    
  }; 
  
  PDWaveform::PDWaveform(fhicl::ParameterSet const& parameterSet)
    : EDAnalyzer(parameterSet){  
    reconfigure(parameterSet);
  }
  
  void PDWaveform::reconfigure(fhicl::ParameterSet const& p) {
    fSSPInput       = p.get< std::string >("SSPInputModule");
    fSSPInstance    = p.get< std::string >("SSPInstanceName");
    
    //Try this, reconfigure fhicl settings
    fSSP_m1=p.get<unsigned int>("SSP_m1");
    fSSP_m2=p.get<unsigned int>("SSP_m2");
    fSSP_i1=p.get<unsigned int>("SSP_i1");
    fSSP_i2=p.get<unsigned int>("SSP_i2");
    fSSP_disc_width=p.get<unsigned int>("SSP_disc_width");
    fSSP_readout_pretrigger=p.get<unsigned int>("SSP_readout_pretrigger");
    fSSP_wfm_verbose=p.get<unsigned int>("SSP_wfm_verbose");
    fPDwaveform_fft=p.get<unsigned int>("PDwaveform_fft");
    fSSP_corrchan1=p.get<unsigned int>("SSP_Corr_Chan_1");
    fSSP_corrchan2=p.get<unsigned int>("SSP_Corr_Chan_2");
    fSSP_intwin=p.get<unsigned int>("SSP_calib_int_win");

    if( fSSP_wfm_verbose ){
      std::cout << " fSSP_m1=" << fSSP_m1 <<std::endl;
      std::cout << " fSSP_m2=" << fSSP_m2 <<std::endl;
      std::cout << " fSSP_i1=" << fSSP_i1 <<std::endl;
      std::cout << " fSSP_i2=" << fSSP_i2 <<std::endl;
      std::cout << " fSSP_disc_width=" << fSSP_disc_width <<std::endl;
      std::cout << " fSSP_readout_pretrigger=" << fSSP_readout_pretrigger <<std::endl;
      std::cout << " fSSP_corrchan1="<< fSSP_corrchan1 << std::endl;
      std::cout << " fSSP_corrchan2="<< fSSP_corrchan2 << std::endl;
      std::cout << " fSSP_intwin="<< fSSP_intwin << std::endl;
    }
  }
  
  PDWaveform::~PDWaveform() {}
  
  void PDWaveform::beginJob() { 
    
    art::ServiceHandle<art::TFileService> tFileService;
   
    PDchanMean = tFileService->make<TH2F>("Mean vs. Channel","Mean vs. Channel",288,0.,288.,1000,1000.,2000.);
    PDchanMax = tFileService->make<TH2F>("Max vs. Channel","Max vs. Channel",288.,0.,288.,2500,1500.,4000.);
    PDchanMaxPed = tFileService->make<TH2F>("Max - Pedestal vs. Channel","Max - Pedestal vs. Channel",288.,0.,288.,2500,0,2500.);
    PDchanPED = tFileService->make<TH2F>("PedVals vs. Channel","PedVals vs. Channel",288,0.,288.,1000,1000.,2000.);
    PDchanRMS = tFileService->make<TH2F>("RMS vs. Channel","RMS vs. Channel",288,0.,288.,100,0.,10.);
    PDchanRMSwide = tFileService->make<TH2F>("Coarse RMS vs. Channel","Coarse RMS vs. Channel",288,0.,288.,100,0.,100.);
    PDchanFFT = tFileService->make<TH2F>("FFTFreq vs. Channel","FFTFreq vs. Channel",288,0.,288.,1000,0.,75.);
    PDCalib = tFileService->make<TH2D>("Integral_cal","Integral Calibration by Channel",288,0,288.,1000,0.0,1000.0);
    PDtrigs = tFileService->make<TH1I>("Triggers vs. Channel","Triggers vs. Channel",288.,0.,288.);
    PDPEDhist = tFileService->make<TH1F>("Pedestal vs. Channel","Pedestal vs. Channel",288.,0.,288.);
    PDchanThres = tFileService->make<TH1F>("Threshold vs. Channel","Threshold vs. Channel",288,0.,288.);
    PDchanCorr = tFileService->make<TH2D>(Form("ADCMax_chan%d_chan%d",fSSP_corrchan1,fSSP_corrchan2),
					  Form("ADCMax Channel %d vs. ADCMax Channel %d",fSSP_corrchan1,fSSP_corrchan2),4000,0.0,4000.0,4000,0.0,4000.0);
    
    for(int i=0;i<288;i++){
      PDchanRawPerTrace[i] = tFileService->make<TH2F>(Form("raw_per_trace_chan_%d",i),
						      Form("Raw Persistence Traces Channel %d",i),2000.,0.,2000.,1000.,1350.,4000.); 
      PDchanCorPerTrace[i] = tFileService->make<TH2F>(Form("cor_per_trace_chan_%d",i),
						      Form("Pedestal Substracted Persistence Traces Channel %d",i),2000.,0.,2000.,1000.,0.,2000.); 
      PDchanPEDRough[i] = tFileService->make<TH2F>(Form("ped_calc_trace_chan_%d",i),
						   Form("Wave Form Fraction for Pedestal %d",i),40.,0.,40.,1000.,1500.,2500.); 
      PDchanWaveInt[i] = tFileService->make<TH1D>(Form("wave_intgerals_pedsub_chan_%d",i),
						  Form("Pedestal Subtracted Wave Integrals Channel %d",i),10000,0.0,100000.0); 
      PDchanWaveIntPre[i] = tFileService->make<TH1D>(Form("pre_wave_intgerals_pedsub_chan_%d",i),
						     Form("Pedestal Subtracted Pre-Beam Wave Integrals Channel %d",i),1000,0.0,1000.0); 
      PDchanWaveIntPost[i] = tFileService->make<TH1D>(Form("post_wave_intgerals_pedsub_chan_%d",i),
						      Form("Pedestal Subtracted Post-Beam Wave Integrals Channel %d",i),10000,0.0,100000.0); 
    }
  }
  
  void PDWaveform::endJob() { 
    std::cout << "Finalizing Histograms." << std::endl;
    for(int i=0;i<288;i++) {
      PDPEDhist->SetBinContent(i+1,PDchanPEDRough[i]->ProjectionY()->GetMean());
      PDchanThres->SetBinContent(i,PDchanMaxPed->ProjectionY("",i,i)->GetXaxis()->GetBinLowEdge(PDchanMaxPed->ProjectionY("",i,i)->GetMaximumBin())); 
    }
  }
  
  void pd_monitor::PDWaveform::setRootObjects() { }
  
  void PDWaveform::beginRun(const art::Run& run) { }

  void PDWaveform::analyze(const art::Event& event) {

    LOG_INFO("PDWaveform")
      << "-------------------- Photodetector waveforms -------------------";
    
    // Get the data with the correct label and instance from the root file	
    art::Handle< std::vector<raw::OpDetWaveform> > RawSSP;
    event.getByLabel("ssprawdecoder", "daq", RawSSP);

    
    // Make sure data is collected
    try { RawSSP->size(); }
    catch(std::exception e) { fIsSSP = false; }
    fIsSSP=true;

    // Put data into a vector of pointers
    std::vector< art::Ptr<raw::OpDetWaveform> > RawPulses;
    if (fIsSSP) { art::fill_ptr_vector(RawPulses, RawSSP); }
    
    //channel correlation variables
    double tschan1=0.0, tschan2=0.0, ampchan1=0.0,ampchan2=0.0;
    
    // Loop over waveforms
    for(auto const & RawPulse : RawPulses) {
      
      // Get waveform from pointer
      const raw::OpDetWaveform & PDdigit = *RawPulse;
      unsigned int CurChannel = PDdigit.ChannelNumber();
      PDtrigs->Fill(CurChannel);
      
      // Loop through individual waveform and print ADC's at each position
      long int ADCval,sum=0,sum2=0,N=0,ADCMax=0;
      double sumthres=0,sumpedsub=0, sumpresub=0,sumpostsub=0;;
      int nBins = PDdigit.size();
      TH1F WfmHist("Waveform","Waveform",nBins,0,nBins), 
	WfmFFT("WfmFFT","WfmFFT",nBins,0,nBins);
      unsigned int intcount = 0;
      for (size_t i = 0; i < PDdigit.size(); i++) {
	ADCval = PDdigit.at(i);
	PDchanRawPerTrace[CurChannel]->Fill(i+1,ADCval);
	PDchanCorPerTrace[CurChannel]->Fill(i+1,ADCval);
	if(i < fSSP_i1) {
	  PDchanPEDRough[CurChannel]->Fill(i+1,ADCval);
	  sumthres+=ADCval;
	}
	  
	ADCMax=std::max(ADCMax,ADCval);
	N=N+1;

	WfmHist.SetBinContent(i+1,ADCval);
	sum+=ADCval;
	sum2+=ADCval*ADCval;
	PDchanPED->Fill(CurChannel,ADCval); 
	if(i > fSSP_i1) sumpedsub+=(ADCval-(sumthres/static_cast<float>(fSSP_i1)));
	if(i > fSSP_i1 && i < 800){
	  if(ADCMax==ADCval) {
	    sumpresub=ADCval-(sumthres/static_cast<float>(fSSP_i1));
	    intcount=i;
	  }
	  if(i-intcount < fSSP_intwin) sumpresub+=(ADCval-(sumthres/static_cast<float>(fSSP_i1)));
	  if(i-intcount == fSSP_intwin) {
	    PDchanWaveIntPre[CurChannel]->Fill(sumpresub);
	    PDCalib->Fill(CurChannel,sumpresub);
	  }
	}
	if(i > 800) sumpostsub+=(ADCval-(sumthres/static_cast<float>(fSSP_i1)));

      }
      float mean = (float)sum/(float)N;
      float rms = sqrt((float)sum2/(float)N - mean*mean );
      //std::cout <<"ADC Sum: " << sum << std::endl;
      PDchanWaveInt[CurChannel]->Fill(sumpedsub);
      PDchanWaveIntPost[CurChannel]->Fill(sumpostsub);
      PDchanMean->Fill(CurChannel,mean);
      PDchanMax->Fill(CurChannel,ADCMax);
      float thres = sumthres/static_cast<float>(fSSP_i1);
      PDchanMaxPed->Fill(CurChannel,ADCMax-thres);
      //PDchanMin->Fill(CurChannel,ADCMin);
      PDchanRMS->Fill(CurChannel,rms);
      PDchanRMSwide->Fill(CurChannel,rms);
      
      if (event.event()%1==0){  // only do FFT on 100% of events (or average them)
	WfmHist.FFT(&WfmFFT,"MAG");
	WfmFFT.Scale(1.0/(float)N);
	for (int freqBin = 2 ; freqBin < nBins/2 ; freqBin++){
	  // Just set the value
	  //PDchanFFT->SetBinContent(CurChannel,freqBin,WfmFFT.GetBinContent(freqBin));
	  PDchanFFT->Fill(CurChannel,((float)freqBin+0.5)*75.0/1000.0,WfmFFT.GetBinContent(freqBin));
	}
      }
    
      if(ADCMax-thres > 100.0){
	if(CurChannel == fSSP_corrchan1){
	  tschan1=PDdigit.TimeStamp();
	  ampchan1=(ADCMax-thres);
	}
	if(CurChannel == fSSP_corrchan2){
	  tschan2=PDdigit.TimeStamp();
	  ampchan2=(ADCMax-thres);
	}
	if(tschan1 != 0.0 && tschan2 != 0.0 && (abs(tschan1-tschan2) < 100.0E-9)){
	  PDchanCorr->Fill(ampchan1,ampchan2);
	  tschan1=0.0;
	  tschan2=0.0;
	  ampchan1=0.0;
	  ampchan2=0.0;
	}
      }
    }
 
    return;

  }
  
  DEFINE_ART_MODULE(pd_monitor::PDWaveform)
  
} // namespace

#endif // PDonlinemonitor_module
