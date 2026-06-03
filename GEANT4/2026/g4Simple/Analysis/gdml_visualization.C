#include <iostream>
#include "TGeoManager.h"
#include "TSystem.h"
#include "TCollection.h"
#include "TGeoVolume.h"

// --- Function to set transparency on all volumes ---
// Function to set transparency on all volumes
void SetAllTransparency(Int_t transp) {
    if (!gGeoManager) return;
    TCollection* volumes = gGeoManager->GetListOfVolumes();
    TIter next(volumes);
    TGeoVolume* vol;
    while ((vol = (TGeoVolume*)next())) {
        if (vol->IsVisible()) { // Only apply transparency if already visible
            vol->SetTransparency(transp);
            // Ensure lines are visible for contrast
            vol->SetLineColor(kBlack);
        }
    }
    std::cout << "--- Applied transparency (" << transp << "%) to all visible volumes." << std::endl;
}

// --- Main execution function for the macro ---
void visualize_gdml(const char* filename, Int_t transparency = 70) {

    // 1. Load necessary libraries (Gdml and Geometry packages)
    gSystem->Load("libGeom");
    gSystem->Load("libGdml");

    // 2. Import the GDML file
    if (TGeoManager::Import(filename) == nullptr) {
        std::cerr << "Error: Failed to import GDML file: " << filename << std::endl;
        return;
    }
    std::cout << "--- Successfully loaded GDML: " << filename << std::endl;

    // 3. Set visualization level for detail
    gGeoManager->SetVisLevel(5); // Adjust for complexity

    // Assuming your geometry is loaded and gGeoManager exists
    TGeoVolume *topVol = gGeoManager->GetTopVolume();

    if (!topVol) {
        std::cerr << "Error: Top volume not found." << std::endl;
        return;
    }

    // Get the list of positioned daughter nodes
    TObjArray *daughters = topVol->GetNodes();
    Int_t numDaughters = daughters->GetEntries();

    std::cout << "Top volume '" << topVol->GetName() << "' has "
              << numDaughters << " daughter nodes." << std::endl;
    // 4. Set transparency for all volumes
    SetAllTransparency(transparency);

    Int_t numToHide = 1000;
    for (Int_t i = 0; i < numDaughters && i < numToHide; ++i) {

        // Get the TGeoNode object
        TGeoNode *node = (TGeoNode*)daughters->At(i);

        // Get the TGeoVolume associated with this node
        TGeoVolume *volToHide = node->GetVolume();

        // Set the volume visibility to false
        volToHide->SetVisibility(kFALSE); // kFALSE is equivalent to 0/false

        std::cout << "Hiding volume: " << volToHide->GetName() << std::endl;
    }


    // 5. Draw the top volume using the OpenGL viewer
    if (topVol) {
        topVol->Draw("ogl");
    } else {
        std::cerr << "Error: Top volume not found after import." << std::endl;
    }
}

// Main execution function
void visualize_internal(const char* gdml_filename, Int_t transparency = 70) {

    // 1. Load libraries and GDML file
    gSystem->Load("libGeom");
    gSystem->Load("libGdml");
    if (TGeoManager::Import(gdml_filename) == nullptr) {
        std::cerr << "Error: Failed to import GDML file: " << gdml_filename << std::endl;
        return;
    }
    std::cout << "--- Successfully loaded GDML: " << gdml_filename << std::endl;

    TGeoVolume *topVol = gGeoManager->GetTopVolume();
    if (!topVol) return;

    // 2. Identify and HIDE the main outer container (the first daughter of the World)
    TObjArray *daughters = topVol->GetNodes();
    if (daughters->GetEntries() > 0) {
        TGeoNode *main_node = (TGeoNode*)daughters->At(0);
        TGeoVolume *main_vol = main_node->GetVolume();

        // **HIDE the largest volume (often the detector envelope)**
        main_vol->SetVisibility(kFALSE);

        std::cout << "HIDDEN the main external volume: " << main_vol->GetName()
                  << " (Try again if this is not the right volume)." << std::endl;
    }

    // 3. Set remaining volumes to transparent (including the World volume)
    topVol->SetTransparency(transparency); // Make the World transparent
    SetAllTransparency(transparency);      // Make all internal parts transparent

    // 4. Draw the geometry
    topVol->Draw("ogl");
}