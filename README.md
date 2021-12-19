# Llama Compiler Project - NTUA 
In this course (Compilers - NTUA) we were asked to create a compiler for a given language, llama, which is a functional programming language. The doc of this language can be found in greek [here](https://github.com/ChristosHadjichristofi/compilers-NTUA/blob/main/doc_llama2021.pdf).

<p align="center">
	<img alt="Byte Code Size" src="https://img.shields.io/github/languages/code-size/ChristosHadjichristofi/compilers-NTUA?color=red" />
	<img alt="# Lines of Code" src="https://img.shields.io/tokei/lines/github/ChristosHadjichristofi/compilers-NTUA?color=red" />
	<img alt="# Languages Used" src="https://img.shields.io/github/languages/count/ChristosHadjichristofi/compilers-NTUA?color=yellow" />
	<img alt="Top language" src="https://img.shields.io/github/languages/top/ChristosHadjichristofi/compilers-NTUA?color=yellow" />
	<img alt="Last commit" src="https://img.shields.io/github/last-commit/ChristosHadjichristofi/compilers-NTUA?color=important" />
</p>

## Authors
* [Christos Hadjichristofi](https://github.com/ChristosHadjichristofi)
* [Panos Zevgolatakos](https://github.com/panoszvg)

## Llama Compiler Supported Features
* Basic Data Types: integer, float, character, boolean
* Reference Types and Arrays of one or more dimensions (cannot create Ref Array, or Array of Array)
* User Defined Types (can be recursive too)
* Library Functions
* Type Inference
* Function Closures
* High Order Functions
* Structural Equality
* Optimizations

## Library
To build the llama library, lib.a was used which was created using this [Library](https://github.com/abenetopoulos/edsger_lib). Most of the functions that the compiler supports comes from the lib.a file. Some other functions that were not implemented were created by us with the help of llvm.

### Functions Llama Compiler supports

#### **Input and Output**

* print_int    : int           -> unit
* print_bool   : bool          -> unit
* print_char   : char          -> unit
* print_float  : float         -> unit
* print_string : array of char -> unit
* read_int     : unit          -> int
* read_bool    : unit          -> bool
* read_char    : unit          -> char
* read_float   : unit          -> float
* read_string  : unit          -> array of char

##### **Math Functions**

* abs          : int           -> int
* fabs         : float         -> float
* sqrt         : float         -> float
* sin          : float         -> float
* cos          : float         -> float
* tan          : float         -> float
* atan         : float         -> float
* exp          : float         -> float
* ln           : float         -> float
* pi           : pi            -> float

#### **Increase/Decrease Functions**

* incr         : int ref       -> unit
* decr         : int ref       -> unit

#### **Convert Functions**

* float_of_int : int           -> float
* int_of_float : float         -> int
* round        : float         -> int
* int_of_char  : char          -> int
* char_of_int  : int           -> char

### How to build
* Clone the repository
* Make sure you have [nasm](https://www.nasm.us/)
* ```./libs.sh```
* We changed some synonyms using the ```change_syms``` file

## Dependencies
* [Flex](https://en.wikipedia.org/wiki/Flex_(lexical_analyser_generator))
* [Bison](https://www.gnu.org/software/bison/)
* [LLVM](https://llvm.org/)
* [Clang](https://clang.llvm.org/) - Used to create assembly code from ir and executable from assembly code)
* [C++11](https://www.cplusplus.com/)
* [Library](https://github.com/abenetopoulos/edsger_lib) - Requires nasm if you want to build the library yourself

### Build
The llama compiler executable used the following versions of the above dependencies:
* Flex  - Version 2.6.4
* Bison - Version 3.5.1
* LLVM  - Version 10.0.0
* Clang - Version 10.0.0-4ubuntu

## Installation
* Clone the repository and move to src folder
* ```make``` to create the llama compiler executable

## Usage
The compiler can be used with any of the flags (in any combination) described below:
```c++
./llama [path/to/llamafile] -h    (* Prints all possible flag options *)
./llama [path/to/llamafile] -O    (* Optimization Flag *)
./llama [path/to/llamafile] -ast  (* Prints Abstract Syntax Tree *)
./llama [path/to/llamafile] -fv   (* Prints Free Variables each Function has *)
./llama [path/to/llamafile] -i    (* Prints Intermediate Code - LLVM IR *)
./llama [path/to/llamafile] -f    (* Prints Assembly Code *)
./llama [path/to/llamafile] -l    (* Enables __LINE__ Macro - Debugging Purposes *)
./llama [path/to/llamafile] -st   (* Prints complete SymbolTable (may have duplicates, because of scopes). *)
```
