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
/// \file B1/src/PrimaryGeneratorAction.cc
/// \brief Implementation of the B1::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"
#include "G4AnalysisManager.hh"
namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    gps=new G4GeneralParticleSource();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete gps;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    auto ana = G4AnalysisManager::Instance();
    gps->GeneratePrimaryVertex(anEvent);

    G4int id=0;
    ana->FillNtupleIColumn(id,0,anEvent->GetEventID());
    ana->FillNtupleSColumn(id,1,gps->GetParticleDefinition()->GetParticleName());
    ana->FillNtupleIColumn(id,2,gps->GetParticleDefinition()->GetPDGEncoding());
    ana->FillNtupleDColumn(id,3,gps->GetParticlePosition().x());
    ana->FillNtupleDColumn(id,4,gps->GetParticlePosition().y());
    ana->FillNtupleDColumn(id,5,gps->GetParticlePosition().z());
    ana->FillNtupleDColumn(id,6,gps->GetParticleEnergy());
    ana->FillNtupleDColumn(id,7,gps->GetParticleMomentumDirection().x());
    ana->FillNtupleDColumn(id,8,gps->GetParticleMomentumDirection().y());
    ana->FillNtupleDColumn(id,9,gps->GetParticleMomentumDirection().z());
    ana->AddNtupleRow(id);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}


