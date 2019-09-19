// local tools
#include "ObjectFilters.cxx"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/Muon.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"

// AnalysisBase tool include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

// 3rd party includes
#include "TFile.h"
#include "H5Cpp.h"

// stl includes
#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <cassert>

using namespace std;

int main (int argc, char *argv[]) {

    // parse input
    const char* ALG = argv[0];
    string inputFilename = argv[1];

    // object filters
    ObjectFilters object_filters;

    // Open the file:
    unique_ptr<TFile> ifile(TFile::Open(inputFilename.c_str(), "READ"));
    if ( ! ifile.get() || ifile->IsZombie()) {
        throw logic_error("Couldn't open file: " + inputFilename);
        return 1;
    }
    cout << "Opened file: " << inputFilename << endl;

    // Connect the event object to it:
    RETURN_CHECK(ALG, xAOD::Init());
    xAOD::TEvent event(xAOD::TEvent::kClassAccess);
    RETURN_CHECK(ALG, event.readFrom(ifile.get()));

    // Leptons
    TFile outputFile("output.root", "recreate");
    TTree* outputTree = new TTree("BaselineTree", "baseline tree");

    int entry_n; outputTree->Branch("event_n", &entry_n, "event_n/I");
    int pdgID; outputTree->Branch("pdgID", &pdgID, "pdgID/I");
    float lep_pT; outputTree->Branch("lep_pT", &lep_pT, "lep_pT/F");
    float lep_eta; outputTree->Branch("lep_eta", &lep_eta, "lep_eta/F");
    float lep_phi; outputTree->Branch("lep_phi", &lep_phi, "lep_phi/F");
    float lep_d0; outputTree->Branch("lep_d0", &lep_d0, "lep_d0/F");
    float lep_d0_over_sigd0; outputTree->Branch("lep_d0_over_sigd0", &lep_d0_over_sigd0, "lep_d0_over_sigd0/F");
    float lep_z0; outputTree->Branch("lep_z0", &lep_z0, "lep_z0/F");
    float lep_dz0; outputTree->Branch("lep_dz0", &lep_dz0, "lep_dz0/F");
    int truth_type; outputTree->Branch("truth_type", &truth_type, "truth_type/I");
    int isolation; outputTree->Branch("isolation", &isolation, "isolation/I");  //added for tag and probe

    int entries = event.getEntries();
    entries = 1000;
    cout << "got " << entries << " entries" << endl;

    cout << "\nProcessing leptons" << endl;
    for (entry_n = 0; entry_n < entries; ++entry_n) {

        // Print some status
        if ( ! (entry_n % 500)) {
            cout << "Processing " << entry_n << "/" << entries << "\n";
        }

        // Load the event
        bool ok = event.getEntry(entry_n) >= 0;
        if (!ok) throw logic_error("getEntry failed");

        // Get tracks and leptons
        const xAOD::TrackParticleContainer *tracks;
        RETURN_CHECK(ALG, event.retrieve(tracks, "InDetTrackParticles"));
        const xAOD::VertexContainer *primary_vertices;
        RETURN_CHECK(ALG, event.retrieve(primary_vertices, "PrimaryVertices"));
        const xAOD::Vertex *primary_vertex = primary_vertices->at(0);
        const xAOD::ElectronContainer *electrons;
        RETURN_CHECK(ALG, event.retrieve(electrons, "Electrons"));
        const xAOD::MuonContainer *muons;
        RETURN_CHECK(ALG, event.retrieve(muons, "Muons"));
        const xAOD::CaloClusterContainer *calo_clusters;
        RETURN_CHECK(ALG, event.retrieve(calo_clusters, "CaloCalTopoClusters"));

        // Filter objects
        vector<const xAOD::TrackParticle*> filtered_tracks = object_filters.filter_tracks(tracks, primary_vertex);
        vector<const xAOD::Electron*> filtered_electrons = object_filters.filter_electrons(electrons);
        vector<const xAOD::Muon*> filtered_muons = object_filters.filter_muons(muons);

        // Write event
        SG::AuxElement::ConstAccessor<float> accessPromptVar("PromptLeptonVeto");

        auto process_lepton = [&] (const xAOD::IParticle* lepton, const xAOD::TrackParticle* track_particle) {
            lep_pT = lepton->pt();
            lep_eta = lepton->eta();
            lep_phi = lepton->phi();
            lep_d0 = track_particle->d0();
            lep_d0_over_sigd0 = xAOD::TrackingHelpers::d0significance(track_particle);
            lep_z0 = track_particle->z0();
            lep_dz0 = track_particle->z0() - primary_vertex->z();
            PLT = accessPromptVar(*lepton);

            trk_lep_dR->clear(); trk_pT->clear(); trk_eta->clear(); trk_phi->clear();
            trk_d0->clear(); trk_z0->clear(); trk_charge->clear(); chiSquared->clear();
            nIBLHits->clear(); nPixHits->clear(); nPixHoles->clear(); nPixOutliers->clear();
            nSCTHits->clear(); nSCTHoles->clear(); nTRTHits->clear();
            for (auto track : filtered_tracks) {
                float dR = track->p4().DeltaR(lepton->p4());
                if (dR > 0.5) continue; 
                trk_lep_dR->push_back(dR);
                trk_pT->push_back(track->pt());
                trk_eta->push_back(track->eta());
                trk_phi->push_back(track->phi());
                trk_d0->push_back(track->d0());
                trk_z0->push_back(track->z0());
                trk_charge->push_back(track->charge());
                chiSquared->push_back(track->chiSquared());
                uint8_t placeholder;
                track->summaryValue(placeholder, xAOD::numberOfInnermostPixelLayerHits); nIBLHits->push_back(placeholder);
                track->summaryValue(placeholder, xAOD::numberOfPixelHits); nPixHits->push_back(placeholder);
                track->summaryValue(placeholder, xAOD::numberOfPixelHoles); nPixHoles->push_back(placeholder);
                track->summaryValue(placeholder, xAOD::numberOfPixelOutliers); nPixOutliers->push_back(placeholder);
                track->summaryValue(placeholder, xAOD::numberOfSCTHits); nSCTHits->push_back(placeholder);
                track->summaryValue(placeholder, xAOD::numberOfSCTHoles); nSCTHoles->push_back(placeholder);
                track->summaryValue(placeholder, xAOD::numberOfTRTHits); nTRTHits->push_back(placeholder);
            }
        };

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

        for (auto electron : filtered_electrons) {
            truth_type = xAOD::TruthHelpers::getParticleTruthType(*electron); // 2 = real prompt, 3 = HF
            if (truth_type != 2 && truth_type != 3) continue;

            pdgID = 11;
            auto track_particle = electron->trackParticle();
            process_lepton(electron, track_particle);
            process_electron_cones(electron);

            outputTree->Fill();
        }

        for (auto muon : filtered_muons) {
            truth_type = xAOD::TruthHelpers::getParticleTruthType(*(muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle))); // 2 = real prompt, 3 = HF
            if (truth_type != 2 && truth_type != 3) continue;

            pdgID = 13;
            auto track_particle = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
            process_lepton(muon, track_particle);
            process_muon_cones(muon);

            outputTree->Fill();
        }
    }

    outputTree->Write();
    outputFile.Close();

    return 0;
}
