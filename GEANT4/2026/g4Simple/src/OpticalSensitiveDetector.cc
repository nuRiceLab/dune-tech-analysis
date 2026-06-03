//
// Created by ilker on 5/31/26.
//

#include "OpticalSensitiveDetector.hh"

#include <G4OpticalPhoton.hh>

#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
OpticalSensitiveDetector::OpticalSensitiveDetector(const G4String& name) : G4VSensitiveDetector(name)
{
    fname=name;
}


G4bool OpticalSensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory* )
{
    G4Track* atrack = aStep->GetTrack();
    //std::cout << "Optical Processing .. " <<std::endl;
    if (atrack->GetParticleDefinition() != G4OpticalPhoton::Definition()) return false;
    G4double edep = atrack->GetTotalEnergy();


    // Create a hit and set its properties
    G4int id=3;
    G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    G4AnalysisManager *ana=G4AnalysisManager::Instance();
    ana->FillNtupleIColumn(id,0,eventID);
    ana->FillNtupleSColumn(id,1,atrack->GetParticleDefinition()->GetParticleName());
    ana->FillNtupleIColumn(id,2,atrack->GetParticleDefinition()->GetPDGEncoding());
    ana->FillNtupleDColumn(id,3,aStep->GetPostStepPoint()->GetPosition().x());
    ana->FillNtupleDColumn(id,4,aStep->GetPostStepPoint()->GetPosition().y());
    ana->FillNtupleDColumn(id,5,aStep->GetPostStepPoint()->GetPosition().z());
    ana->FillNtupleDColumn(id,6,aStep->GetTrack()->GetGlobalTime());
    ana->FillNtupleDColumn(id,7,edep);
    ana->FillNtupleSColumn(id,8,fname);
    ana->FillNtupleIColumn(id,9,atrack->GetTrackID());
    ana->AddNtupleRow(id);
    atrack->SetTrackStatus(fStopAndKill);
    return true;
}
