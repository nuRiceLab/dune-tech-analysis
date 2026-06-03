//
// Created by ilker on 5/31/26.
//

#include "G4Step.hh"
#include "G4VSensitiveDetector.hh"

#ifndef B1_OPTICALSENSITIVEDETECTOR_HH
#define B1_OPTICALSENSITIVEDETECTOR_HH


class OpticalSensitiveDetector : public G4VSensitiveDetector
{
    public:
    OpticalSensitiveDetector(const G4String& name);

    G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist) override;
    G4String fname;
};


#endif //B1_OPTICALSENSITIVEDETECTOR_HH
