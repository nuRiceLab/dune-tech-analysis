#include <iostream>
#include <stack>
#include <utility>

using namespace std;

//Solution from LeetCode - Stack Approach
//Terms, defined by (), are placed on the stack, a FILO data structure to calculate the expression
//Sign is determined by int, everytime a minus sign is seen, the sign changes itself by multiplying
//itself by -1.
int calculate(std::string& s){
    int sum = 0;
    int sign = 1;
    stack<pair<int,int>> st;

    for(int i=0; i<s.size();i++){
        //Check for numbers
        if(isdigit(s[i])){
            long long int num = 0;
            while(i<s.size() && isdigit(s[i])){
                num = num * 10 + (s[i] - '0');
                i++;
                
            }
            i--;
            sum += num * sign;
            sign = 1;
        }
        else if(s[i] == '('){ //Check for start of terms
            //Saves the current sum by pushing it onto the stack
            //It calculates the terms sum first, then goes back to the original sum later
            st.push({sum, sign});
            sum = 0;
            sign = 1;
        }
        else if(s[i] == ')'){ //Check for the end of terms
            sum = st.top().first + (st.top().second * sum);
            st.pop();
        }
        else if(s[i] == '-'){ //Make numbers negative or check for subtraction
            sign = -1 * sign;
        }
    }
    return sum;
}





//Program expects a valid expression that uses only the basic operations of addition (+), subtraction (-) and parathesis. 
int main(){
    std::string input;

    std::cout << "Enter an expression:: ";
    std::getline(std::cin, input);

    std::cout << "Expression:: " << input;

    auto answer = calculate(input);

    std::cout << "\nAnswer:: " << answer << std::endl;

    return 0;
}