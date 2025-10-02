# delta-lang
[![CMake](https://github.com/TerraCraftere3/delta-lang/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/TerraCraftere3/delta-lang/actions/workflows/cmake_windows.yml) 
![Top language](https://img.shields.io/github/languages/top/TerraCraftere3/delta-lang?color=yellow&logo=cplusplus)
![Language count](https://img.shields.io/github/languages/count/TerraCraftere3/delta-lang?color=blue)
![Repo size](https://img.shields.io/github/repo-size/TerraCraftere3/delta-lang?color=red&logo=gitlab)
![GitHub License](https://img.shields.io/github/license/TerraCraftere3/delta-lang)

Delta is a C Style Programming Language that is compiled to LLVM Intermediate Representation

## How to Build
- Clone the repository using `git clone --recursive https://github.com/TerraCraftere3/delta-lang` into any folder you want
### Terminal
- Open the cloned repo in a terminal and enter the following commands:
    - `mkdir build`
    - `cd build`
    - `cmake .. -G "Visual Studio 17 2022"` (Adjust the Generator to your Visual Studio Version)
    - `cmake --build . --config Release` or open the generated Solution File
### VSCode
- Open the cloned repo in vscode
- Press CTRL + Shift + P
- Enter "CMake: Debug" and press enter
- The Project will build and run the example project

## Structure
| Folder        | Usage                                                   |
| ------------- | ------------------------------------------------------- |
| ``.github``   | Contains workflows and github properties                |
| ``.vscode``   | Contains the settings for vscode cmake                  |
| ``build``     | The output of cmake                                     |
| ``example``   | The Example Project (usually showcases the new feature) |
| ``extension`` | A VSCode Extension for syntax highlighting              |
| ``runtime``   | All the files required by delta.exe                     |
| ``src``       | The Source Code of the Compiler                         |
| ``stdlib``    | The Standard Library shipped with the compiler          |
| ``vendor``    | External Libraries like spdlog                          |

## Usage
### Variables
```
let a: int = 42; // Setting Variable
let b: short = 3;
let c: long = a + b;  
c = 2 * c; // Updating Variable
```

### Maths 
```
let d: int = a * (b + 2);
let e: int = d - 128;
```

### Scopes
```
let a: int = 3;
{
    let b: int = 9;
}
let b: int = 3; // Can redefine because scope is closed
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
let a: int = 3

// This is a multiline comment
/*if(error) {
    exit(error)
}*/
```

### Constants
```
// Constant values cant be changed after declaration
let const ZERO: int = 0;
```

### Types
```
let a: int = 10;
let b: float = 1.2345f;
let pi: double = 3.14;
```
Any integer type is compatible with another integer type, so are other types (like floats in the future).

### Functions
```
fn add(a: int, b: int) -> int {
    return a + b;
}

let result: int = add(3, 5);
```

### Casting
```
let a: int = 10;
let b: float = (float) a;
```

### Pointer
```
fn modifyInt(int* ptr, int newValue) -> void{
    *ptr = newValue;
}

let x: int = 10;
modifyInt(&x, 42); // sets the value of x to 42
```

### Chars
```
let c: char = 'H';
```

### Strings
```
let const str: char* = "Hello World\n";
printf(str);
```

### Arrays
```
let array: int* = malloc(8 * 4); // Allocates an array of 8 * int32
array[0] = 4;
array[1] = 16;
...
array[7] = 3;
```

### Main Function

```
fn main() -> int{
    return 0;
}
```
The return value of main() is used as the exit code of the program

### Includes
```
#include <stdio> // includes io functions like printf
#include <stdgraphics>

fn main() -> int{
    printf("Hello World");
}
```

### Definitions
```
#define PI 3.14159265359
#define someFunction windowsBackend_someFunctionCall()
```
The name of a definition cant be the same as an existing token

## Grammar
(LaTeX Expression might not render correctly in Github)

$$
\begin{align}
[\text{Prog}] &\to [\text{FuncDecl}]^* \space [\text{Statement}]^* \space\textit{List of Functions and Statements}
\\
[\text{FuncDecl}] &\to 
\begin{cases}
fn \space \text{Identifier}([\text{ParamList}]?) \space \text{->} \space [\text{Type}][\text{Scope}]
\\
fn \space \text{Identifier}([\text{ParamList}]?) \space [\text{Scope}] & \textit{Default to void type}
\end{cases}
\\
[\text{ParamList}] &\to \text{Param}^*
\\
[\text{Param}] &\to \text{Identifier}: \space [\text{Type}]
\\
[\text{FuncCall}] &\to \text{Identifier}([\text{ArgList}]?)
\\
[\text{ArgList}] &\to \text{Expr}^*
\\
[\text{Statement}] &\to 
\begin{cases}
    exit([\text{Expr}]); 
    \\
    let \space[\text{const}|\epsilon] \space\text{Identifier}: \space[\text{Type}] = [\text{Expr}]; & \textit{Let Variable}
    \\
    \text{Identifier} = [\text{Expr}]; & \textit{Assign to Variable}
    \\
    \text{Identifier++}; & \textit{Increment Variable}
    \\
    *[\text{Expr}] = [\text{Expr}]; & \textit{Assign to Pointer}
    \\
    [\text{Expr}][[\text{Expr}]] = [\text{Expr}]; & \textit{Assign to Array}
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
- Nonterminals: $` [\text{Category / Element}] `$ - Can continue to have subnodes
- Terminals: $` \text{Element} `$ - Can NOT have subnodes
- Alternations: $` \text{float64} \space | \space \text{double} `$ - Means that two values are the same
- Comments: $` \textit{Example Comment} `$ - Clarifies something about the element that came before
- Cases: $` \begin{cases}
    \text{Case 1}
    \\
    \text{Case 2}
\end{cases} `$ - Means that one thing can have multiple types of subnodes, etc.
