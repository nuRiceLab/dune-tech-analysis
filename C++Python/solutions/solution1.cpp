#include <iostream>

/*
Program expects two arguments - program name and roman numeral.
If Roman number is invalid (i.e. outside valid Roman numeral [1 - 3999], uncommon invalid repetition, invalid numberal order, or empty string) return -1
*/
int main(int argc, char *argv[]){
    std::string roman = argv[1];
    int number = 0;
    int i = 0;
    
    std::cout << "Roman Numeral:: " << roman << std::endl;

    //Check for Ms first
    while(i < roman.length() && (roman.at(i) == 'M')){
        if(roman.at(i) == 'M'){
            number += 1000;
        }  
        i++;
    }

    if(number>3000){
        std::cout << "ERROR:: Outside of the anticipated range.";
        return -1;
    }

    //Check for Cs (Normal case) and subtraction D and M
    while(i < roman.length() && (roman.at(i) == 'C' || roman.at(i) == 'D' || roman.at(i) == 'M')){
        if(roman.at(i) == 'C'){ //Add 100
            number += 100;
        } else if (roman.at(i) == 'M') { //900
            number -= 100;
            number += 900;
        } else if (roman.at(i) == 'L') { //400
            number -= 100;
            number += 400;
        }
        i++;
    }

    //Case for 50
    if(i < roman.length() && roman.at(i) == 'L'){
        number += 50;
        i++;
    }

    //Check for Xs and subtraciton C and L
    while(i < roman.length() && (roman.at(i) == 'X' || roman.at(i) == 'C' || roman.at(i) == 'L')){
        if(roman.at(i) == 'X'){ //Add 10
            number += 10;
        } else if (roman.at(i) == 'C') { //90
            number -= 10;
            number += 90;
        } else if (roman.at(i) == 'L') { //40
            number -= 10;
            number += 40;
        } 
        i++;
    }

    //Case for 5
    if(i < roman.length() && roman.at(i) == 'V'){
        number += 5;
        i++;
    }
    

    //Check for Is (Normal case) and subtraction X and V
    while(i < roman.length()){
        if(roman.at(i) == 'I'){ 
            number += 1;
        } else if(roman.at(i) == 'V'){ //4
            number -= 1;
            number += 4;
        } else if(roman.at(i) == 'X'){ //9
            number -= 1;
            number += 9;
        } else {
            std::cout << "ERROR:: Invalid Numeral Order\ncharacter at index " << i << std::endl;
            return -1;
        }
        i++;
    }
    
    std::cout << "Number:: " << number << std::endl;
    
    return 0;
}