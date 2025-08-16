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
                out = S[in] ^ S[i & in];  // AND operation
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

    // Output the best fault trail count for each S-box input value Num
    double totalRecoveryProbability = 0.0; // To calculate total recovery probability
    int totalCount = 0; // To calculate total fault path count

    int n;  // Number of fault injections
    std::cout << "Please enter the number of fault injections (between 2 and 9): ";
    std::cin >> n;  // Input the number of fault injections

    for (int Num = 0; Num < 32; ++Num) {
        std::cout << "\nBest fault trails for S-box input " << Num << ": \n";

        int bestFaultTrailCount = 0;

        if (n == 2) {
            // Calculate two fault injections
            for (int i = 0; i < 32; ++i) {
                for (int j = 0; j < 32; ++j) {
                    // Use columnsWith_Num to select different columns for intersection calculation
                    std::vector<int> intersection = calculateIntersection(Possible_input[i][columnsWith_Num[Num][i]], Possible_input[j][columnsWith_Num[Num][j]]);
                    if (intersection.size() == 1) {
                        bestFaultTrailCount++;
                    }
                }
            }
        }
        else if (n == 3) {
            // Calculate three fault injections
            for (int i = 0; i < 32; ++i) {
                for (int j = 0; j < 32; ++j) {
                    for (int k = 0; k < 32; ++k) {
                        // Use columnsWith_Num to select different columns for intersection calculation
                        std::vector<int> firstIntersection = calculateIntersection(Possible_input[i][columnsWith_Num[Num][i]], Possible_input[j][columnsWith_Num[Num][j]]);
                        std::vector<int> finalIntersection = calculateIntersection(firstIntersection, Possible_input[k][columnsWith_Num[Num][k]]);

                        // If the intersection size is 1, we found a best fault value
                        if (finalIntersection.size() == 1) {
                            bestFaultTrailCount++;
                        }
                    }
                }
            }
        }
        else if (n == 4) {
            // Calculate four fault injections
            for (int i = 0; i < 32; ++i) {
                for (int j = 0; j < 32; ++j) {
                    for (int k = 0; k < 32; ++k) {
                        for (int l = 0; l < 32; ++l) {
                            // Use columnsWith_Num to select different columns for intersection calculation
                            std::vector<int> firstIntersection = calculateIntersection(Possible_input[i][columnsWith_Num[Num][i]], Possible_input[j][columnsWith_Num[Num][j]]);
                            std::vector<int> secondIntersection = calculateIntersection(firstIntersection, Possible_input[k][columnsWith_Num[Num][k]]);
                            std::vector<int> finalIntersection = calculateIntersection(secondIntersection, Possible_input[l][columnsWith_Num[Num][l]]);

                            // If the intersection size is 1, we found a best fault value
                            if (finalIntersection.size() == 1) {
                                bestFaultTrailCount++;
                            }
                        }
                    }
                }
            }
        }
        else if (n == 5) {
            // Calculate five fault injections
            for (int i = 0; i < 32; ++i) {
                for (int j = 0; j < 32; ++j) {
                    for (int k = 0; k < 32; ++k) {
                        for (int l = 0; l < 32; ++l) {
                            for (int m = 0; m < 32; ++m) {
                                // Use columnsWith_Num to select different columns for intersection calculation
                                std::vector<int> firstIntersection = calculateIntersection(Possible_input[i][columnsWith_Num[Num][i]], Possible_input[j][columnsWith_Num[Num][j]]);
                                std::vector<int> secondIntersection = calculateIntersection(firstIntersection, Possible_input[k][columnsWith_Num[Num][k]]);
                                std::vector<int> thirdIntersection = calculateIntersection(secondIntersection, Possible_input[l][columnsWith_Num[Num][l]]);
                                std::vector<int> finalIntersection = calculateIntersection(thirdIntersection, Possible_input[m][columnsWith_Num[Num][m]]);

                                // If the intersection size is 1, we found a best fault value
                                if (finalIntersection.size() == 1) {
                                    bestFaultTrailCount++;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (n == 6) {
            // Calculate six fault injections
            for (int i = 0; i < 32; ++i) {
                for (int j = 0; j < 32; ++j) {
                    for (int k = 0; k < 32; ++k) {
                        for (int l = 0; l < 32; ++l) {
                            for (int m = 0; m < 32; ++m) {
                                for (int n2 = 0; n2 < 32; ++n2) {
                                    // Use columnsWith_Num to select different columns for intersection calculation
                                    std::vector<int> firstIntersection = calculateIntersection(Possible_input[i][columnsWith_Num[Num][i]], Possible_input[j][columnsWith_Num[Num][j]]);
                                    std::vector<int> secondIntersection = calculateIntersection(firstIntersection, Possible_input[k][columnsWith_Num[Num][k]]);
                                    std::vector<int> thirdIntersection = calculateIntersection(secondIntersection, Possible_input[l][columnsWith_Num[Num][l]]);
                                    std::vector<int> fourthIntersection = calculateIntersection(thirdIntersection, Possible_input[m][columnsWith_Num[Num][m]]);
                                    std::vector<int> finalIntersection = calculateIntersection(fourthIntersection, Possible_input[n2][columnsWith_Num[Num][n2]]);

                                    // If the intersection size is 1, we found a best fault value
                                    if (finalIntersection.size() == 1) {
                                        bestFaultTrailCount++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (n == 7) {
            // Calculate seven fault injections
            for (int i = 0; i < 32; ++i) {
                for (int j = 0; j < 32; ++j) {
                    for (int k = 0; k < 32; ++k) {
                        for (int l = 0; l < 32; ++l) {
                            for (int m = 0; m < 32; ++m) {
                                for (int n2 = 0; n2 < 32; ++n2) {
                                    for (int n3 = 0; n3 < 32; ++n3) {
                                        // Use columnsWith_Num to select different columns for intersection calculation
                                        std::vector<int> firstIntersection = calculateIntersection(Possible_input[i][columnsWith_Num[Num][i]], Possible_input[j][columnsWith_Num[Num][j]]);
                                        std::vector<int> secondIntersection = calculateIntersection(firstIntersection, Possible_input[k][columnsWith_Num[Num][k]]);
                                        std::vector<int> thirdIntersection = calculateIntersection(secondIntersection, Possible_input[l][columnsWith_Num[Num][l]]);
                                        std::vector<int> fourthIntersection = calculateIntersection(thirdIntersection, Possible_input[m][columnsWith_Num[Num][m]]);
                                        std::vector<int> fifthIntersection = calculateIntersection(fourthIntersection, Possible_input[n2][columnsWith_Num[Num][n2]]);
                                        std::vector<int> finalIntersection = calculateIntersection(fifthIntersection, Possible_input[n3][columnsWith_Num[Num][n3]]);

                                        // If the intersection size is 1, we found a best fault value
                                        if (finalIntersection.size() == 1) {
                                            bestFaultTrailCount++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (n == 8) {
            // Calculate eight fault injections
            for (int i = 0; i < 32; ++i) {
                for (int j = 0; j < 32; ++j) {
                    for (int k = 0; k < 32; ++k) {
                        for (int l = 0; l < 32; ++l) {
                            for (int m = 0; m < 32; ++m) {
                                for (int n2 = 0; n2 < 32; ++n2) {
                                    for (int n3 = 0; n3 < 32; ++n3) {
                                        for (int n4 = 0; n4 < 32; ++n4) {
                                            // Use columnsWith_Num to select different columns for intersection calculation
                                            std::vector<int> firstIntersection = calculateIntersection(Possible_input[i][columnsWith_Num[Num][i]], Possible_input[j][columnsWith_Num[Num][j]]);
                                            std::vector<int> secondIntersection = calculateIntersection(firstIntersection, Possible_input[k][columnsWith_Num[Num][k]]);
                                            std::vector<int> thirdIntersection = calculateIntersection(secondIntersection, Possible_input[l][columnsWith_Num[Num][l]]);
                                            std::vector<int> fourthIntersection = calculateIntersection(thirdIntersection, Possible_input[m][columnsWith_Num[Num][m]]);
                                            std::vector<int> fifthIntersection = calculateIntersection(fourthIntersection, Possible_input[n2][columnsWith_Num[Num][n2]]);
                                            std::vector<int> sixthIntersection = calculateIntersection(fifthIntersection, Possible_input[n3][columnsWith_Num[Num][n3]]);
                                            std::vector<int> finalIntersection = calculateIntersection(sixthIntersection, Possible_input[n4][columnsWith_Num[Num][n4]]);

                                            // If the intersection size is 1, we found a best fault value
                                            if (finalIntersection.size() == 1) {
                                                bestFaultTrailCount++;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        std::cout << "\nNumber of best fault trails for input " << Num << ": " << bestFaultTrailCount << std::endl;

        // Calculate recovery probability
        double recoveryProbability = static_cast<double>(bestFaultTrailCount) / std::pow(32, n); // 32^n
        std::cout << "Recovery probability for input " << Num << ": " << recoveryProbability << std::endl;

        // Accumulate recovery probability
        totalRecoveryProbability += recoveryProbability;
        totalCount++;
    }

    // Calculate average recovery probability
    double averageRecoveryProbability = totalRecoveryProbability / totalCount;
    std::cout << "\nAverage recovery probability for n=" << n << " (random-AND fault injection): " << averageRecoveryProbability << std::endl;

    system("pause");
    return 0;
}
