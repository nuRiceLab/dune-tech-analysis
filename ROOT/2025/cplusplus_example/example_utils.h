#include <iostream>
#include <string>
#include <cstring>
#include "TMath.h"


struct Config {
    std::string output_file{""}, input_file{""};
    int nevents = 10;
};

double LengthModel(double p) {
    return p / .2; //The particle loses 2 MeV/c in momentum per cm -- convert to m
}

double GausPDF(double p, double mu=5., double sigma=.1) {
    return (1/sqrt(2*TMath::Pi()*sigma*sigma)) * exp(-0.5*std::pow((p - mu), 2)/(sigma*sigma));
}
 
double GetEventWeight(double p) {

    return GausPDF(p, 5.1, 0.1)/GausPDF(p, 5.0, .3);
}

bool ParseArgs(int argc, char ** argv, Config & args, bool do_read=true) {
    bool found_output = false, found_input = false;
    for (int i = 1; i < argc; ++i) {
        if( ( strcmp( argv[i], "--help" )  == 0 ) || ( strcmp( argv[i], "-h" ) == 0 ) ){

            if (do_read)
                std::cout << "Usage: ./reading_data -i input_file -o output_file -n nevents (default:10)" << std::endl;
            else
                std::cout << "Usage: ./writing_data -o output_file -n nevents (default:10)" << std::endl;
            std::cout << std::endl;

            return false;
        }

        if (!strcmp(argv[i], "-o")) {
            args.output_file = argv[i+1];
            found_output = true;
        }

        if (!strcmp(argv[i], "-i")) {
            args.input_file = argv[i+1];
            found_input = true;
        }

        if (!strcmp(argv[i], "-n")) {
            int n = std::atoi(argv[i+1]);
            if (n < 0) {
                std::cerr << "Need to provide positive nevents" << std::endl;
                return false;
            }

            args.nevents = n;
        }
    }
    if (!found_output) {
        std::cerr << "Need to provide output file via '-o' " << std::endl;
        return false;
    }
    if (!found_input && !do_read) {
        std::cerr << "Need to provide input file via '-i' " << std::endl;
        return false;
    }


    return true;
}