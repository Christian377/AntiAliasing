# Comparative Analysis of Anti-Aliasing Algorithms

## Project Description
This project empirically evaluates and compares the performance and visual quality of three industry-standard anti-aliasing algorithms: Multi-Sampling Anti-Aliasing (MSAA), Fast Approximate Anti-Aliasing (FXAA), and Subpixel Morphological Anti-Aliasing (SMAA). The program renders two test scenes (a simple Triangle and a complex Dartboard) using a custom C/OpenGL engine, measuring GPU frame times for each algorithm to quantify trade-offs between visual fidelity and computational cost in Forward Rendering pipelines.

## Input files
* **Shader Source Code (`resources/shaders/`)**: Contains the GLSL source code for all rendering passes:
    * `vertex_default.glsl` / `fragment_default.glsl`: Basic geometry shaders.
    * `vertex_fullscreen_quad.glsl`: Post-processing pass setup.
    * `fragment_fxaa.glsl`: Simplified Console FXAA implementation.
    * `fragment_fxaa_iterative.glsl`: High-quality PC FXAA implementation (v3.11).
    * `SMAA.hlsl`: The core SMAA library.
    * `vertex_*_smaa.glsl` / `fragment_*_smaa.glsl`: The three-pass SMAA shader implementation (Edge, Blend, Neighborhood).
* **Fonts (`resources/Inter-4.1/`)**: Font files used by the ImGui interface.

## Output files
The C program generates raw performance logs (comma-separated values of frame times in nanoseconds) in the executable directory (`bin/`).
* `aa_NONE*.txt`: Baseline performance logs.
* `aa_MSAAx*.txt`: Hardware multi-sampling results.
* `aa_FXAA*.txt`: Post-processing FXAA results.
* `aa_SMAA_*.txt`: Multi-pass SMAA results.

The MATLAB script processes the raw logs and generates mean value bar charts comparing the algorithms in the results folder (`results/`).
* `performance_triangle.png`: Frame time comparison for the simple scene.
* `performance_dartboard.png`: Frame time comparison for the complex scene.

These files are intermediate data processed by the MATLAB script to generate the final visualizations.

## Report
The final report is located in the **`docs/`** directory as `report.pdf`. This directory also contains the templates used for the report structure.

## Running the program

### Dependencies
* **Operating System**: Tested on Windows, Linux compatible 
* **Build System**: CMake (Version 3.10+).
* **Compiler**: Any recent C/C++ compiler.
* **Languages**: C, GLSL, MATLAB (for analysis).
* **External Libraries**:
    * **GLFW** (Windowing & Input)
    * **GLAD** (OpenGL Loader)
    * **cimgui / Dear ImGui** (User Interface)

### Option A: Automated Benchmark
The project includes a MATLAB script (`aa.m`) that handles the entire pipeline: it automatically creates the build directory, compiles the C code via CMake, runs the benchmark mode, and plots the results.

Run the script in the terminal:
 ```
 git clone --recursive https://github.com/Christian377/AntiAliasing.git
 cd AntiAliasing
 matlab -batch aa
 ```
 *(This will compile the project and save the resulting graphs.)*

### Option B: Manual UI Mode
If you wish to visually inspect the anti-aliasing quality or performance in real-time, you can build and run the C engine manually. This launches a window with an ImGui overlay.

**1. Build via CMake:**
```bash
git clone --recursive https://github.com/Christian377/AntiAliasing.git
cd AntiAliasing
mkdir .build
cd .build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

**2. Execute:**
Run the executable located in the `bin` folder:
```bash
./bin/aa  # (or .\bin\aa.exe on Windows)
```

**3. Controls:**
* **Anti-Aliasing Algorithm:** Click the buttons (e.g., "MSAA x4", "FXAA", "SMAA_ULTRA") to switch algorithms instantly.
* **Scene Selection:** Toggle between "Triangle" (Simple) and "Dartboard" (Complex) to see how different geometry affects edge detection.
* **Tracing:** You can manually trigger a sample recording session from this UI if desired.

## Contributors
* **Christian Abboud** - Core Engine, Shader Implementation, Automation Logic.

## Acknowledgments

### Data sources
* **Fonts**: Inter Font Family (`resources/Inter-4.1`).

### Code
* **FXAA 3.11 Implementation**: The `fragment_fxaa_iterative.glsl` shader is ported from the original NVIDIA FXAA 3.11 HLSL source code by **Timothy Lottes** (NVIDIA).
* **SMAA Implementation**: The `SMAA.hlsl` library is integrated directly from the original source code by **Jorge Jimenez**, **Jose I. Echevarria**, **Tiago Sousa**, and **Diego Gutierrez**. The accompanying shader logic (`vertex/fragment_*_smaa.glsl`) was implemented to interface with this library.
* **Documentation & Code Structure**: Portions of the code documentation (Doxygen headers) and report structure were refined with assistance from **Google Gemini (LLM)**.
