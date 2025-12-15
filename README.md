# Aleng Scripting Language

Aleng is a dynamic, interpreted scripting language designed for simplicity and ease of use. It stands out for its clean syntax, robust error handling, and a modular architecture that facilitates extension and embedding in other projects.

## About The Project

This repository contains the complete C++ source code for the Aleng interpreter, which includes the lexer, parser, visitor-based evaluator, and a module management system. Aleng is built with modern C++20 and CMake, ensuring a clean and portable codebase.

### Key Features

*   **Simple & Expressive Syntax:** Focused on readability and ease of learning.
*   **Dynamic Typing:** Variables can store values of any supported type: **Number**, **String**, **Boolean**, **List**, **Map**, and **Function**.
*   **First-Class Functions:** Functions are treated as values, allowing for functional programming patterns like closures and variadic functions (which accept a variable number of arguments).
*   **Rich Data Structures:** Native support for **Lists** (dynamic arrays) and **Maps** (dictionaries/objects), with support for nested property access.
*   **Modularity:** Easy import of custom Aleng scripts or native C++ functions as modules.
*   **Integrated Testing Library:** Includes a standard library for writing and running unit tests.
*   **Clear Error Reporting:** Provides formatted, context-aware error messages, including file paths, line numbers, and column numbers to simplify debugging.

## Getting Started

To get the Aleng interpreter up and running on your local machine, follow the build and installation steps below.

### Prerequisites

*   A C++20 compatible compiler (e.g., GCC 10+, Clang 12+, MSVC v142+)
*   CMake (version 3.16 or higher)
*   Python 3 (for build-time script generation)
*   Git

### Building from Source

1.  **Clone the repository:**
    
    ```shell
    git clone https://github.com/Aletropy/aleng.git
    cd aleng
    ```
    
2.  **Configure the project with CMake:**
    
    ```shell
    cmake -B build
    ```
    
3.  **Build the project:**
    
    ```shell
    cmake --build build
    ```
    
    This will create an executable named `AlengCLI` (or `AlengCLI.exe` on Windows) inside the `build` directory.

### Running Aleng

Once built, you can run Aleng scripts in two ways:

1.  **Running a file:** The interpreter will automatically look for and execute a `main.aleng` file in the current directory or a specified path.
    
    ```shell
    ./build/AlengCLI /path/to/your/project
    ```
    
2.  **Interactive Mode (REPL):** Launch the Read-Eval-Print Loop (REPL) to experiment with Aleng code interactively.
    
    ```shell
    ./build/AlengCLI --repl
    ```
    
    To exit the REPL, type `.exit` and press Enter.

## Language Tour

Here is an overview of the Aleng language syntax and features, based on the code example and project documentation.

### Variables and Types

Variables are dynamically typed. The assignment operator (`=`) is used to declare and initialize them.

| Type | Example | Description |
| :--- | :--- | :--- |
| **Number** | `pi = 3.14159` | All numbers are internally floating-point. |
| **String** | `name = "Aleng"` | Character strings. |
| **Boolean** | `is_active = True` | Logical values `True` or `False`. |
| **List** | `items = [1, "a", True]` | Dynamic arrays, accessed by index (0-based). |
| **Map** | `person = { "name": "Alex", "age": 30 }` | Key-value pairs (dictionaries/objects). |

### Data Structures

#### Lists

Access and modification of elements are done by zero-based indexing. The built-in function `Append` can be used to add elements.

```aleng
numbers = [10, 20, 30]
Print(numbers[1]) # Output: 20

Append(numbers, 40) # Adds 40
Print(numbers.length) # Output: 4
```

#### Maps (Maps/Objects)

Access and modification of elements can be done using dot notation or string indexing.

```aleng
user = { "name": "Sam", "status": "active" }
Print(user.name) # Output: Sam

user.status = "inactive"
user["new_prop"] = 123

# Maps can simulate objects with methods
Fn get_greeting(person_obj)
    Return "Hello, " + person_obj.name
End

person = {
    "name": "Alex",
    "greet": get_greeting
}

message = person.greet(person)
Print(message) # Output: Hello, Alex
```

### Functions

Functions are declared with the `Fn` keyword and can use the `Return` keyword to return a value.

#### Variadic Functions

Functions can accept a variable number of arguments by prefixing the parameter name with the `$` symbol. This special parameter collects all additional arguments into a **List**.

```aleng
Fn sum_all($numbers)
    total = 0
    For num in numbers
        total = total + num
    End
    Return total
End

Print(sum_all(1, 2, 3)) # Output: 6
Print(sum_all()) # Output: 0
```

#### Recursive Functions

Aleng supports recursive function calls, as demonstrated in the factorial calculation example.

```aleng
Fn factorial(n)
    If n <= 1
        Return 1
    End
    Return n * factorial(n - 1)
End

Print(factorial(5)) # Output: 120
```

### Control Flow

#### Conditionals (If/Else)

Conditional structures are defined with `If`, `Else` (optional), and terminated with `End`.

```aleng
If age >= 18
    Print("You are an adult.")
Else
    Print("You are a minor.")
End
```

#### Loops (For/While)

Aleng supports `For` loops for numeric iteration and over collections, and `While` loops.

| Loop Type | Syntax | Example |
| :--- | :--- | :--- |
| **Numeric For** | `For var = start .. end [step step_value]` | `For i = 1 .. 5` |
| **Collection For** | `For item in collection` | `For fruit in fruits` |
| **While** | `While condition` | `While counter < 3` |

### Modules

Use the `Import` keyword to load modules. The result is a **Map** containing the module's exported variables and functions.

```aleng
# In 'my_module.aleng':
Fn PublicFunc()
    Print("Hello from the module!")
End
PI = 3.14

# In 'main.aleng':
MyModule = Import "my_module"
MyModule.PublicFunc()
Print("PI is: " + MyModule.PI)
```

### Unit Tests

The built-in testing library (`std/test`) allows for the creation of test suites and assertions.

```aleng
Test = Import "std/test"
CoreSuite = Test.CreateSuite("My Awesome Feature")

Fn test_addition()
    Test.Assert.Equals(2 + 2, 4, "2+2 should equal 4")
End
CoreSuite.Add("should perform basic addition", test_addition)

CoreSuite.Run()
```

## Roadmap

The project is under active development, with plans to expand features and improve performance:

*   **Phase 1: Core Language and Tooling**
    *   Web Playground (using WebAssembly).
    *   Language Server Protocol (LSP) for code editors.
    *   Standard Library Expansion (File I/O, string manipulation).
*   **Phase 2: Advanced Language Features**
    *   Object-Oriented Programming (Classes, Methods, Inheritance).
    *   Concurrency Model (Lightweight threads or async/await syntax).
*   **Phase 3: Quality and Performance**
    *   Google Test Integration for low-level testing.
    *   Performance Benchmarking and optimization.
    *   Investigation of Just-In-Time (JIT) compilation.
