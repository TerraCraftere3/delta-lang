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
[\text{Prog}] &\to [\text{Statement}]^*
\\
[\text{Statement}] &\to 
\begin{cases}
    exit([\text{Expr}]); 
    \\
    let \space\text{Identifier} = [\text{Expr}];
\end{cases}
\\
[\text{BinExpr}] &\to
\begin{cases}
    [\text{Expr}] * [\text{Expr}] & \text{prec}=1
    \\
    [\text{Expr}] + [\text{Expr}] & \text{prec}=0
\end{cases}
\\
[\text{Term}] &\to 
\begin{cases}
    \text{Int Literal}
    \\
    \text{Identifier}
\end{cases}
\\
[\text{Expr}] &\to 
\begin{cases}
    \text{Term}
    \\
    \text{BinExpr}
\end{cases}
\end{align}
$$