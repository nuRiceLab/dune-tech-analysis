// Includes some helpful utilities for this exercise
#include "example_utils.h"

// Standard c++/c includes
#include <vector>


// Task -- Add the proper includes here


int main(int argc, char ** argv) {

    //Parse arguments
    Config args;
    if (!ParseArgs(argc, argv, args)) return 1;

    std::cout << "Will write a TFile with the name " << args.output_file << std::endl;
    // Task -- Make a writable TFile with the name given by 'args.output_file' (a std::string)


    //Task -- Make 2 TDirectories: 1 named 'TreeDir', 1 named 'HistDir

    //Task -- Make a TTree named "tree" within TreeDir

    //Task -- Make 2 TH1Ds. 1 named "hist1D" 1 named "weighted1D" within HistDir, Each has 50 bins, in the 3, 7 range

    
    //Task -- Set those hist's axis titles to 'Momentum [GeV/c]' for x and 'Entries' for y"
    std::string xtitle = "Momentum [GeV/c]",
                ytitle = "Entries";

    //Task -- Make a TRandom3 object initialized with seed = 12345

    //Task -- Make some TBranches
    //     -- The following are required
    //          -- momentum (double)
    //          -- event_weight (double)
    //          -- poisson_number (int)
    //          -- my_vector (vector of doubles)


    //We'll make args.nevents number of events within this tree
    for (size_t ievent = 0; ievent < args.nevents; ++ievent) {

        //Just some output to track progress
        if (!(ievent % 100))
            std::cout << "\rProcessing event " << ievent << std::flush;

        //Task -- Generate 2 random numbers:
        //     -- a) from a Gaussian with mean 5 and width .5
        //     -- b) an integer from a Poisson distribution -- you choose the mean

        //We'll generate some event weight to be used later
        double w = GetEventWeight(p);
    
        //Task -- fill the histograms with that same gaussian random number from above
        //     -- hist1D will be filled unweighted, weighted1D will be filled with the weight 
        //     -- from above

        //Task -- set the values of the tree's branches with the relevant values from above
        //     -- Add entries to my_vector based off of the poisson number you generated (you choose their values)
        //     -- Make sure you clear any vector fields first so they don't keep accruing in size!

        //Task -- Write the event to the TTree
    }
    std::cout << std::endl;

    //Task -- Write out the objects we created in the correct directories and close the file

    std::cout << "Done" << std::endl;

    // Exit with success code
    return 0;
}
