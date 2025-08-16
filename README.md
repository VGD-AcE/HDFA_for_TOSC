# Repository Overview 🛠️🔐



The projects, data, and documentation in this repository are provided solely to support the validation of the paper **“HDFA: Hybrid Differential Fault Analysis on Ascon”**. Unauthorized use is prohibited. For inquiries, please contact the developer.

This repository contains four projects corresponding to the experimental results in the manuscript: **Table 3**, **Table 6**, **Table 8 (Fault Model A)**, and **Table 8 (Fault Model B)**. Additionally, it includes all data for **Figure 4** and **Table 8** (Excel files) and a configuration guide for the **Libxl** library required to run **Table8_result(fault model A&B)**. For validation of **Examples 1, 2, and 3** in the manuscript and for ease of reference, the repository also provides two Excel files:

- *Differential distribution table after two 5-bit random-XOR fault injections (Table 3)*
- *Differential distribution table after two 5-bit random-AND fault injections*

The project is based entirely on resources shared by the Ascon development team at [Ascon Official Resources](https://ascon.isec.tugraz.at/resources.html). The original Ascon implementation has been slightly modified to extract the intermediate state values needed for fault attacks. **Fault Model A** and **Fault Model B** attack simulations have been implemented, along with two fault distribution models—**ALL-uniform** and **HW-uniform**—which can be dynamically switched within the program.

**Table3_result**: Generates the differential distribution table and the complete set of fault-trail results. In the manuscript, these correspond to Table 3 (two 5-bit random-XOR fault injections) and Table 4 (fault trails under random-XOR). The program has also been extended with a simple annotation to generate the corresponding results for two 5-bit random-AND fault injections.

**Table6_result**: Probability of recovering *m* under random-AND faults.

**Table8_result**: Attack simulations for Fault Model A and Fault Model B, supporting dynamic switching between **ALL-uniform** and **HW-uniform** fault distribution models.

The **Table8_result(fault model A)** and **Table8_result(fault model B)** projects generate random fault values, perform fault injection on the Ascon algorithm, and record the results in Excel files. These results help evaluate the algorithm’s vulnerability under different fault models. The following sections focus on the **configuration** and **execution** of these two projects.

------

## 🛠️ Requirements



To successfully compile and run this project, please ensure the following dependencies and tools are installed:

- **Microsoft Visual Studio 2022** (or newer)

- libxl Library

   

  for Excel file handling (version compatible with Visual Studio 2022)

  - Only necessary for **Table8_result(fault model A)** and **Table8_result(fault model B)**
  - You can obtain the library from [LibXL Official](https://www.libxl.com/)
  - **Note**: The program has been pre-activated with a valid license key embedded in the code, so no additional activation is required.

- **Windows operating system** (required for `windows.h` usage)

Additionally, the project requires **C++11** or later for compiling.

------

## 📝 Setup Instructions



### 1️⃣ Install Microsoft Visual Studio 2022



Ensure that **Microsoft Visual Studio 2022** (or newer) is installed on your system. Follow the steps below for installation and configuration:

1. **Download Visual Studio 2022** from the [official Microsoft website](https://visualstudio.microsoft.com/).
2. During installation, select the **Desktop development with C++** workload to ensure you have the necessary C++ tools installed.

### 2️⃣ Download and Setup libxl Library



To efficiently read and write Excel files, the project utilizes the **libxl library**. The configuration process is outlined below:

The repository contains the **`include_cpp`** folder, **`lib64`** folder, and **`libxl.dll`** file (located in the `x64/Release` folder) necessary for loading the **libxl** library.

This setup is intended for compiling and running on **Windows systems**.

**Visual Studio environment configurations for libxl:**

- **C/C++ → General → Additional Include Directories**: Set this to the path of the **`include_cpp`** folder.
- **Linker → General → Additional Library Directories**: Set this to the path of the **`lib64`** folder.
- **Linker → Input → Additional Dependencies**: Add `libxl.lib`.

Finally, **copy the `libxl.dll` file** to the path where the `.exe` file is located (this project uses `x64/Release`).

------

## 🚀 Running the Program



### 1️⃣ Compile the Project



Once the dependencies are set up, open the project in **Visual Studio 2022**:

1. Open the `.sln` file in Visual Studio.
2. Ensure that you're targeting **x64** architecture.
3. Build the project by selecting `Build` > `Build Solution`.

### 2️⃣ Executable Version Available



The program has been successfully compiled into an executable (`.exe`). You can find the compiled executable in the `Table8_result(fault model A)\x64\Release` directory. Double-click the executable to run it.

Upon running the program, the **Excel file** containing the fault injection results will be generated in the same directory as the `.exe` file.

> **Note**: Wait for the message `Excel file generated successfully!` to appear in the command window before attempting to open the Excel file.

------

## 📊 Results



- After the program finishes running, you will see the following messages in the **DOS window**:

  - **"Average fault injection rounds"**: Indicates how many rounds of fault injection are required to recover all 64 S-box input values.
  - **"Average fault nibble"**: Indicates the average number of fault injections needed for each S-box to recover its input value.

  ### **Excel Output**

  

  **Fault Model A** ⚡

  After the program finishes and the message `Excel file generated successfully!` appears, you can open the generated **Excel file** located in the same directory as the executable. The **Excel file** will be named `Ascon_random-mix_trials.xlsx` and will contain the following columns:

  1. 5-bit S-box input values at the final round of P12 in the Finalization phase of Ascon

     :

     - These are the 5-bit input values of the S-box at the final round of the P12 permutation in the Ascon algorithm’s Finalization phase.

  2. Total fault injection rounds to recover each S-box input value

     :

     - This column records how many rounds of fault injections (across all 64 S-boxes) are needed to recover the original input value of each S-box.

  3. Total random-xor fault injections for each S-box input value

     :

     - This column records the total number of random **XOR** faults injected for each S-box input value to recover the original value.

  4. Total random-and fault injections for each S-box input value

     :

     - This column records the total number of random **AND** faults injected for each S-box input value to recover the original value.

  **Fault Model B** ⚡

  Similarly, the **Excel file** will be named `Ascon_random-and_trials.xlsx` and will contain the following columns:

  1. **5-bit S-box input values at the final round of P12 in the Finalization phase of Ascon**.
  2. **Total random-and fault injections for each S-box input value**.

------

## ⚙️ Fault Distributions



This model simulates fault injection into the Ascon algorithm’s finalization phase, specifically during the last round of the P12 permutation. The program provides two types of fault distributions:

- **ALL-uniform**: Faults are injected uniformly across all bits.
- **HW-uniform**: Faults are injected with a uniform distribution based on Hamming weight.

You can switch between these models in the code, depending on the attack scenario you want to simulate.

------

## 🔧 Customization



You can modify the fault models and experiment parameters by changing the following variables:

- **Fault Model Type**: Switch between `ALL-uniform` and `HW-uniform`.
- **Number of Trials**: Adjust `trial_Num` to specify how many trials to run in each experiment.
- **Random Fault Injection**: The program randomly generates fault values for each trial.

------

## 💡 Notes



- **libxl License**: This project uses the **libxl** library for Excel file handling, and the license has been pre-activated in the program. No additional activation is required.
- **Fault Injection Parameters**: Fault injection parameters can be adjusted based on your desired experiment configuration. Refer to the code comments for more details.

------

## 🧑‍💻 Troubleshooting



If you encounter any issues, please check the following:

1. **Library Configuration**: Ensure that the **libxl** library paths are correctly set in Visual Studio.
2. **Missing DLL**: Make sure the `libxl.dll` file is located in the same directory as your `.exe` file.
3. **Error in Excel File Generation**: If Excel is not installed, the program will fail to generate results.

------

## 📝 License



This project is licensed under the MIT License - see the [LICENSE](https://github.com/VGD-AcE/HDFA_for_TOSC/blob/main/LICENSE) file for details.

------

## 💬 Contact



If you have any questions or feedback, feel free to reach out to me via [email](mailto:your-email@example.com) or open an issue on GitHub.
