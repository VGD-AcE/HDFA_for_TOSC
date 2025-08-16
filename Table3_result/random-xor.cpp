#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <cmath>  // Include cmath to use std::pow for power calculations

using namespace std;

char S[32] = { 4,11,31,20,26,21,9,2,27,5,8,18,29,3,6,28,
               30,19,7,14,0,13,17,24,16,12,1,25,22,10,15,23 };     // Ascon

// Function: Calculate intersection of two sets
std::vector<int> calculateIntersection(const std::vector<int>& set1, const std::vector<int>& set2) {
    std::vector<int> intersection;
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), std::back_inserter(intersection));
    return intersection;
}

int main() {
    // Define a 3D variable length array: 32x4x?
    std::vector<std::vector<std::vector<int>>> Possible_input(32, std::vector<std::vector<int>>(4));

    // Store the index of columns containing the element Num
    int columnsWith_Num[32][32] = { 0 };

    int out;
    // Solve the differential equation
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int in = 0; in < 32; in++) {
                out = S[in] ^ S[i ^ in];  // Differential distribution table after two 5-bit random-XOR fault injections
                // out = S[in] ^ S[i & in];  // Differential distribution table after two 5-bit random-AND fault injections
                if (j == out) {
                    Possible_input[i][j % 4].push_back(in);

                    // Record the index of the column where Num is located
                    columnsWith_Num[in][i] = j % 4;  // In row i, value 'in' corresponds to column j % 4
                }
            }
            std::sort(Possible_input[i][j % 4].begin(), Possible_input[i][j % 4].end());
        }
    }

    // Print the entire table (only output in terminal)
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << "Possible_input[" << i << "][" << j << "]: ";
            for (int k = 0; k < Possible_input[i][j].size(); ++k) {
                std::cout << Possible_input[i][j][k] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    std::vector<int> temp;

	for (int Num = 0; Num < 32; ++Num) {
		std::cout << "\nThe optimal fault trail when the S-box input value is " << Num << ": \n";
		for (int i = 0; i < 32; ++i) {               //First fault injection
			for (int j = 0; j < 32; ++j) {           //Second fault injection
				if (calculateIntersection(Possible_input[i][columnsWith_Num[Num][i]], Possible_input[j][columnsWith_Num[Num][j]]).size() == 2) {
					std::cout << i << " -> " << j << "\t";
				}
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}

    system("pause");
    return 0;
}
