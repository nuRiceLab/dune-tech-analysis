// Includes some helpful utilities for this exercise
#include "example_utils.h"

// Standard c++/c includes
#include <vector>


// Task -- Add the proper includes here
#include "TTree.h"
#include "TFile.h"
#include "TH2D.h"
#include "TRandom3.h"

int main(int argc, char ** argv) {

    //Parse arguments
    Config args;
    if (!ParseArgs(argc, argv, args)) return 1;

    std::cout << "Will read a TFile with the name " << args.input_file << std::endl;
    // Task -- Make a read-only TFile with the name given by 'args.input_file' (a std::string)
    TFile fin(args.input_file.c_str(), "open");

    //Task -- extract the TTree under TreeDir, and the 2 hists under HistDir
    TTree * tree = (TTree*)fin.Get("TreeDir/tree");
    TH1D * hist = (TH1D*)fin.Get("HistDir/hist1D");
    TH1D * weighted = (TH1D*)fin.Get("HistDir/weighted1D");

    //Task -- print out the sum of entry counts in both histograms
    std::cout << hist->Integral() << std::endl;
    std::cout << weighted->Integral() << std::endl;

    //Task -- Make a new writable TFile to store the histograms 
    //     -- The file's name will come from args.output_file
    TFile fout(args.output_file.c_str(), "recreate");

    //Task -- make a new TH1D named "length" w/ 40 bins from 20 to 40
    TH1D h_length("length", "", 40, 20, 40);

    //Task -- Access the following TBranches
    //          -- momentum (double)
    //          -- my_vector (vector of doubles)
    double momentum = 0.;
    std::vector<double> * my_vector = nullptr;
    tree->SetBranchAddress("momentum", &momentum);
    tree->SetBranchAddress("my_vector", &my_vector);

    //We'll read args.nevents (or the max number of entries) from the tree
    int nentries =  ((tree->GetEntries() < args.nevents) ? tree->GetEntries() : args.nevents);
    for (int ievent = 0; ievent < nentries; ++ievent) {

        //Just some output to track progress
        if (!(ievent % 100))
            std::cout << "\rProcessing event " << ievent << std::flush;

        //Task -- Access the next event
        tree->GetEntry(ievent);

        //We'll simulate the particle traveling some distance according to its momentum
        double distance = LengthModel(momentum);
        std::cout << distance << std::endl;
    
        //Task -- fill the length histogram with that distance
        h_length.Fill(distance);
    }
    std::cout << std::endl;

    //Make this for later
    TH2D weights_vs_momentum("weights_vs_momentum", "", 100, 3, 7, 100, 0, 5.);

    //Task -- From the loaded TTree, draw the 2D distribution of weights vs momentum,
    //     -- and store it in the histogram we just created called "weights_vs_momentum".
    tree->Draw("event_weight:momentum>>weights_vs_momentum");

    //When outside of the prompt, this is a simple way to get histograms from TTree::Draw
    // auto * weights_vs_momentum = (TH2D*)gDirectory->Get("weights_vs_momentum");

    //Task -- Write out the hists (length & weights_vs_momentum) we created in the output file and close the file
    fout.cd();
    h_length.Write();
    weights_vs_momentum.Write();
    fout.Close();


    std::cout << "Done" << std::endl;

    // Exit with success code
    return 0;
}
