// VisualizeGeometry.C
// Loads a GDML geometry and hides the outermost volume(s) to reveal the interior
//
// Usage:
//   root -l 'VisualizeGeometry.C("dune10kt_v5_refactored_1x2x6_nowires.gdml")'
//   root -l 'VisualizeGeometry.C("dune10kt_v5_refactored_1x2x6_nowires.gdml","geometry.png",4,2)'
//                                                                               ^vis_level ^layers to hide

#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoMaterial.h"
#include "TGeoNode.h"
#include "TCanvas.h"
#include "TList.h"
#include "TObjArray.h"
#include "TROOT.h"
#include "TSystem.h"

void ColorizeByMaterial(TGeoManager* geo);
void PrintMaterialSummary(TGeoManager* geo);
void PrintVolumeTree(TGeoNode* node, Int_t depth, Int_t maxDepth);
void HideOuterLayers(TGeoNode* node, Int_t nLayers, Int_t currentDepth = 0);

// ─────────────────────────────────────────────────────────────────────────────
void VisualizeGeo(const char* gdmlFile   = "detector.gdml",
                       const char* outImage   = "",
                       Int_t       visLevel   = 5,
                       Int_t       hideLayers = 1)   // ← how many outer layers to hide
{
    if (gSystem->AccessPathName(gdmlFile)) {
        Error("VisualizeGeometry", "GDML file not found: %s", gdmlFile);
        return;
    }

    // ── Load geometry ─────────────────────────────────────────────────────────
    TGeoManager::Import(gdmlFile);
    TGeoManager* geo = gGeoManager;

    if (!geo) {
        Error("VisualizeGeometry", "TGeoManager failed to import: %s", gdmlFile);
        return;
    }

    TGeoVolume* top = geo->GetTopVolume();
    if (!top) {
        Error("VisualizeGeometry", "No top volume found.");
        return;
    }

    Info("VisualizeGeometry", "Loaded    : %s", gdmlFile);
    Info("VisualizeGeometry", "Top volume: %s", top->GetName());
    Info("VisualizeGeometry", "Volumes   : %d", geo->GetListOfVolumes()->GetEntries());
    Info("VisualizeGeometry", "Materials : %d", geo->GetListOfMaterials()->GetEntries());

    // ── Print volume tree so we know what we are hiding ───────────────────────
    printf("\n[INFO] Volume tree (first 4 levels):\n");
    PrintVolumeTree(geo->GetTopNode(), 0, 4);

    // ── Vis settings ──────────────────────────────────────────────────────────
    geo->SetVisLevel(visLevel);
    geo->SetVisOption(0);
    ColorizeByMaterial(geo);

    // ── Hide outer layers ─────────────────────────────────────────────────────
    // Walk the node tree and call SetVisibility(0) on volumes at depths
    // 0 .. hideLayers-1, making their enclosing shapes invisible while
    // leaving all daughters fully drawn.
    printf("\n[INFO] Hiding %d outer layer(s)...\n", hideLayers);
    HideOuterLayers(geo->GetTopNode(), hideLayers, 0);

    // ── Canvas & draw ─────────────────────────────────────────────────────────
    Bool_t savingToFile = (TString(outImage).Length() > 0);
    if (savingToFile) gROOT->SetBatch(kTRUE);

    TCanvas* c = new TCanvas("c_geometry",
                              Form("GDML Geometry: %s", gdmlFile),
                              1200, 900);
    c->SetFillColor(kBlack);

    top->Draw("ogl");    // use "" for wireframe if OpenGL is unavailable
    c->Update();

    if (savingToFile) {
        c->SaveAs(outImage);
        Info("VisualizeGeometry", "Saved → %s", outImage);
    }

    PrintMaterialSummary(geo);
}

// ─────────────────────────────────────────────────────────────────────────────
// Hide volumes at depth 0 .. nLayers-1 in the node tree.
// At each hidden depth we call:
//   SetVisibility(0)   → do not draw this volume's own shape
//   SetVisDaughters(1) → but DO draw everything inside it
// This is exactly like peeling an onion — the skin disappears, the inside shows.
// ─────────────────────────────────────────────────────────────────────────────
void HideOuterLayers(TGeoNode* node, Int_t nLayers, Int_t currentDepth)
{
    if (!node) return;

    TGeoVolume* vol = node->GetVolume();
    if (!vol) return;

    if (currentDepth < nLayers) {
        // Hide this volume's own shape but keep daughters visible
        vol->SetVisibility(0);        // don't draw the enclosing solid
        vol->SetVisDaughters(1);      // but draw everything inside
        printf("[INFO]   depth %d — hiding volume: %-40s (material: %s)\n",
               currentDepth,
               vol->GetName(),
               vol->GetMaterial()->GetName());
    }

    // Recurse into daughters if we have not yet reached the hide depth
    if (currentDepth < nLayers) {
        Int_t nDaughters = node->GetNdaughters();
        for (Int_t i = 0; i < nDaughters; i++) {
            HideOuterLayers(node->GetDaughter(i), nLayers, currentDepth + 1);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Print the volume tree up to maxDepth levels — helps you decide
// how many layers to hide without running the full visualization.
// ─────────────────────────────────────────────────────────────────────────────
void PrintVolumeTree(TGeoNode* node, Int_t depth, Int_t maxDepth)
{
    if (!node || depth > maxDepth) return;

    TGeoVolume* vol = node->GetVolume();

    // Indent by depth
    for (Int_t i = 0; i < depth; i++) printf("  ");

    printf("[%d] %-40s  material: %-25s  daughters: %d\n",
           depth,
           vol->GetName(),
           vol->GetMaterial()->GetName(),
           node->GetNdaughters());

    Int_t nDaughters = node->GetNdaughters();
    for (Int_t i = 0; i < nDaughters; i++) {
        PrintVolumeTree(node->GetDaughter(i), depth + 1, maxDepth);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ColorizeByMaterial(TGeoManager* geo)
{
    const Int_t   nMats = 14;
    const char*   names[nMats] = {
        "lar", "liquidargon", "argon",
        "steel", "iron",
        "copper",
        "lead",
        "scintillator",
        "rock", "concrete",
        "silicon",
        "aluminum", "aluminium",
        "air"
    };
    const Color_t colors[nMats] = {
        kCyan+1,   kCyan+1,   kCyan+1,
        kGray+2,   kGray+2,
        kOrange+3,
        kGray+1,
        kYellow+1,
        kOrange-7, kGray,
        kGreen+2,
        kBlue-7,   kBlue-7,
        kWhite
    };

    TObjArray* volumes = geo->GetListOfVolumes();
    Int_t      nVols   = volumes->GetEntries();

    for (Int_t i = 0; i < nVols; i++) {
        TGeoVolume* vol     = (TGeoVolume*)volumes->At(i);
        TString     matName = vol->GetMaterial()->GetName();
        matName.ToLower();

        Color_t color = kRed - 4;
        for (Int_t j = 0; j < nMats; j++) {
            if (matName.Contains(names[j])) {
                color = colors[j];
                break;
            }
        }
        vol->SetLineColor(color);
        vol->SetFillColor(color);
        vol->SetTransparency(40);
    }
    Info("ColorizeByMaterial", "Colorized %d volumes.", nVols);
}

// ─────────────────────────────────────────────────────────────────────────────
void PrintMaterialSummary(TGeoManager* geo)
{
    TList* materials = geo->GetListOfMaterials();
    Int_t  nMats     = materials->GetEntries();

    printf("\n[INFO] Materials:\n");
    printf("  %-30s  %18s  %22s\n",
           "Material", "Density [g/cm3]", "Radiation Length [cm]");
    printf("  %s\n", TString('-', 74).Data());

    for (Int_t i = 0; i < nMats; i++) {
        TGeoMaterial* mat = (TGeoMaterial*)materials->At(i);
        printf("  %-30s  %18.4f  %22.4f\n",
               mat->GetName(),
               mat->GetDensity(),
               mat->GetRadLen());
    }
    printf("\n");
}