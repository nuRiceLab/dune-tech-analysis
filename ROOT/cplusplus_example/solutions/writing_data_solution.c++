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

    std::cout << "Will write a TFile with the name " << args.output_file << std::endl;
    // Task -- Make a writable TFile with the name given by 'args.output_file' (a std::string)
    //TODO -- MAKE SURE I SPELL OUT C_STR() SOMEWHERE
    TFile fout(args.output_file.c_str(), "recreate");


    //Task -- Make 2 TDirectories: 1 named 'TreeDir', 1 named 'HistDir
    auto * tree_dir = fout.mkdir("TreeDir");
    auto * hist_dir = fout.mkdir("HistDir");


    //Task -- Make a TTree named "tree" within TreeDir
    tree_dir->cd();
    TTree tree("tree", "");

    //Task -- Make 2 TH1Ds. 1 named "hist1D" 1 named "weighted1D" within HistDir, Each has 50 bins, in the 3, 7 range
    hist_dir->cd();
    TH1D hist("hist1D", "Example Hist", 50, 3, 7);
    TH1D hist_weighted("weighted1D", "Example Hist", 50, 3, 7);

    
    //Task -- Set those hist's axis titles to 'Momentum [GeV/c]' for x and 'Entries' for y"
    std::string xtitle = "Momentum [GeV/c]",
                ytitle = "Entries";
    hist.SetXTitle(xtitle.c_str());
    hist.SetYTitle(ytitle.c_str());

    hist_weighted.SetXTitle(xtitle.c_str());
    hist_weighted.SetYTitle(ytitle.c_str());


    //Task -- Make a TRandom3 object initialized with seed = 12345
    TRandom3 rng(12345);

    //Task -- Make some TBranches
    //     -- The following are required
    //          -- momentum (double)
    //          -- event_weight (double)
    //          -- poisson_number (int)
    //          -- my_vector (vector of doubles)
    double momentum_out = 0., event_weight_out = 0.;
    int poisson_out = 0;
    std::vector<double> my_vector;
    tree.Branch("momentum", &momentum_out);
    tree.Branch("event_weight", &event_weight_out);
    tree.Branch("poisson_number", &poisson_out);
    tree.Branch("my_vector", &my_vector);


    //We'll make args.nevents number of events within this tree
    for (size_t ievent = 0; ievent < args.nevents; ++ievent) {

        //Just some output to track progress
        if (!(ievent % 100))
            std::cout << "\rProcessing event " << ievent << std::flush;

        //Task -- Generate 2 random numbers:
        //     -- a) from a Gaussian with mean 5 and width .5
        //     -- b) an integer from a Poisson distribution -- you choose the mean
        double p = rng.Gaus(5., .3);
        int n = rng.Poisson(7);

        //We'll generate some event weight to be used later
        double w = GetEventWeight(p);

    
        //Task -- fill the histograms with that same gaussian random number from above
        //     -- hist1D will be filled unweighted, weighted1D will be filled with the weight 
        //     -- from above
        hist.Fill(p);
        hist_weighted.Fill(p, w);

        //Task -- set the values of the tree's branches with the relevant values from above
        //     -- Add entries to my_vector based off of the poisson number you generated (you choose their values)
        //     -- Make sure you clear any vector fields first so they don't keep accruing in size!
        momentum_out = p;
        poisson_out = n;
        event_weight_out = w;
        my_vector.clear();
        for (int i = 0; i < n; ++i) my_vector.push_back(i);

        //Task -- Write the event to the TTree
        tree.Fill();
    }
    std::cout << std::endl;

    //Task -- Write out the objects we created in the correct directories and close the file
    tree_dir->cd();
    tree.Write();
    hist_dir->cd();
    hist.Write();
    hist_weighted.Write();

    std::cout << "Done" << std::endl;

    // Exit with success code
    return 0;
}
