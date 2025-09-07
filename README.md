# delta-lang [![CMake](https://github.com/TerraCraftere3/delta-lang/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/TerraCraftere3/delta-lang/actions/workflows/cmake_windows.yml)
The Delta Programming Language

## How to Build
- Clone the repository using `git clone --recursive https://github.com/TerraCraftere3/delta-lang` into any folder you want
- Open a terminal in the repository and run the following commands:
    - `mkdir build`
    - `cd build`
    - `cmake .. -G "Visual Studio 17 2022"` (Adjust the Generator to your Visual Studio Version)
    - `cmake --build . --config Release` or open the generated Solution File

## Examples
### Variables
```
let a = 42;
let b = 3;
let c = a + b;
```

### Maths 
```
let d = a * b;
let e = d - 128;
```

### Exit
```
let code = 0;
exit(code);
```
or just `exit(0)`

### Scopes
```
let a = 3;
{
    let b = 9;
}
let b = 3;
```
You cannot shadow variables that are outside scopes

### IF Statements
```
let a = 3;
if(a){
    let b = a + 3;
}
```
You cannot shadow variables that are outside scopes

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
    \\
    if([\text{Expr}])[\text{Scope}]
    \\
    [\text{Scope}]
\end{cases}
\\
[\text{Scope}] &\to
\begin{cases}
    \{[\text{Statement}]^*\}
\end{cases}
\\
[\text{BinExpr}] &\to
\begin{cases}
    [\text{Expr}] / [\text{Expr}] & \text{prec}=1
    \\
    [\text{Expr}] * [\text{Expr}] & \text{prec}=1
    \\
    [\text{Expr}] - [\text{Expr}] & \text{prec}=0
    \\
    [\text{Expr}] + [\text{Expr}] & \text{prec}=0
\end{cases}
\\
[\text{Term}] &\to 
\begin{cases}
    \text{Int Literal}
    \\
    \text{Identifier}
    \\
    (\text{Expr})
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