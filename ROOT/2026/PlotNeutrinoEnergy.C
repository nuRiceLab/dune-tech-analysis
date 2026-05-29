
void PlotNeutrinoEnergy(const char* inFile  = "neutrino_energy.root",
                        const char* outPlot = "neutrino_energy_plot.png")
{
    // ── Open file & retrieve TTree ────────────────────────────────────────────
    TFile *f = TFile::Open(inFile, "READ");
    if (!f || f->IsZombie()) {
        Error("PlotNeutrinoEnergy", "Cannot open file: %s", inFile);
        return;
    }

    TTree *tree = (TTree*)f->Get("NeutrinoTree");
    if (!tree) {
        Error("PlotNeutrinoEnergy", "TTree 'NeutrinoTree' not found in %s", inFile);
        f->Close();
        return;
    }

    Info("PlotNeutrinoEnergy", "Opened '%s' — %lld events found.", inFile, tree->GetEntries());

    // ── Connect branch to a local variable ───────────────────────────────────
    Double_t NeutrinoEnergy;
    tree->SetBranchAddress("NeutrinoEnergy", &NeutrinoEnergy);

    // ── Create histogram ──────────────────────────────────────────────────────
    TH1D *h = new TH1D("h_NeutrinoEnergy",
                        "Neutrino Energy Distribution;"   // title
                        "Neutrino Energy [GeV];"          // x-axis label
                        "Events / 0.09 GeV",              // y-axis label
                        100, 0, 10.0);

    h->SetLineColor(kBlue + 1);
    h->SetLineWidth(2);
    h->SetFillColorAlpha(kBlue - 9, 0.45);

    // ── Fill loop ─────────────────────────────────────────────────────────────
    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);           // loads all branches for event i
        h->Fill(NeutrinoEnergy);
    }
    // ── Canvas & drawing ──────────────────────────────────────────────────────
    TCanvas *c = new TCanvas("c_NeutrinoEnergy", "Neutrino Energy", 900, 650);
    c->SetGrid();
    c->SetLeftMargin(0.12);
    c->SetBottomMargin(0.12);

    h->Draw("HIST"); 
    c->Update();
    c->SaveAs(outPlot);
}