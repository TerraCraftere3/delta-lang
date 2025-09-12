# delta-lang
[![CMake](https://github.com/TerraCraftere3/delta-lang/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/TerraCraftere3/delta-lang/actions/workflows/cmake_windows.yml) 
![Top language](https://img.shields.io/github/languages/top/TerraCraftere3/delta-lang?color=yellow&logo=cplusplus)
![Language count](https://img.shields.io/github/languages/count/TerraCraftere3/delta-lang?color=blue)
![Repo size](https://img.shields.io/github/repo-size/TerraCraftere3/delta-lang?color=red&logo=gitlab)
![GitHub License](https://img.shields.io/github/license/TerraCraftere3/delta-lang)

Delta is a C Style Programming Language that is compiled to LLVM Intermediate Representation

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
int a = 42; // Setting Variable
short b = 3;
long c = a + b;  
c = 2 * c; // Updating Variable
```

### Maths 
```
int d = a * (b + 2);
int e = d - 128;
```

### Exit
```
int code = 0;
exit(code); // or just exit(0)
```

### Scopes
```
int a = 3;
{
    int b = 9;
}
int b = 3; // Can redefine because scope is closed
```
You cannot shadow variables that are outside scopes

### IF Statements
```
if(statement_a){
    ...
}elif(statement_b){
    ...
}else{
    exit(1);
}
```
You cannot shadow variables that are outside scopes

### Comments
```
// This is an example comment
int a = 3

// This is a multiline comment
/*if(error) {
    exit(error)
}*/
```

### Constants
```
// Constant values cant be changed after declaration
const long ZERO = 0;
```

### Types
```
int a = 10;
float b = 1.2345f;
double pi = 3.14;
```
Any integer type is compatible with another integer type, so are other types (like floats in the future).

### Functions
```
int add(int a, int b) {
    return a + b;
}

int result = add(3, 5);
```

### Casting
```
int a = 10;
float b = (float) a;
```

### Pointer
```
void modifyInt(int* ptr, int newValue) {
    *ptr = newValue;
}

int x = 10;
modifyInt(&x, 42); // sets the value of x to 42
```

### Strings
```
const char* str = "Hello World\n";
printf(str);
```

### Arrays
```
int* array = malloc(8 * 4); // Allocates an array of 8 * int32
array[0] = 4;
array[1] = 16;
...
array[7] = 3;
```

### Main Function

```
int32 main(){
    return 0;
}
```
The return value of main() is used as the exit code of the program

### External Functions
```
// MSVCRT (C runtime)
void exit(int)
int printf(char*);
void* malloc(long);
void free(void*);
long strlen(long);
long strcpy(long, long);
```

## Grammar
(LaTeX Expression might not render correctly in Github)

$$
\begin{align}
[\text{Prog}] &\to [\text{FuncDecl}]^* \space [\text{Statement}]^* \space\textit{List of Functions and Statements}
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
    [\text{const}|\epsilon]\space[\text{Type}] \space\text{Identifier} = [\text{Expr}]; & \textit{Let Variable}
    \\
    \text{Identifier} = [\text{Expr}]; & \textit{Assign to Variable}
    \\
    *[\text{Expr}] = [\text{Expr}] & \textit{Assign to Pointer}
    \\
    [\text{Expr}][[\text{Expr}]] = [\text{Expr}] & \textit{Assign to Array}
    \\
    if([\text{Expr}])[\text{Scope}][\text{IfPred}]
    \\
    [\text{Scope}]
    \\
    [\text{Expr}];
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
    [\text{Expr}] / [\text{Expr}] & \text{prec}=2
    \\
    [\text{Expr}] * [\text{Expr}] & \text{prec}=2
    \\
    [\text{Expr}] - [\text{Expr}] & \text{prec}=1
    \\
    [\text{Expr}] + [\text{Expr}] & \text{prec}=1
    \\
    [\text{Expr}] <= [\text{Expr}] & \text{prec}=0
    \\
    [\text{Expr}] < [\text{Expr}] & \text{prec}=0
    \\
    [\text{Expr}] == [\text{Expr}] & \text{prec}=0
    \\
    [\text{Expr}] >= [\text{Expr}] & \text{prec}=0
    \\
    [\text{Expr}] > [\text{Expr}] & \text{prec}=0
\end{cases}
\\
[\text{Term}] &\to 
\begin{cases}
    \text{Int} \space|\space  \text{Char Literal} & \textit{Char is just ASCII Value as Int}
    \\
    \text{Float Literal}
    \\
    \text{Double Literal}
    \\
    \text{String Literal}  & \textit{Equals to const char*}
    \\
    \text{Identifier} & \textit{Variable}
    \\
    [\text{Expr}]
    \\
    [\text{FuncCall}]
    \\
    ([\text{Type}])\space[\text{Expr}] & \textit{Cast}
    \\
    \&\text{Identifier} & \textit{Address of pointer}
    \\
    *\text{Identifier} & \textit{Dereference pointer}
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
    \text{int8} & | & \text{char} & \textit{1 Byte Integer}
    \\
    \text{int16} & | & \text{short} & \textit{2 Byte Integer}
    \\
    \text{int32} & | & \text{int} & \textit{4 Byte Integer}
    \\
    \text{int64} & | & \text{long} & \textit{8 Byte Integer}
    \\
    \text{float32} & | & \text{float} & \textit{4 Byte Float}
    \\
    \text{float64} & | & \text{double} & \textit{8 Byte Float}
    \\
    \text{int8*} & | & \text{char*} & \textit{Can be used as String}
    \\
    \text{int16*} & | & \text{short*}
    \\
    \text{int32*} & | & \text{int*}
    \\
    \text{int64*} & | & \text{long*}
    \\
    \text{float32*} & | & \text{float*}
    \\
    \text{float64*} & | & \text{double*}
\end{cases}
\end{align}
$$

### Grammar Rules
- Nonterminals: $ [\text{Category / Element}] $ - Can continue to have subnodes
- Terminals: $ \text{Element} $ - Can NOT have subnodes
- Alternations: $ \text{float64} \space | \space \text{double} $ - Means that two values are the same
- Comments: $ \textit{Example Comment} $ - Clarifies something about the element that came before
- Cases: $ \begin{cases}
    \text{Case 1} &
    \\
    \text{Case 2} &
\end{cases} $ - Means that one thing can have multiple types of subnodes, etc.
