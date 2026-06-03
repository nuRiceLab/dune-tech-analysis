//
// Created by ilker on 5/31/26.
//

#include "G4Step.hh"
#include "G4VSensitiveDetector.hh"
#include "G4AnalysisManager.hh"

#ifndef B1_SENSITIVEDETECTOR_HH
#define B1_SENSITIVEDETECTOR_HH


class SensitiveDetector : public G4VSensitiveDetector
{
    public:
    SensitiveDetector(const G4String& name);

    G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist) override;
    G4String fname;
};


#endif //B1_SENSITIVEDETECTOR_HH
