void GenerateNeutrinoEnergy(Int_t nEvents = 10000,
    const char* outFile = "neutrino_energy.root")
{
    // ── Output file ─────────────────────────────
    TFile *f = TFile::Open(outFile, "RECREATE");
    if (!f || f->IsZombie()) {
        Error("GenerateNeutrinoEnergy", "Cannot open output file: %s", outFile);
        return;
    }
    // TTree construction
    TTree *tree = new TTree("NeutrinoTree", "Simulated Neutrino Energy Data");
    Double_t NeutrinoEnergy;
    tree->Branch("NeutrinoEnergy", &NeutrinoEnergy, "NeutrinoEnergy/D");

    // TRandom3 uses the Mersenne Twister algorithm — standard in HEP
    TRandom3 rng(42);   // seed=42 for reproducibility; use 0 for truly random
    const Double_t mean  = 3.5;   // GeV
    const Double_t sigma = 1.0;   // GeV

    // ── Fill loop ─────────────────────────────────────────────
    for (Int_t i = 0; i < nEvents; i++) {
        NeutrinoEnergy = rng.Gaus(mean, sigma);
        tree->Fill();
    }

    // ── Write & close ─────────────────────────────────────────
    f->Write();
    f->Close();

}   

