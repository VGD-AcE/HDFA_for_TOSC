#include <stdio.h>
#include <windows.h>
#include <time.h> 
#include <stdint.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <set>
#include <algorithm>
#include <random>
#include <iterator>
#include <numeric>
#include <chrono>
#include <iomanip> // Include header for std::setprecision and std::fixed
#include "libxl.h"

int Sbox[64] = { 0 };     // Store the correct input values for Ascon's finalization S-box in the last round

#define ASCON_128A_IV                         \
  (((uint64_t)(1) << 0) |    \
   ((uint64_t)(12) << 16) |      \
   ((uint64_t)(8) << 20) | \
   ((uint64_t)(16 * 8) << 24) |   \
   ((uint64_t)(16) << 40))

/* get byte from 64-bit Ascon word */
#define GETBYTE(x, i) ((uint8_t)((uint64_t)(x) >> (8 * (i))))

/* set byte in 64-bit Ascon word */
#define SETBYTE(b, i) ((uint64_t)(b) << (8 * (i)))

/* set padding byte in 64-bit Ascon word */
#define PAD(i) SETBYTE(0x01, i)

/* define domain separation bit in 64-bit Ascon word */
#define DSEP() SETBYTE(0x80, 7)

/* load bytes into 64-bit Ascon word */
static inline uint64_t LOADBYTES(const uint8_t* bytes, int n) {
    int i;
    uint64_t x = 0;
    for (i = 0; i < n; ++i) x |= SETBYTE(bytes[i], i);
    return x;
}

/* store bytes from 64-bit Ascon word */
static inline void STOREBYTES(uint8_t* bytes, uint64_t x, int n) {
    int i;
    for (i = 0; i < n; ++i) bytes[i] = GETBYTE(x, i);
}

typedef struct {
    uint64_t x[5];
} ascon_state_t;

void uint64_to_bin(uint64_t val, int bin[64]);
void print_bin_array(const char* name, const int bin[64]);

static int is_final_p12 = 0;

static inline uint64_t ROR(uint64_t x, int n) {
    return x >> n | x << (-n & 63);
}

static inline void ROUND(ascon_state_t* s, uint8_t C) {
    static int printed_correct_tag_once = 0;
    ascon_state_t t;
    s->x[2] ^= C;

    if (C == 0x4b && is_final_p12) {                   // 0x4b refers to the last permutation in p12
        //printf("\n>>>> FINAL ROUND BEFORE S-BOX:\n");
        //for (int i = 0; i < 5; ++i) printf("s->x[%d] = 0x%016llX\n", i, s->x[i]);

        int bin_x0[64], bin_x1[64], bin_x2[64], bin_x3[64], bin_x4[64];
        uint64_to_bin(s->x[0], bin_x0);
        uint64_to_bin(s->x[1], bin_x1);
        uint64_to_bin(s->x[2], bin_x2);
        uint64_to_bin(s->x[3], bin_x3);
        uint64_to_bin(s->x[4], bin_x4);

        //printf(">>>> BINARY STATE BEFORE S-BOX:\n");
        //print_bin_array("x0_bin", bin_x0);
        //print_bin_array("x1_bin", bin_x1);
        //print_bin_array("x2_bin", bin_x2);
        //print_bin_array("x3_bin", bin_x3);
        //print_bin_array("x4_bin", bin_x4);

        int fivebit_vals[64];
        for (int i = 0; i < 64; ++i) {
            fivebit_vals[i] = (bin_x0[i] << 4) | (bin_x1[i] << 3) | (bin_x2[i] << 2) |
                (bin_x3[i] << 1) | bin_x4[i];
        }

        //        printf(">>>> 5-BIT COLUMN VALUES:\n");
                                                            //Extract the intermediate state of Ascon running to the 
                                                            //finalization stage before the last S-box to prepare 
        for (int i = 0; i < 64; ++i) {              //for fault injection in the following text
            //            printf("%02d", fivebit_vals[i]);
            Sbox[i] = fivebit_vals[i];
            //if ((i + 1) % 16 == 0)
            //    printf("\n");
            //else
            //    printf(" ");
        }
    }

    // S-BOX and diffusion layer
    s->x[0] ^= s->x[4];
    s->x[4] ^= s->x[3];
    s->x[2] ^= s->x[1];

    t.x[0] = s->x[0] ^ (~s->x[1] & s->x[2]);
    t.x[1] = s->x[1] ^ (~s->x[2] & s->x[3]);
    t.x[2] = s->x[2] ^ (~s->x[3] & s->x[4]);
    t.x[3] = s->x[3] ^ (~s->x[4] & s->x[0]);
    t.x[4] = s->x[4] ^ (~s->x[0] & s->x[1]);

    t.x[1] ^= t.x[0];
    t.x[0] ^= t.x[4];
    t.x[3] ^= t.x[2];
    t.x[2] = ~t.x[2];

    s->x[0] = t.x[0] ^ ROR(t.x[0], 19) ^ ROR(t.x[0], 28);
    s->x[1] = t.x[1] ^ ROR(t.x[1], 61) ^ ROR(t.x[1], 39);
    s->x[2] = t.x[2] ^ ROR(t.x[2], 1) ^ ROR(t.x[2], 6);
    s->x[3] = t.x[3] ^ ROR(t.x[3], 10) ^ ROR(t.x[3], 17);
    s->x[4] = t.x[4] ^ ROR(t.x[4], 7) ^ ROR(t.x[4], 41);
}

static void P12(ascon_state_t* s) {
    ROUND(s, 0xf0);
    ROUND(s, 0xe1);
    ROUND(s, 0xd2);
    ROUND(s, 0xc3);
    ROUND(s, 0xb4);
    ROUND(s, 0xa5);
    ROUND(s, 0x96);
    ROUND(s, 0x87);
    ROUND(s, 0x78);
    ROUND(s, 0x69);
    ROUND(s, 0x5a);
    ROUND(s, 0x4b);
}

static inline void P8(ascon_state_t* s) {
    ROUND(s, 0xb4);
    ROUND(s, 0xa5);
    ROUND(s, 0x96);
    ROUND(s, 0x87);
    ROUND(s, 0x78);
    ROUND(s, 0x69);
    ROUND(s, 0x5a);
    ROUND(s, 0x4b);
}

void print_tag_info(uint64_t tag_hi, uint64_t tag_lo);

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
    const unsigned char* m, unsigned long long mlen,
    const unsigned char* ad, unsigned long long adlen,
    const unsigned char* nsec, const unsigned char* npub,
    const unsigned char* k) {
    (void)nsec;

    /* set ciphertext size */
    *clen = mlen + 16;

    /* load key and nonce */
    const uint64_t K0 = LOADBYTES(k, 8);
    const uint64_t K1 = LOADBYTES(k + 8, 8);
    const uint64_t N0 = LOADBYTES(npub, 8);
    const uint64_t N1 = LOADBYTES(npub + 8, 8);

    /* initialize */
    ascon_state_t s;
    s.x[0] = ASCON_128A_IV;
    s.x[1] = K0;
    s.x[2] = K1;
    s.x[3] = N0;
    s.x[4] = N1;

    P12(&s);
    s.x[3] ^= K0;
    s.x[4] ^= K1;

    if (adlen) {
        /* full associated data blocks */
        while (adlen >= 16) {
            s.x[0] ^= LOADBYTES(ad, 8);
            s.x[1] ^= LOADBYTES(ad + 8, 8);

            P8(&s);
            ad += 16;
            adlen -= 16;
        }
        /* final associated data block */
        if (adlen >= 8) {
            s.x[0] ^= LOADBYTES(ad, 8);
            s.x[1] ^= LOADBYTES(ad + 8, adlen - 8);
            s.x[1] ^= PAD(adlen - 8);
        }
        else {
            s.x[0] ^= LOADBYTES(ad, adlen);
            s.x[0] ^= PAD(adlen);
        }

        P8(&s);
    }
    /* domain separation */
    s.x[4] ^= DSEP();

    /* full plaintext blocks */
    while (mlen >= 16) {
        s.x[0] ^= LOADBYTES(m, 8);
        s.x[1] ^= LOADBYTES(m + 8, 8);
        STOREBYTES(c, s.x[0], 8);
        STOREBYTES(c + 8, s.x[1], 8);

        P8(&s);
        m += 16;
        c += 16;
        mlen -= 16;
    }
    /* final plaintext block */
    if (mlen >= 8) {
        s.x[0] ^= LOADBYTES(m, 8);
        s.x[1] ^= LOADBYTES(m + 8, mlen - 8);
        STOREBYTES(c, s.x[0], 8);
        STOREBYTES(c + 8, s.x[1], mlen - 8);
        s.x[1] ^= PAD(mlen - 8);
    }
    else {
        s.x[0] ^= LOADBYTES(m, mlen);
        STOREBYTES(c, s.x[0], mlen);
        s.x[0] ^= PAD(mlen);
    }
    m += mlen;
    c += mlen;

    /* finalize */
    s.x[2] ^= K0;
    s.x[3] ^= K1;

    is_final_p12 = 1;  // Enable the flag, only effective for this P12
    P12(&s);
    is_final_p12 = 0;  // Close immediately after use to prevent contamination of other P12

    s.x[3] ^= K0;
    s.x[4] ^= K1;

    /* get tag */
    STOREBYTES(c, s.x[3], 8);
    STOREBYTES(c + 8, s.x[4], 8);
    print_tag_info(s.x[3], s.x[4]);

    return 0;
}

// Convert the 64-bit unsigned integer val to a binary array (64 0s or 1s), 
// with the most significant bit first, and write the result to bin[64].
void uint64_to_bin(uint64_t val, int bin[64]) {
    for (int i = 0; i < 64; ++i) {
        bin[i] = (val >> (63 - i)) & 1;
    }
}

// Format and print a 64-bit binary array bin[] grouped in 8-bit chunks, with variable names
void print_bin_array(const char* name, const int bin[64]) {
    printf("%s = ", name);
    for (int i = 0; i < 64; ++i) {
        printf("%d", bin[i]);
        if ((i + 1) % 8 == 0) printf(" ");
    }
    //    printf("\n");
}

// Print the hexadecimal and binary forms of the tag
void print_tag_info(uint64_t tag_hi, uint64_t tag_lo) {

    int bin_hi[64], bin_lo[64];
    uint64_to_bin(tag_hi, bin_hi);
    uint64_to_bin(tag_lo, bin_lo);

}

// Randomly generate a byte array of a specified length.
void generate_random_data(unsigned char* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        data[i] = rand() % 256;  // 
    }
}

// === standard test vector ===
unsigned char key[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
unsigned char nonce[16];  //  The nonce length is fixed at 16 bytes (length cannot be changed).
unsigned long long mlen = 32;  // msg length is 32 bytes (length can be changed)
unsigned char msg[32];
unsigned long long adlen = 32;  // ad length is 32 bytes (length can be changed)
unsigned char ad[32];
unsigned char ciphertext[64] = { 0 };
unsigned long long clen = 0;

using namespace libxl;

// Define a struct for the result that includes an integer and a double, allowing multiple results to be returned (minimum fault injection rounds and average nibble faults for recovering 64 S-box values in a single experiment)
struct Result {
    int returnFaultRound;
    double returnFaultNibble;
};

int Ascon[32] = { 4,11,31,20,26,21,9,2,27,5,8,18,29,3,6,28,30,19,7,14,0,13,17,24,16,12,1,25,22,10,15,23 }; // Ascon S-box 

int f_Sbox[64] = { 0 };   // Store the incorrect input values for Ascon's finalization S-box after injecting a fault (kicked off 3rd bit)
int fault[64] = { 0 };    // Store the injected nibble fault values for Ascon's finalization S-box

int fivebit_vals[64] = { 0 };

// Function to set the S-box with random values
// This function simulates an S-box fault injection operation
// during the final round p-permutation phase in the Finalization stage.
// It first generates random S-box input values for testing fault injection.
void set_Sbox(int Sbox[]) {

    generate_random_data(nonce, 16);  // Randomly generate a 16-byte nonce
    generate_random_data(msg, mlen);  // Generate msg content randomly based on mlen
    generate_random_data(ad, adlen);  // Generate ad content randomly based on adlen
    // === Encryption ===
    crypto_aead_encrypt(ciphertext, &clen, msg, mlen, ad, adlen, NULL, nonce, key);
}

// Function to set the fault array with random values
void set_fault(int F[]) {
    std::random_device rd;
    std::mt19937 gen(rd());

    // Uniform fault injection
    std::uniform_int_distribution<int> distribution(0, 31);

    for (int i = 0; i < 64; ++i) {
        int randomNum = distribution(gen);
        F[i] = randomNum;
    }

    // Custom fault injection scenario (commented out)
    /*
    std::vector<int> customDistribution = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 0: 1/6
        1, 2, 4, 8, 16, 1, 2, 4, 8, 16,   // 1/6 * 1/5
        3, 5, 6, 9, 10, 12, 17, 18, 20, 24,   // 1/6 * 1/10
        7, 11, 13, 14, 19, 21, 22, 25, 26, 28, // 1/6 * 1/10
        15, 23, 27, 29, 30, 15, 23, 27, 29, 30, // 1/6 * 1/5
        31, 31, 31, 31, 31, 31, 31, 31, 31, 31  // 1/6
    };

    std::uniform_int_distribution<int> distribution(0, customDistribution.size() - 1);
    for (int i = 0; i < 64; ++i) {
        int randomIndex = distribution(gen);
        F[i] = customDistribution[randomIndex];
    }
    */
}

// Function to calculate the intersection of two sets
std::vector<int> calculateIntersection(const std::vector<int>& set1, const std::vector<int>& set2) {
    std::vector<int> intersection;

    // Use set_intersection to calculate the intersection
    std::set_intersection(
        set1.begin(), set1.end(),
        set2.begin(), set2.end(),
        std::back_inserter(intersection)
    );

    return intersection;
}

// Function to perform the Ascon fault injection trial
Result Ascon_trial(Sheet* sheet, int Num) {
    int out2;

    // Define a 3D dynamic array: 32x4x?
    std::vector<std::vector<std::vector<int>>> differ_LSB_2(32, std::vector<std::vector<int>>(4));

    // Solve the differential equation
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int in = 0; in < 32; in++) {
                out2 = Ascon[in] ^ Ascon[i & in];
                if (j == out2) {
                    differ_LSB_2[i][j % 4].push_back(in);
                }
            }
            std::sort(differ_LSB_2[i][j % 4].begin(), differ_LSB_2[i][j % 4].end()); // Sort the sets (important)
        }
    }

    // Set the S-box
    set_Sbox(Sbox);

    std::wstring S;
    for (int i = 0; i < 64; ++i) {
        S += std::to_wstring(Sbox[i]) + L",";
    }
    sheet->writeStr(Num, 1, S.c_str());

    const int COUNT = 100;  // Maximum of fault injections
    int count;
    int temp = 0;

    // Define a 3D dynamic array: COUNT x 64 x ?, to store the set of possible S-box values after each fault injection
    std::vector<std::vector<std::vector<int>>> Intersection(COUNT, std::vector<std::vector<int>>(64));
    // Define a 2D dynamic array: 64 x ?, to store the intersection of two rows of S-box possible values
    std::vector<std::vector<int>> Intersec(64);

    std::wstring Count_and;
    std::wstring Count_all;

    // Define Countand array to store the random-and fault injection counts for each S-box possible value
    int Countand[64] = { 0 };

    for (count = 0; count < COUNT; count++) {
        // Set the fault
        set_fault(fault);

        // Print the injected fault values
        std::wstring f;
        for (int i = 0; i < 64; ++i) {
            f += std::to_wstring(fault[i]) + L",";
        }
        sheet->writeStr(Num, 5 + count * 3, f.c_str());

        // Inject faults into the S-box
        for (int i = 0; i < 64; ++i) {
            f_Sbox[i] = Sbox[i] & fault[i];
        }

        // Print the S-box output differences
        std::wstring dif;
        for (int i = 0; i < 64; ++i) {
            dif += std::to_wstring(Ascon[Sbox[i]] ^ Ascon[f_Sbox[i]]) + L",";
        }
        sheet->writeStr(Num, 6 + count * 3, dif.c_str());

        // Fill the array with intersections
        for (int i = 0; i < 64; ++i) {
            Intersection[count][i] = differ_LSB_2[fault[i]][(Ascon[Sbox[i]] ^ Ascon[f_Sbox[i]]) % 4];
        }

        // Initialize the intersection array on the first fault injection
        std::wstring Sb;
        if (count == 0) {
            for (int i = 0; i < 64; ++i) {
                Intersec[i] = Intersection[0][i];
                Sb += L"{";
                for (int j = 0; j < Intersec[i].size(); ++j) {
                    Sb += std::to_wstring(Intersec[i][j]) + L" ";
                }
                Sb += L"},";

                sheet->writeStr(Num, 7, Sb.c_str());
            }
        }

        // For subsequent fault injections, calculate the intersection with existing S-box possible values
        std::wstring Sbb;
        if (count > 0) {
            for (int i = 0; i < 64; ++i) {
                Intersec[i] = calculateIntersection(Intersection[count][i], Intersec[i]);
                Sbb += L"{";
                for (int j = 0; j < Intersec[i].size(); ++j) {
                    Sbb += std::to_wstring(Intersec[i][j]) + L" ";
                }
                Sbb += L"},";

                sheet->writeStr(Num, 7 + count * 3, Sbb.c_str());
                temp += Intersec[i].size();

                if ((Intersec[i].size() == 1) && (Countand[i] == 0)) { // When the first S-box possible value becomes unique, record the fault round
                    Countand[i] = count + 1;
                }
            }

            if (temp == 64) {
                break; // End experiment if all S-box values are uniquely determined
            }
            else {
                temp = 0;
            }
        }
    }

    // Calculate averages
    int sum2 = 0;
    for (int i = 0; i < 64; ++i) {
        Count_and += std::to_wstring(Countand[i]) + L",";
        sum2 += Countand[i];
    }

    double Average_Nibble2 = double(sum2) / 64;
    Count_and += L"\nAverage random-and faults for 64 S-boxes: " + std::to_wstring(Average_Nibble2);

    // Output the results to Excel
    sheet->writeStr(Num, 4, Count_and.c_str());

    Result result;
    result.returnFaultRound = count + 1;
    result.returnFaultNibble = Average_Nibble2;

    return result;
}

int main() {
    // Start timing
    auto start = std::chrono::high_resolution_clock::now();

    srand((unsigned int)time(NULL));  // Add initialization

    const int trial_Num = 10000; // Number of trials in each group of experiments
    int Count[trial_Num] = { 0 };
    double Countnibble[trial_Num] = { 0 };
    int temp = 0;
    double temp1 = 0;

    // Create an Excel document object
    libxl::Book* book = xlCreateBook();
    book->setKey(L"libxl", L"windows-28232b0208c4ee0369ba6e68abv6v5i3");
    if (book) {
        libxl::Sheet* sheet = book->addSheet(L"Sheet1");

        // Set table row titles
        sheet->writeStr(0, 1, L"5-bit S-box values");
        sheet->writeStr(0, 2, L"Total fault injection rounds to recover each S-box input value");
        sheet->writeStr(0, 3, L"Total random-xor fault injections for each S-box input value");
        sheet->writeStr(0, 4, L"Total random-and fault injections for each S-box input value");

        for (int i = 0; i < 100; ++i) { // Set a maximum of 100 fault injections per experiment group
            std::wstring i_str = std::to_wstring(i + 1);

            std::wstring output_str_1 = L"Experiment " + i_str + L" 5-bit fault values";
            std::wstring output_str_2 = L"Experiment " + i_str + L" S-box output differences";
            std::wstring output_str_3 = L"Experiment " + i_str + L" S-box possible input value sets";

            sheet->writeStr(0, 5 + 3 * i, output_str_1.c_str());
            sheet->writeStr(0, 6 + 3 * i, output_str_2.c_str());
            sheet->writeStr(0, 7 + 3 * i, output_str_3.c_str());
        }

        for (int i = 0; i < trial_Num; ++i) {
            std::wstring i_str = std::to_wstring(i + 1);
            std::wstring output_str = L"Experiment #" + i_str + L":";
            sheet->writeStr(i + 1, 0, output_str.c_str());

            Result result = Ascon_trial(sheet, i + 1);
            Count[i] = result.returnFaultRound;
            Countnibble[i] = result.returnFaultNibble;
            temp += Count[i];
            temp1 += Countnibble[i];
        }

        // End timing
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // Calculate average fault injection rounds and fault nibble
        double Average = double(temp) / double(trial_Num);
        double Averagenibble = double(temp1) / double(trial_Num);
        std::wstring newStr = L"Average fault injection rounds: " + std::to_wstring(Average) + L" Average fault nibble: " + std::to_wstring(Averagenibble) + L" Total time: " + std::to_wstring(duration.count() / 1'000'000.0) + L"s.";
        std::wcout << L"Average fault injection rounds: " << Average << std::endl;
        std::wcout << L"Average fault nibble: " << Averagenibble << std::endl;
        sheet->writeStr(0, 0, newStr.c_str());

        // Save the Excel file
        book->save(L"Ascon_random-and_trials.xlsx");

        // Release resources
        book->release();
        std::cout << "Excel file generated successfully!" << std::endl;
    }
    else {
        std::cerr << "Unable to create Excel document object" << std::endl;
    }

    system("pause");
    return 0;
}
