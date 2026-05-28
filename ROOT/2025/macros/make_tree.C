double GetInteractionProbabiltiy(const double & p, double central=5., double width=.1) {
    
    //3 sigma below -- always interacts
    if (p < (central - 3*width)) return 1.;

    //3 sigma above -- never interacts
    if (p > (central + 3*width)) return 0.;

    //Probability Linearly decreases from -3 to +3 sigma
    return 1 - (1./(6*width)) * (p - (central - 3*width));
}


struct leaf_example {
    unsigned long long r1 = 0;
    double r2 = 0;
    int r3 = 0;  
};

void make_tree(
    std::string outname="example_tree.root",
    int nentries=1000
) {

    //Make a random number generator 
    TRandom3 rng(0);

    //Make the output file and TTree
    TFile fout(outname.c_str(), "recreate");
    TTree tree("tree", "");

    //Make some objects we'll use for branches
    int event_number = 0;
    double momentum = 0.;
    int nproducts = 0;
    bool interacted = false;
    std::string interaction_type = "";
    leaf_example mybranch;
    std::vector<double> product_rs;


    //Actually set up the branches
    tree.Branch("event_number", &event_number);
    tree.Branch("momentum", &momentum);
    tree.Branch("nproducts", &nproducts);
    tree.Branch("interacted", &interacted);
    tree.Branch("interaction_type", &interaction_type);
    tree.Branch("mybranch", &mybranch, "r1/l:r2/D:r3/I");
    tree.Branch("product_rs", &product_rs);
    
    //Do a simple Toy MC
    //We'll use the event_number for the loop condition
    //  -- This not necessary, but illustrative
    for (event_number = 0; event_number < nentries; ++event_number) {

        if (!(event_number%1000))
            std::cout << "Processing " << event_number << std::endl;

        //Momentum will be sample from a Gaussian
        momentum = rng.Gaus(5.0, 0.10);

        //Get int. probability
        double interaction_probability = GetInteractionProbabiltiy(momentum);

        //Check if it interacts
        interacted = (rng.Uniform() < interaction_probability);

        if (interacted) {
            //Number of interaction products follows a falling exponential
            nproducts = std::floor(rng.Exp(10.));
            for (int i = 0; i < nproducts; ++i)
                product_rs.push_back(rng.Gaus());

            //75% chance of inelastic interaction
            //25% chance of elastic interaction
            interaction_type = (
                (rng.Uniform() < .75) ?
                "Inelastic" : "Elastic"
            );
        }
        else { //Defaults
            nproducts = 0;
            interaction_type = ""; 
        }
        
        //Set the leaves of our custom branch
        mybranch.r1 = rng.Poisson(3.141592654);
        mybranch.r2 = rng.Gaus(42);
        mybranch.r3 = rng.Binomial(10, .666);


        //Add the entry to the Tree
        tree.Fill();

        //Reset the branch
        product_rs.clear();
    }

    //Write out the tree and close the file
    tree.Write();
    fout.Close();
}