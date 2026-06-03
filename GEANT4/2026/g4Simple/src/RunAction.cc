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
/// \file B1/src/RunAction.cc
/// \brief Implementation of the B1::RunAction class

#include "RunAction.hh"
#include "G4Run.hh"

#include "G4AnalysisManager.hh"
using namespace std;
namespace B1
{

RunAction::RunAction()
{

}


void RunAction::BeginOfRunAction(const G4Run* run)
{
    cout << "Begin of Run " << run->GetRunID() << endl;
    // Get Analysis Manager
    auto ana = G4AnalysisManager::Instance();
    G4String fileName = "result.root";
    ana->OpenFile(fileName);

    ana->CreateNtuple("GPS", "Primary Particle Info");
    ana->CreateNtupleIColumn("evtid");
    ana->CreateNtupleSColumn("pname");
    ana->CreateNtupleIColumn("pdg");
    ana->CreateNtupleDColumn("x");
    ana->CreateNtupleDColumn("y");
    ana->CreateNtupleDColumn("z");
    ana->CreateNtupleDColumn("kenergy");
    ana->CreateNtupleDColumn("mdx");
    ana->CreateNtupleDColumn("mdy");
    ana->CreateNtupleDColumn("mdz");
    ana->FinishNtuple();

    ana->CreateNtuple("hits", "Particle hits");
    ana->CreateNtupleIColumn("evtid");
    ana->CreateNtupleSColumn("pname");
    ana->CreateNtupleIColumn("pdg");
    ana->CreateNtupleDColumn("x");
    ana->CreateNtupleDColumn("y");
    ana->CreateNtupleDColumn("z");
    ana->CreateNtupleDColumn("t");
    ana->CreateNtupleDColumn("edep");
    ana->CreateNtupleSColumn("det_name");
    ana->CreateNtupleIColumn("TrackId");
    ana->FinishNtuple();


    ana->CreateNtuple("final", "Information about particle at its final destination");
    ana->CreateNtupleIColumn("evtid");
    ana->CreateNtupleSColumn("pname");
    ana->CreateNtupleIColumn("pdg");
    ana->CreateNtupleDColumn("x");
    ana->CreateNtupleDColumn("y");
    ana->CreateNtupleDColumn("z");
    ana->CreateNtupleDColumn("kenergy");
    ana->CreateNtupleSColumn("PreVol");
    ana->CreateNtupleSColumn("PostVol");
    ana->FinishNtuple();

    ana->CreateNtuple("ohits", "Optical Photon hits");
    ana->CreateNtupleIColumn("evtid");
    ana->CreateNtupleSColumn("pname");
    ana->CreateNtupleIColumn("pdg");
    ana->CreateNtupleDColumn("x");
    ana->CreateNtupleDColumn("y");
    ana->CreateNtupleDColumn("z");
    ana->CreateNtupleDColumn("t");
    ana->CreateNtupleDColumn("edep");
    ana->CreateNtupleSColumn("det_name");
    ana->CreateNtupleIColumn("TrackId");

    ana->FinishNtuple();

}

void RunAction::EndOfRunAction(const G4Run* run)
{
    cout << "End of Run " << run->GetRunID() << endl;
    auto ana = G4AnalysisManager::Instance();
    ana->Write();
    ana->CloseFile();
}


}
