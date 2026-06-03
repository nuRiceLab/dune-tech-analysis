//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1/src/SteppingAction.cc
/// \brief Implementation of the B1::SteppingAction class

#include "SteppingAction.hh"
#include "G4RunManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
namespace B1
{

    SteppingAction::SteppingAction()
    {
        ana=G4AnalysisManager::Instance();
    }


    void SteppingAction::UserSteppingAction(const G4Step* step)
    {
        auto atrack = step->GetTrack();
        G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

        if (atrack->GetTrackStatus()==fStopAndKill  && atrack->GetParticleDefinition()==G4OpticalPhoton::Definition())
        {
            G4String PreVol="None",PostVol="None";
            if (step->GetPreStepPoint()->GetPhysicalVolume()) PreVol=step->GetPreStepPoint()->GetPhysicalVolume()->GetName();
            if (step->GetPostStepPoint()->GetPhysicalVolume()) PostVol=step->GetPostStepPoint()->GetPhysicalVolume()->GetName();

            G4int id=2;
            ana->FillNtupleIColumn(id,0,eventID);
            ana->FillNtupleSColumn(id,1,atrack->GetParticleDefinition()->GetParticleName());
            ana->FillNtupleIColumn(id,2,atrack->GetParticleDefinition()->GetPDGEncoding());
            ana->FillNtupleDColumn(id,3,atrack->GetPosition().x());
            ana->FillNtupleDColumn(id,4,atrack->GetPosition().y());
            ana->FillNtupleDColumn(id,5,atrack->GetPosition().z());
            ana->FillNtupleDColumn(id,6,atrack->GetTotalEnergy()/CLHEP::eV);
            ana->FillNtupleSColumn(id,7,PreVol);
            ana->FillNtupleSColumn(id,8,PostVol);
            ana->AddNtupleRow(id);
        }

    }
}
