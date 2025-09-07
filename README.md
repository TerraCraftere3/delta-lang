# delta-lang [![CMake](https://github.com/TerraCraftere3/delta-lang/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/TerraCraftere3/delta-lang/actions/workflows/cmake_windows.yml)
The Delta Programming Language

## How to Build
- Clone the repository using `git clone --recursive https://github.com/TerraCraftere3/delta-lang` into any folder you want
- Open a terminal in the repository and run the following commands:
    - `mkdir build`
    - `cd build`
    - `cmake .. -G "Visual Studio 17 2022"` (Adjust the Generator to your Visual Studio Version)
    - `cmake --build . --config Release` or open the generated Solution File

## Usage
### Variables
```
int32 a = 42;
int32 b = 3;
int32 c = a + b;  // Setting Variable
c = 2 * c;        // Updating Variable
```

### Maths 
```
int32 d = a * (b + 2);
int32 e = d - 128;
```

### Exit
```
int32 code = 0;
exit(code); // or just exit(0)
```

### Scopes
```
int32 a = 3;
{
    int32 b = 9;
}
int32 b = 3; // Can redefine because scope is closed
```
You cannot shadow variables that are outside scopes

### IF Statements
```
if(true){
    ...
}else{
    exit(1);
}
```
You cannot shadow variables that are outside scopes

### Comments
```
// This is an example comment
int32 a = 3

// This is a multiline comment
/*if(error) {
    exit(error)
}*/
```

### Constants
```
// Constant values cant be changed after declaration
const int64 ZERO = 0;
```

### Types
```
int64 a = 1;
long b = 2; // same as int64
int32 c = 3;
int d = 4; // same as int32
int16 e = 5;
short f = 6; // same as int16
int8 g = 7;

// Any integer type is compatible with each other (for example assigning the value of an int64 to an int8)
d = a;
```
Any integer type is compatible with another integer type, so are other types (like floats in the future).

### Functions
```
int add(int a, int b) {
    return a + b;
}

int result = add(3, 5);
```

### Main Function

```
int32 main(){
    return 0;
}
```
The return value of main() is used as the exit code of the program

## Grammar
$$
\begin{align}
[\text{Prog}] &\to [\text{FuncDecl}]^* \space [\text{Statement}]^*
\\
[\text{FuncDecl}] &\to [\text{Type}] \space \text{Identifier}([\text{ParamList}]?)[\text{Scope}]
\\
[\text{ParamList}] &\to \text{Param}^*
\\
[\text{Param}] &\to [\text{Type}] \space \text{Identifier}
\\
[\text{FuncCall}] &\to \text{Identifier}([\text{ArgList}]?)
\\
[\text{ArgList}] &\to \text{Expr}^*
\\
[\text{Statement}] &\to 
\begin{cases}
    exit([\text{Expr}]); 
    \\
    [\text{const}|\epsilon]\space[\text{Type}] \space\text{Identifier} = [\text{Expr}];
    \\
    \text{Identifier} = [\text{Expr}];
    \\
    if([\text{Expr}])[\text{Scope}][\text{IfPred}]
    \\
    [\text{Scope}]
\end{cases}
\\
[\text{Scope}] &\to
\begin{cases}
    \{[\text{Statement}]^*\}
\end{cases}
\\
[\text{IfPred}] &\to
\begin{cases}
    \text{elif}(\text{[Expr]})\text{[Scope]}[\text{IfPred}]
    \\
    \text{else}\text{[Scope]}
    \\
    \epsilon
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
    [\text{Expr}]
    \\
    [\text{FuncCall}]
\end{cases}
\\
[\text{Expr}] &\to 
\begin{cases}
    \text{Term}
    \\
    \text{BinExpr}
\end{cases}
\\
[\text{Type}] &\to 
\begin{cases}
    \text{int8}
    \\
    \text{int16}
    \\
    \text{int32}
    \\
    \text{int64}
\end{cases}
\end{align}
$$