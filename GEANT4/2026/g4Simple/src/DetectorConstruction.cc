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
/// \file B1/src/DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"


#include "G4GDMLParser.hh"
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "SensitiveDetector.hh"
#include "OpticalSensitiveDetector.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4SDManager.hh"
#include "G4UserLimits.hh"

using namespace CLHEP;
namespace B1
{

DetectorConstruction::DetectorConstruction():stepLimit(0.4*mm),Arphase("Liquid") {
  fMessenger= new G4GenericMessenger(this,"/Geometry/","Geometry control");
  fMessenger->DeclarePropertyWithUnit("steplim", "mm" ,stepLimit ,"Finner steps");
  fMessenger->DeclareProperty("ArPhase",Arphase,"Argon Phase Liquid or Gas");

}

DetectorConstruction::~DetectorConstruction() {
  delete fMessenger;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

  /// Gaseous argon ///
G4MaterialPropertiesTable* DetectorConstruction::GAr()
  {
    // An argon gas proportional scintillation counter with UV avalanche photodiode scintillation
    // readout C.M.B. Monteiro, J.A.M. Lopes, P.C.P.S. Simoes, J.M.F. dos Santos, C.A.N. Conde
    //
    // May 2023:
    // Updated scintillation decay and yields from:
    // Triplet Lifetime in Gaseous Argon. Michael Akashi-Ronquest et al.

    G4MaterialPropertiesTable* mpt = new G4MaterialPropertiesTable();

    // REFRACTIVE INDEX
    const G4int ri_entries = 200;
    G4double eWidth = (optPhotMaxE_ - optPhotMinE_) / ri_entries;

    std::vector<G4double> ri_energy;
    for (int i=0; i<ri_entries; i++) {
      ri_energy.push_back(optPhotMinE_ + i * eWidth);
    }

    std::vector<G4double> rIndex;
    for (int i=0; i<ri_entries; i++) {
      G4double wl = hc_ / ri_energy[i] * 1000; // in micron
      // From refractiveindex.info
      rIndex.push_back(1 + 0.012055*(0.2075*pow(wl,2)/(91.012*pow(wl,2)-1) +
                                     0.0415*pow(wl,2)/(87.892*pow(wl,2)-1) +
                                     4.3330*pow(wl,2)/(214.02*pow(wl,2)-1)));
      //G4cout << "* GAr rIndex:  " << std::setw(5) << ri_energy[i]/eV
      //       << " eV -> " << rIndex[i] << G4endl;
    }
    mpt->AddProperty("RINDEX", ri_energy, rIndex);

    // EMISSION SPECTRUM
    G4double Wavelength_peak  = 128.000 * nm;
    G4double Wavelength_sigma =   2.929 * nm;
    G4double Energy_peak  = (hc_ / Wavelength_peak);
    G4double Energy_sigma = (hc_ * Wavelength_sigma / pow(Wavelength_peak,2));
    //G4cout << "*** GAr Energy_peak: " << Energy_peak/eV << " eV   Energy_sigma: "
    //       << Energy_sigma/eV << " eV" << G4endl;

    // Sampling from ~110 nm to 150 nm <----> from ~11.236 eV to 8.240 eV
    const G4int sc_entries = 380;
    std::vector<G4double> sc_energy;
    std::vector<G4double> intensity;
    for (int i=0; i<sc_entries; i++){
      sc_energy.push_back(8.240*eV + 0.008*i*eV);
      intensity.push_back(exp(-pow(Energy_peak/eV-sc_energy[i]/eV,2) /
                              (2*pow(Energy_sigma/eV, 2)))/(Energy_sigma/eV*sqrt(pi*2.)));
      //G4cout << "* GAr energy: " << std::setw(6) << sc_energy[i]/eV << " eV  ->  "
      //       << std::setw(6) << intensity[i] << G4endl;
    }
    mpt->AddProperty("SCINTILLATIONCOMPONENT1", sc_energy, intensity);
    mpt->AddProperty("SCINTILLATIONCOMPONENT2", sc_energy, intensity);
    // CONST PROPERTIES
    mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT1",   6.*ns);
    mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT2",   3480.*ns);
    mpt->AddConstProperty("SCINTILLATIONYIELD", 30/MeV);
    mpt->AddConstProperty("SCINTILLATIONYIELD1", .136);
    mpt->AddConstProperty("SCINTILLATIONYIELD2", .864);
    mpt->AddConstProperty("RESOLUTIONSCALE",    1.0);
    return mpt;
  }

  G4MaterialPropertiesTable* DetectorConstruction::LAr()
  {
    // An argon gas proportional scintillation counter with UV avalanche photodiode scintillation
    // readout C.M.B. Monteiro, J.A.M. Lopes, P.C.P.S. Simoes, J.M.F. dos Santos, C.A.N. Conde
    //
    // May 2023:
    // Updated scintillation decay and yields from:
    // Triplet Lifetime in Gaseous Argon. Michael Akashi-Ronquest et al.

    G4MaterialPropertiesTable* mpt = new G4MaterialPropertiesTable();

    // REFRACTIVE INDEX
    const G4int ri_entries = 200;
    G4double eWidth = (optPhotMaxE_ - optPhotMinE_) / ri_entries;

    std::vector<G4double> ri_energy;
    for (int i=0; i<ri_entries; i++) {
      ri_energy.push_back(optPhotMinE_ + i * eWidth);
    }

    std::vector<G4double> rIndex;
    for (int i=0; i<ri_entries; i++) {
      G4double wl = hc_ / ri_energy[i] * 1000; // in micron
      // From refractiveindex.info
      rIndex.push_back(1 + 0.012055*(0.2075*pow(wl,2)/(91.012*pow(wl,2)-1) +
                                     0.0415*pow(wl,2)/(87.892*pow(wl,2)-1) +
                                     4.3330*pow(wl,2)/(214.02*pow(wl,2)-1)));
      //G4cout << "* GAr rIndex:  " << std::setw(5) << ri_energy[i]/eV
      //       << " eV -> " << rIndex[i] << G4endl;
    }
    mpt->AddProperty("RINDEX", ri_energy, rIndex);


    // EMISSION SPECTRUM
    G4double Wavelength_peak  = 128.000 * nm;
    G4double Wavelength_sigma =   2.929 * nm;
    G4double Energy_peak  = (hc_ / Wavelength_peak);
    G4double Energy_sigma = (hc_ * Wavelength_sigma / pow(Wavelength_peak,2));
    //G4cout << "*** GAr Energy_peak: " << Energy_peak/eV << " eV   Energy_sigma: "
    //       << Energy_sigma/eV << " eV" << G4endl;

    // Sampling from ~110 nm to 150 nm <----> from ~11.236 eV to 8.240 eV
    const G4int sc_entries = 380;
    std::vector<G4double> sc_energy;
    std::vector<G4double> intensity;
    for (int i=0; i<sc_entries; i++){
      sc_energy.push_back(8.240*eV + 0.008*i*eV);
      intensity.push_back(exp(-pow(Energy_peak/eV-sc_energy[i]/eV,2) /
                              (2*pow(Energy_sigma/eV, 2)))/(Energy_sigma/eV*sqrt(pi*2.)));
      //G4cout << "* GAr energy: " << std::setw(6) << sc_energy[i]/eV << " eV  ->  "
      //       << std::setw(6) << intensity[i] << G4endl;
    }
    mpt->AddProperty("SCINTILLATIONCOMPONENT1", sc_energy, intensity);
    mpt->AddProperty("SCINTILLATIONCOMPONENT2", sc_energy, intensity);
    // CONST PROPERTIES
    mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT1",   6.*ns);
    mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT2",   1500.*ns);
    mpt->AddConstProperty("SCINTILLATIONYIELD", 30/MeV);
    mpt->AddConstProperty("SCINTILLATIONYIELD1", .2);
    mpt->AddConstProperty("SCINTILLATIONYIELD2", .8);
    mpt->AddConstProperty("RESOLUTIONSCALE",    1.0);
    return mpt;
  }


G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  // Define Geometry size
  G4double  WorldSize =20*cm;
  G4double  ShieldSize = WorldSize/2+2*cm;
  G4double  ArSize = WorldSize/2;
  G4double  DetSize = 1*cm;


  // Materials
  G4Material* World_mat = nist->FindOrBuildMaterial("G4_AIR");
  G4Material* Shield_mat = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  G4Material* Det_mat = nist->FindOrBuildMaterial("G4_Si");
  G4Material *Ar_mat=nullptr;
  //Gas Argon
  G4Material* GAr_mat = nist->FindOrBuildMaterial("G4_Ar");
  GAr_mat->SetMaterialPropertiesTable(GAr());

  // Liquid Argon
  G4double LArdensity = 1.396*g/cm3;
  G4double LArtemperature = 87*kelvin;
  G4double LArpressure = 1*atmosphere;
  auto LAr_mat = new G4Material("LAr", 18., 39.948*g/mole, LArdensity,
                            kStateLiquid, LArtemperature, LArpressure);
  LAr_mat->SetMaterialPropertiesTable(LAr());

  // Properties
  G4MaterialPropertiesTable* detMPT = new G4MaterialPropertiesTable();
  std::vector<G4double> ephoton = { 1.0*eV, 7.0*eV, 10.0*eV }; // Cover your Argon energy range
  std::vector<G4double> rindex_det = { 1.5, 1.52, 1.57 };     // Approx RINDEX for Silicon
  detMPT->AddProperty("RINDEX", ephoton, rindex_det);
  Det_mat->SetMaterialPropertiesTable(detMPT);

  // Material Properties Table


  // World Volume
  auto solidWorld = new G4Box("World",                           // its name
    0.5 * WorldSize, 0.5 * WorldSize, 0.5 * WorldSize);  // its size

  auto logicWorld = new G4LogicalVolume(solidWorld,  // its solid
    World_mat,                                       // its material
    "World_LV");                                        // its name

  auto physWorld = new G4PVPlacement(nullptr,  // no rotation
    G4ThreeVector(),                           // at (0,0,0)
    logicWorld,                                // its logical volume
    "World_PV",                                   // its name
    nullptr,                                   // its mother  volume
    false,                                     // no boolean operation
    0,                                         // copy number
    0);                            // overlaps checking

  // Shield
  auto solidShield = new G4Box("Shield",0.5 * ShieldSize, 0.5 * ShieldSize, 0.5 * ShieldSize);  // its size
  auto logicShield= new G4LogicalVolume(solidShield,Shield_mat,"Shield_LV");
   new G4PVPlacement(nullptr,
    G4ThreeVector(),
    logicShield,"Shield_PV",
    logicWorld,
    false,
    0,
    0);

  auto solidAr = new G4Box("Ar",0.5 * ArSize, 0.5 * ArSize, 0.5 * ArSize);  // its size

  // Gas or Liquid for Ionization Region
  G4String name="GAr";
  if (Arphase=="Gas") Ar_mat=GAr_mat;
  else
  {
    Ar_mat=LAr_mat;
    name="LAr";
  }


  auto logicAr= new G4LogicalVolume(solidAr,Ar_mat,"Ar_LV");
  logicAr->SetUserLimits(new G4UserLimits(stepLimit)); // Step limit to 1 mm to better sample scintillation light production
   new G4PVPlacement(nullptr,
    G4ThreeVector(),
    logicAr,(name+"_PV").c_str(),
    logicShield,
    false,
    0,
    0);

  // Photon Detector
  auto solidDet = new G4Box("Det",(0.5 * ArSize)-1*cm, (0.5 * ArSize)-1*cm, 1*mm);  // its size
  auto logicDet= new G4LogicalVolume(solidDet,Det_mat,"Det_LV");
  new G4PVPlacement(nullptr,
   G4ThreeVector(0,0,-ArSize/2+DetSize/2),
   logicDet,"Det_PV",
   logicAr,
   false,
   0,
   0);

  // Visibilities

  // Define color within here or macros
  logicWorld->SetVisAttributes(G4VisAttributes(G4Color(1,0,0,0.2))); // Red
  logicAr->SetVisAttributes(G4VisAttributes(G4Color(0,0,1,0.2))); // Argon
  logicDet->SetVisAttributes(G4VisAttributes(G4Color(0,1,0,1))); // Green
  logicShield->SetVisAttributes(G4VisAttributes(G4Color(1,1,1,0.2))); //White


  // Save geometry as gdml
  G4GDMLParser parser;
  G4String gdmlname="geo.gdml";
  std::remove(gdmlname.c_str());
  parser.Write(gdmlname.c_str(),physWorld);
  return physWorld;
}

  void DetectorConstruction::ConstructSDandField()
{

  // Sensitive Detector
  auto sd = new SensitiveDetector("IonizationAr");
  G4SDManager::GetSDMpointer()->AddNewDetector(sd);

  // Attach by name (recommended)
  SetSensitiveDetector("Ar_LV", sd);


  // Optical Detector
  auto o_sd = new OpticalSensitiveDetector("Optical");
  G4SDManager::GetSDMpointer()->AddNewDetector(o_sd);

  // Attach by name (recommended)
  SetSensitiveDetector("Det_LV", o_sd);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
