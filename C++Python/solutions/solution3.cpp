#include <iostream>
#include <vector>
#include <algorithm>


/* This solution takes advantage of the fact that values is sorted. It uses a sliding window technique
* to solve the problem. The sum of values[i] <= values [j] <= values[k] is evaluated. If the sum is 
* less than 0, j increases. If the sum is more than 0, k is decremented. If the sum is 0, the combination
* is added to three_sums if it is unique before j increases. Once j == k, the process repeats but with i = * i+1.
*/
std::vector<std::vector<int>> three_sum(std::vector<int> &values){
    //Storage for the collection of valid triplets
   std::vector<std::vector<int>> three_sums; 

    int total, last_j;
    std::vector<int> ans;
    
    
    for(int i = 0; i < values.size(); i++){
        //Set j  and k
        int j = i + 1;
        int k = values.size() - 1;

        while (j < k){
        
            //Step 1: Get the sum of numbers at indices i, j and k
            total = values.at(i) + values.at(j) + values.at(k);

            //Step 2: Compare total. If total = 0, indices are different, and triple is unique, add triple to three_sums.
            if (total == 0  && (i!=j && i!=k && j!=k)){
                ans = {values[i], values[j], values[k]};

                //If ans is not already in three_sums, add it
                auto it = std::find(three_sums.begin(), three_sums.end(), ans);
                if (it == three_sums .end()) {
                    three_sums.push_back(ans);
                }
                
                //Move values[j] until it is different (Avoid duplicates)
                last_j = values[j];
                while (last_j == values[j] && j < k){
                    j++; 
                }
            } else if (total < 0){ //If total < 0, add 1 to j.
                j++;
            } else { //If total > 0, subtract 1 to k. 
                k--;
            }

        } 
        
    }
    
    return three_sums;
}

int main(int argc, char* argv[]){
    std::vector<int> values; 

    for(int i = 1; i < argc; i++){
        values.push_back(std::stoi(argv[i]));
    }

    //Step 0: Sort the vector for increasing order
    std::sort(values.begin(), values.end());

    auto answer = three_sum(values);

    std::cout << "Given Values:: ";
    for(auto val : values){
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << "Found 3Sums:: ";
    for (const auto& triplet : answer) {
        std::cout << "[ ";
        for (int num : triplet) {
            std::cout << num << " ";
        }
        std::cout << "]\n";
    }
    
    return 0;
}


