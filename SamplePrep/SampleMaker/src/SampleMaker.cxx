#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <cassert>
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "ObjectFilters.cxx"
#include "TFile.h"
#include "TChain.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TString.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/Muon.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
#include "CaloGeoHelpers/CaloSampling.h"

#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/Jet.h"

using namespace std;

int main (int argc, char *argv[]) {
    //--- Open input files - all argv after argv[0] are file names
    std::vector<std::string> inputFileNames;
    for (int i=1; i < argc; i++) inputFileNames.push_back(argv[i]);

    cout << "\nReading input files" << endl;
    TChain* inputFileChain = new TChain("CollectionTree");
    for (unsigned int iFile=0; iFile<inputFileNames.size(); ++iFile)
    {
        cout << "Opened file: " << inputFileNames[iFile].c_str() << endl;
        inputFileChain->Add(inputFileNames[iFile].c_str());
    }

    //--- Whether or not to filter out a lepton's own tracks
    bool filter_own_tracks = true;

    //--- Connect the event object to read from input files
    const char* ALG = argv[0];
    RETURN_CHECK(ALG, xAOD::Init());
    xAOD::TEvent event(xAOD::TEvent::kClassAccess);
    RETURN_CHECK(ALG, event.readFrom(inputFileChain));

    //--- Create branches in unnormalized tree
    TFile outputFile("output.root", "recreate");
    TTree* unnormedTree = new TTree("UnnormedTree", "unnormalized tree");

    int entry_n; unnormedTree->Branch("event_n", &entry_n, "event_n/I");
    int pdgID; unnormedTree->Branch("pdgID", &pdgID, "pdgID/I");
    int truth_type; unnormedTree->Branch("truth_type", &truth_type, "truth_type/I");

    float ptcone20; unnormedTree->Branch("baseline_ptcone20", &ptcone20, "baseline_ptcone20/F");
    float ptcone30; unnormedTree->Branch("baseline_ptcone30", &ptcone30, "baseline_ptcone30/F");
    float ptcone40; unnormedTree->Branch("baseline_ptcone40", &ptcone40, "baseline_ptcone40/F");
    float ptvarcone20; unnormedTree->Branch("baseline_ptvarcone20", &ptvarcone20, "baseline_ptvarcone20/F");
    float ptvarcone30; unnormedTree->Branch("baseline_ptvarcone30", &ptvarcone30, "baseline_ptvarcone30/F");
    float ptvarcone40; unnormedTree->Branch("baseline_ptvarcone40", &ptvarcone40, "baseline_ptvarcone40/F");
    float topoetcone20; unnormedTree->Branch("baseline_topoetcone20", &topoetcone20, "baseline_topoetcone20/F");
    float topoetcone30; unnormedTree->Branch("baseline_topoetcone30", &topoetcone30, "baseline_topoetcone30/F");
    float topoetcone40; unnormedTree->Branch("baseline_topoetcone40", &topoetcone40, "baseline_topoetcone40/F");
    float eflowcone20; unnormedTree->Branch("baseline_eflowcone20", &eflowcone20, "baseline_eflowcone20/F");
    float ptcone20_over_pt; unnormedTree->Branch("baseline_ptcone20_over_pt", &ptcone20_over_pt, "baseline_ptcone20_over_pt/F");
    float ptcone30_over_pt; unnormedTree->Branch("baseline_ptcone30_over_pt", &ptcone30_over_pt, "baseline_ptcone30_over_pt/F");
    float ptcone40_over_pt; unnormedTree->Branch("baseline_ptcone40_over_pt", &ptcone40_over_pt, "baseline_ptcone40_over_pt/F");
    float ptvarcone20_over_pt; unnormedTree->Branch("baseline_ptvarcone20_over_pt", &ptvarcone20_over_pt, "baseline_ptvarcone20_over_pt/F");
    float ptvarcone30_over_pt; unnormedTree->Branch("baseline_ptvarcone30_over_pt", &ptvarcone30_over_pt, "baseline_ptvarcone30_over_pt/F");
    float ptvarcone40_over_pt; unnormedTree->Branch("baseline_ptvarcone40_over_pt", &ptvarcone40_over_pt, "baseline_ptvarcone40_over_pt/F");
    float topoetcone20_over_pt; unnormedTree->Branch("baseline_topoetcone20_over_pt", &topoetcone20_over_pt, "baseline_topoetcone20_over_pt/F");
    float topoetcone30_over_pt; unnormedTree->Branch("baseline_topoetcone30_over_pt", &topoetcone30_over_pt, "baseline_topoetcone30_over_pt/F");
    float topoetcone40_over_pt; unnormedTree->Branch("baseline_topoetcone40_over_pt", &topoetcone40_over_pt, "baseline_topoetcone40_over_pt/F");
    float eflowcone20_over_pt; unnormedTree->Branch("baseline_eflowcone20_over_pt", &eflowcone20_over_pt, "baseline_eflowcone20_over_pt/F");
    //float PLT; unnormedTree->Branch("baseline_PLT", &PLT, "baseline_PLT/F");

	float calc_ptcone20; unnormedTree->Branch("calc_ptcone20", &calc_ptcone20, "calc_ptcone20/F");
    float calc_ptcone30; unnormedTree->Branch("calc_ptcone30", &calc_ptcone30, "calc_ptcone30/F");
    float calc_ptcone40; unnormedTree->Branch("calc_ptcone40", &calc_ptcone40, "calc_ptcone40/F");
    float calc_ptvarcone20; unnormedTree->Branch("calc_ptvarcone20", &calc_ptvarcone20, "calc_ptvarcone20/F");
    float calc_ptvarcone30; unnormedTree->Branch("calc_ptvarcone30", &calc_ptvarcone30, "calc_ptvarcone30/F");
    float calc_ptvarcone40; unnormedTree->Branch("calc_ptvarcone40", &calc_ptvarcone40, "calc_ptvarcone40/F");



    float lep_pT; unnormedTree->Branch("lep_pT", &lep_pT, "lep_pT/F");
    unnormedTree->Branch("ROC_slicing_lep_pT", &lep_pT, "ROC_slicing_lep_pT/F");  // for making ROC curve plots
    float lep_eta; unnormedTree->Branch("lep_eta", &lep_eta, "lep_eta/F");
    float lep_theta; unnormedTree->Branch("lep_theta", &lep_theta, "lep_theta/F");
    float lep_phi; unnormedTree->Branch("lep_phi", &lep_phi, "lep_phi/F");
    float lep_d0; unnormedTree->Branch("lep_d0", &lep_d0, "lep_d0/F");
    float lep_d0_over_sigd0; unnormedTree->Branch("lep_d0_over_sigd0", &lep_d0_over_sigd0, "lep_d0_over_sigd0/F");
    float lep_z0; unnormedTree->Branch("lep_z0", &lep_z0, "lep_z0/F");
    float lep_dz0; unnormedTree->Branch("lep_dz0", &lep_dz0, "lep_dz0/F");
    int lep_has_associated_jet; unnormedTree->Branch("lep_has_associated_jet", &lep_has_associated_jet, "lep_has_associated_jet/I");
    float lep_DL1r; unnormedTree->Branch("lep_DL1r", &lep_DL1r, "lep_DL1r/F");

    vector<float>* trk_lep_dR = new vector<float>; unnormedTree->Branch("trk_lep_dR", "vector<float>", &trk_lep_dR);
    vector<float>* trk_pT = new vector<float>; unnormedTree->Branch("trk_pT", "vector<float>", &trk_pT);
    vector<float>* trk_eta = new vector<float>; unnormedTree->Branch("trk_eta", "vector<float>", &trk_eta);
    vector<float>* trk_phi = new vector<float>; unnormedTree->Branch("trk_phi", "vector<float>", &trk_phi);
    vector<float>* trk_d0 = new vector<float>; unnormedTree->Branch("trk_d0", "vector<float>", &trk_d0);
    vector<float>* trk_z0 = new vector<float>; unnormedTree->Branch("trk_z0", "vector<float>", &trk_z0);
    vector<float>* trk_lep_dEta = new vector<float>; unnormedTree->Branch("trk_lep_dEta", "vector<float>", &trk_lep_dEta);
    vector<float>* trk_lep_dPhi = new vector<float>; unnormedTree->Branch("trk_lep_dPhi", "vector<float>", &trk_lep_dPhi);
    vector<float>* trk_lep_dD0 = new vector<float>; unnormedTree->Branch("trk_lep_dD0", "vector<float>", &trk_lep_dD0);
    vector<float>* trk_lep_dZ0 = new vector<float>; unnormedTree->Branch("trk_lep_dZ0", "vector<float>", &trk_lep_dZ0);
    vector<float>* trk_chi2 = new vector<float>; unnormedTree->Branch("trk_chi2", "vector<float>", &trk_chi2);
    vector<int>* trk_charge = new vector<int>; unnormedTree->Branch("trk_charge", "vector<int>", &trk_charge);
    vector<int>* trk_nIBLHits = new vector<int>; unnormedTree->Branch("trk_nIBLHits", "vector<int>", &trk_nIBLHits);
    vector<int>* trk_nPixHits = new vector<int>; unnormedTree->Branch("trk_nPixHits", "vector<int>", &trk_nPixHits);
    vector<int>* trk_nPixHoles = new vector<int>; unnormedTree->Branch("trk_nPixHoles", "vector<int>", &trk_nPixHoles);
    vector<int>* trk_nPixOutliers = new vector<int>; unnormedTree->Branch("trk_nPixOutliers", "vector<int>", &trk_nPixOutliers);
    vector<int>* trk_nSCTHits = new vector<int>; unnormedTree->Branch("trk_nSCTHits", "vector<int>", &trk_nSCTHits);
    vector<int>* trk_nSCTHoles = new vector<int>; unnormedTree->Branch("trk_nSCTHoles", "vector<int>", &trk_nSCTHoles);
    vector<int>* trk_nTRTHits = new vector<int>; unnormedTree->Branch("trk_nTRTHits", "vector<int>", &trk_nTRTHits);
    vector<float>* trk_vtx_x = new vector<float>; unnormedTree->Branch("trk_vtx_x", "vector<float>", &trk_vtx_x);
    vector<float>* trk_vtx_y = new vector<float>; unnormedTree->Branch("trk_vtx_y", "vector<float>", &trk_vtx_y);
    vector<float>* trk_vtx_z = new vector<float>; unnormedTree->Branch("trk_vtx_z", "vector<float>", &trk_vtx_z);
    vector<int>* trk_vtx_type = new vector<int>; unnormedTree->Branch("trk_vtx_type", "vector<int>", &trk_vtx_type);

    // See https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaD3PDtoxAOD
    vector<float>* calo_cluster_lep_dR = new vector<float>; unnormedTree->Branch("calo_cluster_lep_dR", "vector<float>", &calo_cluster_lep_dR);
    vector<float>* calo_cluster_e = new vector<float>; unnormedTree->Branch("calo_cluster_e", "vector<float>", &calo_cluster_e);
    vector<float>* calo_cluster_pT = new vector<float>; unnormedTree->Branch("calo_cluster_pT", "vector<float>", &calo_cluster_pT);
    vector<float>* calo_cluster_eta = new vector<float>; unnormedTree->Branch("calo_cluster_eta", "vector<float>", &calo_cluster_eta);
    vector<float>* calo_cluster_phi = new vector<float>; unnormedTree->Branch("calo_cluster_phi", "vector<float>", &calo_cluster_phi);
    //vector<float>* calo_cluster_etamoment = new vector<float>; unnormedTree->Branch("calo_cluster_etamoment", "vector<float>", &calo_cluster_etamoment);
    //vector<float>* calo_cluster_phimoment = new vector<float>; unnormedTree->Branch("calo_cluster_phimoment", "vector<float>", &calo_cluster_phimoment);
    vector<float>* calo_cluster_lep_dEta = new vector<float>; unnormedTree->Branch("calo_cluster_lep_dEta", "vector<float>", &calo_cluster_lep_dEta);
    vector<float>* calo_cluster_lep_dPhi = new vector<float>; unnormedTree->Branch("calo_cluster_lep_dPhi", "vector<float>", &calo_cluster_lep_dPhi);

    //--- Cutflow table [HF_electron/isolated_electron/HF_muon/isolated_muon][truth_type/medium/impact_params/isolation]
    int cutflow_table[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

    auto update_cutflow = [&] (vector<pair<const xAOD::Electron*, int>> electrons, vector<pair<const xAOD::Muon*, int>> muons, int stage) {
        //cout << "Stage " << stage << " " << electrons.size() << " " << muons.size() << endl;
        for (auto electron_info : electrons) {
            truth_type = electron_info.second;
            int is_isolated = (truth_type == 2);
            cutflow_table[is_isolated][stage]++;
        }
        for (auto muon_info : muons) {
            truth_type = muon_info.second;
            int is_isolated = (truth_type == 6);
            cutflow_table[2+is_isolated][stage]++;
        }
    };

    auto print_cutflow = [&] () {
        cout << "Printing cutflow table:" << endl;
        for (int i=0; i<4; i++) {
            cout << cutflow_table[i][0] << " " << cutflow_table[i][1] << " " << cutflow_table[i][2] << " " << cutflow_table[i][3] << endl;
        }
    };
    //TODO: uncomment if needed. 
    //--- Accessors
    //SG::AuxElement::ConstAccessor<float> accessPromptVar("PromptLeptonVeto");

    std::string FlvTagCutDefinitionsFileName = "/eos/atlas/atlascerngroupdisk/asg-calib/xAODBTaggingEfficiency/13TeV/2019-21-13TeV-MC16-CDI-2019-10-07_v1.root";
    std::string WP = "FixedCutBEff_77";
    BTaggingSelectionTool *m_bTagSel_DL1r = new BTaggingSelectionTool("BTagSel_DL1r");
    m_bTagSel_DL1r->setProperty( "FlvTagCutDefinitionsFileName", FlvTagCutDefinitionsFileName);
    m_bTagSel_DL1r->setProperty("TaggerName", "DL1");
    m_bTagSel_DL1r->setProperty("OperatingPoint", WP);
    m_bTagSel_DL1r->setProperty("JetAuthor", "AntiKt4EMPFlowJets_BTagging201903");
    if (!m_bTagSel_DL1r->initialize()) {
        cout << "BTagging tool could not initialize" << endl;
        exit (EXIT_FAILURE);
    }

    auto process_lepton = [&] (const xAOD::IParticle* lepton, const xAOD::Vertex *primary_vertex, const vector<const xAOD::TrackParticle*> &filtered_tracks, vector<const xAOD::CaloCluster*> &filtered_calo_clusters, vector<const xAOD::Jet*> &filtered_jets, bool is_electron) {
        //--- Get lepton's associated track particle
        const xAOD::TrackParticle* track_particle; 
        if (is_electron) track_particle = ((xAOD::Electron*)lepton)->trackParticle(); 
        else track_particle = ((xAOD::Muon*)lepton)->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);

        //--- Functions to retrieve ptcone and etcone variables
        auto process_electron_cones = [&] (const xAOD::Electron* electron) {
            electron->isolation(ptcone20,xAOD::Iso::ptcone20_TightTTVA_pt1000);
            ptcone30 = numeric_limits<float>::quiet_NaN();
            ptcone40 = numeric_limits<float>::quiet_NaN();
            electron->isolation(ptvarcone20,xAOD::Iso::ptvarcone20);
            electron->isolation(ptvarcone30,xAOD::Iso::ptvarcone30_TightTTVA_pt1000);
            electron->isolation(ptvarcone40,xAOD::Iso::ptvarcone40);
            electron->isolation(topoetcone20,xAOD::Iso::topoetcone20);
            topoetcone30 = numeric_limits<float>::quiet_NaN();
            electron->isolation(topoetcone40,xAOD::Iso::topoetcone40);
            electron->isolation(eflowcone20,xAOD::Iso::neflowisol20);
            // topocluster stuff - using https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/IsolationManualCalculation
        };

        auto process_muon_cones = [&] (const xAOD::Muon* muon) {
            muon->isolation(ptcone20,xAOD::Iso::ptcone20);
            muon->isolation(ptcone30,xAOD::Iso::ptcone30);
            muon->isolation(ptcone40,xAOD::Iso::ptcone40);
            muon->isolation(ptvarcone20,xAOD::Iso::ptvarcone20);
            muon->isolation(ptvarcone30,xAOD::Iso::ptvarcone30);
            muon->isolation(ptvarcone40,xAOD::Iso::ptvarcone40);
            muon->isolation(topoetcone20,xAOD::Iso::topoetcone20);
            muon->isolation(topoetcone30,xAOD::Iso::topoetcone30);
            muon->isolation(topoetcone40,xAOD::Iso::topoetcone40);
            muon->isolation(eflowcone20,xAOD::Iso::neflowisol20);
        };
		
        //--- Retrieve all relevant lepton variables
        lep_pT = lepton->pt();
        lep_eta = lepton->eta();
        lep_theta = 2 * atan(exp(-lep_eta));
        lep_phi = lepton->phi();
        lep_d0 = track_particle->d0();
        lep_d0_over_sigd0 = xAOD::TrackingHelpers::d0significance(track_particle);
        lep_z0 = track_particle->z0();
        lep_dz0 = track_particle->z0() - primary_vertex->z();
        // PLT = accessPromptVar(*lepton);
        if (is_electron) process_electron_cones((const xAOD::Electron*)lepton);
        else process_muon_cones((const xAOD::Muon*)lepton);

        ptcone20_over_pt = ptcone20 / lep_pT;
        ptcone30_over_pt = ptcone30 / lep_pT;
        ptcone40_over_pt = ptcone40 / lep_pT;
        ptvarcone20_over_pt = ptvarcone20 / lep_pT;
        ptvarcone30_over_pt = ptvarcone30 / lep_pT;
        ptvarcone40_over_pt = ptvarcone40 / lep_pT;
        topoetcone20_over_pt = topoetcone20 / lep_pT;
        topoetcone30_over_pt = topoetcone30 / lep_pT;
        topoetcone40_over_pt = topoetcone40 / lep_pT;
        eflowcone20_over_pt = eflowcone20 / lep_pT;

        //--- Find nearest jet and get b-tagging variable
        lep_has_associated_jet = 0;
        float nearest_dR = -1;
        const xAOD::Jet *nearest_jet = NULL;
        for (auto jet : filtered_jets) {
            float jet_dR = jet->p4().DeltaR(lepton->p4());
            if (jet_dR > 0.4) continue;
            if (nearest_dR == -1 || jet_dR < nearest_dR) {
                lep_has_associated_jet = 1;
                nearest_jet = jet;
                nearest_dR = jet_dR;
            }
        }

        if (lep_has_associated_jet) {
            double lep_DL1r_double = 0.0;
            m_bTagSel_DL1r->getTaggerWeight(*nearest_jet, lep_DL1r_double);
            lep_DL1r = lep_DL1r_double;
        }
        else {
            lep_DL1r = 0.0;
        }

        //--- Check if lepton passes cuts
        bool dz0_cut = fabs(lep_dz0 * sin(lep_theta)) < 0.5;
        bool d0_over_sigd0_cut = (is_electron and (fabs(lep_d0_over_sigd0) < 5)) or (!is_electron and (fabs(lep_d0_over_sigd0) < 3));
        bool passes_cuts = (dz0_cut and d0_over_sigd0_cut);
        if (!passes_cuts) return false;

        //--- Store tracks associated to lepton in dR cone
        float max_dR = 0.5;

        auto get_electron_own_tracks = [&] (const xAOD::Electron* electron) {
            set<const xAOD::TrackParticle*> electron_tracks = xAOD::EgammaHelpers::getTrackParticles((const xAOD::Egamma*)electron, true);
            return electron_tracks;
        };

        auto get_muon_own_tracks = [&] (const xAOD::Muon* muon) {
            xAOD::Muon::TrackParticleType type = xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle;
            auto own_track = muon->trackParticle(type);
            set<const xAOD::TrackParticle*> muon_tracks {own_track};
            return muon_tracks;
        };

        trk_lep_dR->clear(); trk_pT->clear(); trk_eta->clear(); trk_phi->clear();
        trk_d0->clear(); trk_z0->clear(); trk_charge->clear(); trk_chi2->clear();
        trk_lep_dEta->clear(); trk_lep_dPhi->clear(); trk_lep_dD0->clear(); trk_lep_dZ0->clear();
        trk_nIBLHits->clear(); trk_nPixHits->clear(); trk_nPixHoles->clear(); trk_nPixOutliers->clear();
        trk_nSCTHits->clear(); trk_nSCTHoles->clear(); trk_nTRTHits->clear();
        trk_vtx_x->clear(); trk_vtx_y->clear(); trk_vtx_z->clear(); trk_vtx_type->clear();
        set<const xAOD::TrackParticle*> own_tracks;
        if (is_electron) own_tracks = get_electron_own_tracks((const xAOD::Electron*)lepton);
        else own_tracks = get_muon_own_tracks((const xAOD::Muon*)lepton);
		
		calc_ptcone20 = 0; calc_ptcone30 = 0; calc_ptcone40 = 0; calc_ptvarcone20 = 0; calc_ptvarcone30 = 0; calc_ptvarcone40 = 0;
		float var_R_20 = std::min(10e3/lepton->pt(), 0.20); 
		float var_R_30 = std::min(10e3/lepton->pt(), 0.30); 
		float var_R_40 = std::min(10e3/lepton->pt(), 0.40);

		if (is_electron){
			std::set<const xAOD::TrackParticle*> electron_tracks = xAOD::EgammaHelpers::getTrackParticles((const xAOD::Egamma*)lepton, true);
		cout<<__LINE__<<endl;
			for (auto trk : filtered_tracks) {
			    if (!trk) continue;
			    bool matches_own_track = false;
			    for (auto own_track : electron_tracks)
			        if (trk == own_track) matches_own_track = true;
			    if (matches_own_track) continue;
			    if (trk->vertex() && trk->vertex()!=primary_vertex) continue;
			    if (trk->p4().DeltaR(lepton->p4()) < 0.20) calc_ptcone20 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < 0.30) calc_ptcone30 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < 0.40) calc_ptcone40 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < var_R_20) calc_ptvarcone20 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < var_R_30) calc_ptvarcone30 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < var_R_40) calc_ptvarcone40 += trk->pt();
			}
		}else{
		cout<<__LINE__<<endl;
			xAOD::Muon::TrackParticleType type = xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle;
			auto own_track = ((xAOD::Muon*)lepton)->trackParticle(type);
			for (auto trk : filtered_tracks) {
			    if (!trk) continue;
			    if (trk == own_track) continue;
			    if (trk->p4().DeltaR(lepton->p4()) < 0.20) calc_ptcone20 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < 0.30) calc_ptcone30 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < 0.40) calc_ptcone40 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < var_R_20) calc_ptvarcone20 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < var_R_30) calc_ptvarcone30 += trk->pt();
			    if (trk->p4().DeltaR(lepton->p4()) < var_R_40) calc_ptvarcone40 += trk->pt();
			}

		}

        bool has_associated_tracks = false;
        for (auto track : filtered_tracks) {
            if (!track->vertex() or track->vertex()!=primary_vertex) continue;
            bool matches_own_track = false;
            for (auto own_track : own_tracks)
                if (track == own_track) matches_own_track = true;
            if (filter_own_tracks && matches_own_track) continue;
            float dR = track->p4().DeltaR(lepton->p4());
            if (dR > max_dR) continue; 

            if (!matches_own_track) has_associated_tracks = true;  // don't count the lepton's own tracks

            trk_lep_dR->push_back(dR);
            trk_pT->push_back(track->pt());
            trk_eta->push_back(track->eta());
            trk_phi->push_back(track->phi());
            trk_d0->push_back(track->d0());
            trk_z0->push_back(track->z0());
            trk_charge->push_back(track->charge());
            trk_chi2->push_back(track->chiSquared());

            trk_lep_dEta->push_back(track->eta() - lep_eta);
            trk_lep_dPhi->push_back(track->phi() - lep_phi);
            trk_lep_dD0->push_back(track->d0() - lep_d0);
            trk_lep_dZ0->push_back(track->z0() - lep_z0);

            uint8_t placeholder;
            track->summaryValue(placeholder, xAOD::numberOfInnermostPixelLayerHits); trk_nIBLHits->push_back(placeholder);
            track->summaryValue(placeholder, xAOD::numberOfPixelHits); trk_nPixHits->push_back(placeholder);
            track->summaryValue(placeholder, xAOD::numberOfPixelHoles); trk_nPixHoles->push_back(placeholder);
            track->summaryValue(placeholder, xAOD::numberOfPixelOutliers); trk_nPixOutliers->push_back(placeholder);
            track->summaryValue(placeholder, xAOD::numberOfSCTHits); trk_nSCTHits->push_back(placeholder);
            track->summaryValue(placeholder, xAOD::numberOfSCTHoles); trk_nSCTHoles->push_back(placeholder);
            track->summaryValue(placeholder, xAOD::numberOfTRTHits); trk_nTRTHits->push_back(placeholder);

            auto vertexLink = track->vertexLink();
            if (vertexLink != 0) {
                auto vertex = *vertexLink;
                trk_vtx_x->push_back(vertex->x());
                trk_vtx_y->push_back(vertex->y());
                trk_vtx_z->push_back(vertex->z());
                trk_vtx_type->push_back(vertex->vertexType());
            }
            else {
                trk_vtx_x->push_back(0);
                trk_vtx_y->push_back(0);
                trk_vtx_z->push_back(0);
                trk_vtx_type->push_back(-1);
            }
        }

        //--- Store calo clusters associated to lepton in dR cone
        calo_cluster_lep_dR->clear(); calo_cluster_e->clear(); calo_cluster_pT->clear(); calo_cluster_eta->clear(); calo_cluster_phi->clear();
        /*calo_cluster_etamoment->clear(); calo_cluster_phimoment->clear();*/ calo_cluster_lep_dEta->clear(); calo_cluster_lep_dPhi->clear();

        for (auto calo_cluster : filtered_calo_clusters) {
            float dR = calo_cluster->p4().DeltaR(lepton->p4());
            if (dR > max_dR) continue; 

            calo_cluster_lep_dR->push_back(dR);
            calo_cluster_e->push_back(calo_cluster->e());
            calo_cluster_pT->push_back(calo_cluster->pt());
            calo_cluster_eta->push_back(calo_cluster->eta());
            calo_cluster_phi->push_back(calo_cluster->phi());
            //calo_cluster_etamoment->push_back(calo_cluster->getMomentValue(xAOD::CaloCluster::ETA1CALOFRAME));
            //calo_cluster_phimoment->push_back(calo_cluster->getMomentValue(xAOD::CaloCluster::PHI1CALOFRAME));

            calo_cluster_lep_dEta->push_back(calo_cluster->eta() - lep_eta);
            calo_cluster_lep_dPhi->push_back(calo_cluster->phi() - lep_phi);
        }

        //--- Remove leptons with no associated tracks
        return has_associated_tracks;
    };

    //--- Loop over events
    int entries = event.getEntries();
    cout << "Retrieved " << entries << " events" << endl;
    //entries = 1000;
    cout << "\nProcessing leptons" << endl;

    ObjectFilters object_filters;

    for (entry_n = 0; entry_n < entries; ++entry_n) {
        //--- Get event
        if (entry_n%5 == 0) cout << "Processing event " << entry_n << "/" << entries << "\n";
        event.getEntry(entry_n);
		cout<<__LINE__<<endl;
        //--- Get event objects
        const xAOD::TrackParticleContainer *tracks;
        const xAOD::VertexContainer *primary_vertices;
        const xAOD::ElectronContainer *electrons;
        const xAOD::MuonContainer *muons;
        const xAOD::CaloClusterContainer *calo_clusters;
        const xAOD::JetContainer *jets;

		cout<<__LINE__<<endl;
        RETURN_CHECK(ALG, event.retrieve(tracks, "InDetTrackParticles"));
        RETURN_CHECK(ALG, event.retrieve(primary_vertices, "PrimaryVertices"));
        RETURN_CHECK(ALG, event.retrieve(electrons, "Electrons"));
        RETURN_CHECK(ALG, event.retrieve(muons, "Muons"));
        RETURN_CHECK(ALG, event.retrieve(calo_clusters, "CaloCalTopoClusters"));
        RETURN_CHECK(ALG, event.retrieve(jets, "AntiKt4EMTopoJets"));

		cout<<__LINE__<<endl;
        const xAOD::Vertex *primary_vertex = primary_vertices->at(0);

		cout<<__LINE__<<endl;
        //--- Filter objects
        vector<const xAOD::TrackParticle*> filtered_tracks = object_filters.filter_tracks(tracks, primary_vertex);
        vector<pair<const xAOD::Electron*, int>> filtered_electrons = object_filters.filter_electrons_truth_type(electrons);
        vector<pair<const xAOD::Muon*, int>> filtered_muons = object_filters.filter_muons_truth_type(muons);
        update_cutflow(filtered_electrons, filtered_muons, 0);
        filtered_electrons = object_filters.filter_electrons_medium(filtered_electrons);
        filtered_muons = object_filters.filter_muons_tight(filtered_muons);
        update_cutflow(filtered_electrons, filtered_muons, 1);
        vector<const xAOD::CaloCluster*> filtered_calo_clusters = object_filters.filter_calo_clusters(calo_clusters);
        vector<const xAOD::Jet*> filtered_jets = object_filters.filter_jets(jets);

		cout<<__LINE__<<endl;
        //--- Write event
        vector<pair<const xAOD::Electron*, int>> new_filtered_electrons;
        vector<pair<const xAOD::Muon*, int>> new_filtered_muons;
        for (auto electron_info : filtered_electrons) {
            const xAOD::Electron* electron = electron_info.first;
            truth_type = electron_info.second;
            pdgID = 11;
		cout<<__LINE__<<endl;
            if (!process_lepton(electron, primary_vertex, filtered_tracks, filtered_calo_clusters, filtered_jets, true)) continue;
		cout<<__LINE__<<endl;
            new_filtered_electrons.push_back(electron_info);
            unnormedTree->Fill();
        }
        for (auto muon_info : filtered_muons) {
            const xAOD::Muon* muon = muon_info.first;
            truth_type = muon_info.second;
            pdgID = 13;
		cout<<__LINE__<<endl;
            if (!process_lepton(muon, primary_vertex, filtered_tracks, filtered_calo_clusters, filtered_jets, false)) continue;
		cout<<__LINE__<<endl;
            new_filtered_muons.push_back(muon_info);
            unnormedTree->Fill();
        }
		cout<<__LINE__<<endl;
        update_cutflow(new_filtered_electrons, new_filtered_muons, 2);
		cout<<__LINE__<<endl;

        //--- Additional cutflow step for comparison - what does a ptvarcone40/lep_pT cut identify as isolated?
        vector<pair<const xAOD::Electron*, int>> isolated_filtered_electrons;
        vector<pair<const xAOD::Muon*, int>> isolated_filtered_muons;
        float temp_ptvarcone40;
        float temp_lep_pT;
		cout<<__LINE__<<endl;
        for (auto electron_info : new_filtered_electrons) {
            const xAOD::Electron* electron = electron_info.first;
            electron->isolation(temp_ptvarcone40,xAOD::Iso::ptvarcone40);
            temp_lep_pT = electron->pt();
            //cout << temp_ptvarcone40 << " " << temp_lep_pT << " " << temp_ptvarcone40/temp_lep_pT << endl;
            if (temp_ptvarcone40/temp_lep_pT < 0.1)
                isolated_filtered_electrons.push_back(electron_info);
        }
		cout<<__LINE__<<endl;
        for (auto muon_info : new_filtered_muons) {
            const xAOD::Muon* muon = muon_info.first;
            muon->isolation(temp_ptvarcone40,xAOD::Iso::ptvarcone40);
            temp_lep_pT = muon->pt();
            //cout << temp_ptvarcone40 << " " << temp_lep_pT << " " << temp_ptvarcone40/temp_lep_pT << endl;
            if (temp_ptvarcone40/temp_lep_pT < 0.1)
                isolated_filtered_muons.push_back(muon_info);
        }
		cout<<__LINE__<<endl;
        update_cutflow(isolated_filtered_electrons, isolated_filtered_muons, 3);
    }

    cout << "\n" << endl;
    print_cutflow(); // Print # leptons passing each step
    outputFile.cd();
    unnormedTree->Write();

    //--- Create normalized tree
    cout << "\nCreating normalized tree" << endl;
    TTree* normalizedTree = new TTree("NormalizedTree", "normalized tree");
    TObjArray* myBranches = (TObjArray*)(unnormedTree->GetListOfBranches())->Clone();
    myBranches->SetOwner(kFALSE);
    Long64_t treeEntries = unnormedTree->GetEntries();

    for (int i=0; i<myBranches->GetEntries(); i++) {
        //--- Get branch
        string currentBranchName = myBranches->At(i)->GetName();
        TBranch* currentBranch = unnormedTree->GetBranch((TString)currentBranchName);

        //--- Find data type of branch
        TClass* branchClass; EDataType branchType;
        currentBranch->GetExpectedType(branchClass, branchType);
        string varType;
        if (branchType == -1)
            varType = currentBranch->GetClassName();
        else if (branchType == 3)
            varType = "I";
        else if (branchType == 5)
            varType = "F";
        else {
            cout << "Unrecognized branch type" << endl;
            exit(0);
        }

        //--- Get branch mean and RMS
        TH1F* histo = new TH1F("histo", "", 1, -1000000, 100000);
        unnormedTree->Draw((currentBranchName + ">>histo").c_str());
        float branchMean = histo->GetMean();
        float branchRMS = histo->GetRMS();
        delete histo;
        if (currentBranchName.rfind("lep_",0)!=0 && currentBranchName.rfind("trk_",0)!=0 && currentBranchName.rfind("calo_cluster_",0)!=0) {
            // don't normalize branches that don't start with lep_, trk_, or calo_cluster_
            branchMean = 0;
            branchRMS = 1;
        }

        //--- Fill tree with normalized branch
        unnormedTree->SetBranchStatus(currentBranchName.c_str(), 1);
        auto fillNonVecBranch = [&] (auto branchVar) {
            unnormedTree->SetBranchAddress(currentBranchName.c_str(), &branchVar);
            float newFloatVar;
            TBranch* normalizedBranch = normalizedTree->Branch(currentBranchName.c_str(), &newFloatVar, (currentBranchName+"/F").c_str());
            for (Long64_t i=0; i<treeEntries; i++) {
                unnormedTree->GetEntry(i);
                newFloatVar = (branchVar-branchMean) / branchRMS;
                normalizedBranch->Fill();
            }
        };
        auto fillVecBranch = [&] (auto branchVar) {
            unnormedTree->SetBranchAddress(currentBranchName.c_str(), &branchVar);
            vector<float>* newFloatVecVar = new vector<float>;
            TBranch* normalizedBranch = normalizedTree->Branch(currentBranchName.c_str(), "vector<float>", &newFloatVecVar);
            for (int i=0; i<treeEntries; i++) {
                unnormedTree->GetEntry(i);
                newFloatVecVar->clear();
                for (size_t i=0; i<branchVar->size(); i++)
                    newFloatVecVar->push_back((branchVar->at(i)-branchMean) / branchRMS);
                normalizedBranch->Fill();
            }
        };
        int intVar = -1; float floatVar = -1;
        vector<int>* intVecVar = new vector<int>; vector<float>* floatVecVar = new vector<float>;
        if (varType == "I") fillNonVecBranch(intVar);
        else if (varType == "F") fillNonVecBranch(floatVar);
        else if (varType == "vector<int>") fillVecBranch(intVecVar);
        else if (varType == "vector<float>") fillVecBranch(floatVecVar);
    }
    normalizedTree->SetEntries(treeEntries);
    normalizedTree->Write();

    outputFile.Close();
    delete inputFileChain;

    return 0;
}
