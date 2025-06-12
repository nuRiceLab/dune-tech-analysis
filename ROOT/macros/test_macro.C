
void test_macro( //Same name as macro file
    std::string outname="test_macro_output.root",
    double central=1.,
    double width=1.) {

    //Make the random number generator
    TRandom3 rng(0);

    //Make a new output file -- overwrite any previous existing one
    TFile fout(outname.c_str(), "recreate");
    
    //Histogram for output
    TH1D h("hist", "", 100, central - 3*width, central + 3*width);

    //Fill it with gaussian random numbers
    std::cout << "Filling Random" << std::endl;
    for (int i = 0; i < 10000; ++i) {
        h.Fill(rng.Gaus(central, width));
    }
    std::cout << "Done" << std::endl;
    
    //Write histogram and close file
    h.Write();
    fout.Close();
}