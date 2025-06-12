// Includes some helpful utilities for this exercise
#include "example_utils.h"

// Standard c++/c includes
#include <vector>


// Task -- Add the proper includes here


int main(int argc, char ** argv) {

    //Parse arguments
    Config args;
    if (!ParseArgs(argc, argv, args)) return 1;

    std::cout << "Will read a TFile with the name " << args.input_file << std::endl;
    // Task -- Make a read-only TFile with the name given by 'args.input_file' (a std::string)

    //Task -- extract the TTree under TreeDir, and the 2 hists under HistDir

    //Task -- print out the sum of entry counts in both histograms

    //Task -- Make a new writable TFile to store the histograms 
    //     -- The file's name will come from args.output_file

    //Task -- make a new TH1D named "length" w/ 40 bins from 20 to 40

    //Task -- Access the following TBranches
    //          -- momentum (double)
    //          -- my_vector (vector of doubles)

    //We'll read args.nevents (or the max number of entries) from the tree
    int nentries =  ((tree->GetEntries() < args.nevents) ? tree->GetEntries() : args.nevents);
    for (int ievent = 0; ievent < nentries; ++ievent) {

        //Just some output to track progress
        if (!(ievent % 100))
            std::cout << "\rProcessing event " << ievent << std::flush;

        //Task -- Access the next event

        //We'll simulate the particle traveling some distance according to its momentum
        double distance = LengthModel(momentum);
        std::cout << distance << std::endl;
    
        //Task -- fill the length histogram with that distance
    }
    std::cout << std::endl;

    //Make this for later
    TH2D weights_vs_momentum("weights_vs_momentum", "", 100, 3, 7, 100, 0, 5.);

    //Task -- From the loaded TTree, draw the 2D distribution of weights vs momentum,
    //     -- and store it in the histogram we just created called "weights_vs_momentum".

    //Task -- Write out the hists (length & weights_vs_momentum) we created in the output file and close the file

    std::cout << "Done" << std::endl;

    // Exit with success code
    return 0;
}
