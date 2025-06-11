#include <iostream>
#include <vector>

const int RED = 0, WHITE = 1, BLUE = 2;

/*
Helper function of quick sort function. Decides on a pivot. If arr[j] is less than pivot, arr[i] and arr[j].
*/
int partition(std::vector<int>& arr, int low, int high){
    //Choose a pivot
    int pivot = arr[high];

    //Index of smaller element
    int i = low - 1;

    for(int j = low; j <= high; j ++){
        if(arr[j] < pivot){
            i++;

            //Swap i and j
            std::swap(arr[i], arr[j]);
        }
    }

    // Move pivot after smaller elements and
    // return its position
    std::swap(arr[i + 1], arr[high]);  
    return i + 1;
}

/*
* Based on the classic quick sort algorithm. Quick Sort is an in-place sorting alogrithm that limits
* the amount of space required for sorthing, while being faster than bubble sort. Other sort algorithms
* can be used.
*/
void quick_sort_colors(std::vector<int>& arr, int low, int high){
    if (low < high){
        int part = partition(arr, low, high);

        quick_sort_colors(arr, low, part - 1);
        quick_sort_colors(arr, part + 1, high);
    }
}


void sort_colors(std::vector<int>& colors){
    quick_sort_colors(colors, 0, colors.size() -1);
}


/*
* Helper function to simply print color names instead of the numbers. You can edit this to
* add more colors. 
*/
void print_colors(std::vector<int>& colors){
    for (auto c: colors){
        switch (c){
            case RED: 
                std::cout << "RED ";
                break;
            case WHITE: 
                std::cout << "WHITE ";
                break;
            case BLUE: 
                std::cout << "BLUE " ;
                break;
            default: 
                std::cout << "UNKNOWN_COLOR " ;
                break;
        }
    }
}




//Program expects a sequence of numbers. Numbers are read as strings when they are arguements 
//in the terminal, so they are converted first from an arry of strings to a vector of ints
int main(int argc, char* argv[]){
    std::vector<int> colors; 

    for(int i = 1; i < argc; i++){
        //Coverts a string to integer
        colors.push_back(std::stoi(argv[i]));
    }

    std::cout << "Before ordering:: ";
    print_colors(colors);
    std::cout << std::endl;

    sort_colors(colors);

    std::cout << "After ordering:: ";
    print_colors(colors);
    std::cout << std::endl;
    
}