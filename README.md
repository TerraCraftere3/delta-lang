# delta-lang
The Delta Programming Language

## How to Build
- Clone the repository using `git clone --recursive https://github.com/TerraCraftere3/delta-lang` into any folder you want
- Open a terminal in the repository and run the following commands:
    - `mkdir build`
    - `cd build`
    - `cmake .. -G "Visual Studio 17 2022"` (Adjust the Generator to your Visual Studio Version)
    - `cmake --build . --config Release` or open the generated Solution File

## Grammar
$$
\begin{align}
[\text{exit}] &\to exit([\text{expr}])
\\
[\text{expr}] &\to \text{int\_literal}
\end{align}
$$