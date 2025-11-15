# Aleng Scripting Language

Aleng is a dynamic, interpreted scripting language designed for simplicity and ease of use. It features a clean syntax, robust error handling, and a modular architecture that makes it easy to extend and embed.

## About The Project

This repository contains the complete C++ source code for the Aleng interpreter, including the lexer, parser, visitor-based evaluator, and a module management system. Aleng is built with modern C++20 and CMake, ensuring a clean and portable codebase.

**Core Features:**

*   **Simple & Expressive Syntax:** Aims for readability and ease of learning.
*   **Dynamic Typing:** Variables can hold values of any supported type (Number, String, Boolean, List, Map, Function).
*   **Rich Data Structures:** Built-in support for lists (arrays) and maps (dictionaries).
*   **First-Class Functions:** Functions are treated as values; they can be stored in variables, passed as arguments, and returned from other functions, enabling functional programming patterns like closures.
*   **Modular by Design:** Easily import custom Aleng scripts or native C++ functions as modules.
*   **Built-in Testing Library:** Includes a standard library for writing and running unit tests.
*   **Clear Error Reporting:** Provides formatted, context-aware error messages with file paths, line numbers, and column numbers to simplify debugging.

## Getting Started

To get the Aleng interpreter up and running on your local machine, follow the build and installation steps below.

### Prerequisites

*   A C++20 compatible compiler (e.g., GCC 10+, Clang 12+, MSVC v142+)
*   CMake (version 3.16 or higher)
*   Python 3 (for build-time script generation)
*   Git

### Building from Source

1.  **Clone the repository:**
    ```sh
    git clone https://github.com/your_username/aleng.git
    cd aleng
    ```

2.  **Configure the project with CMake:**
    ```sh
    cmake -B build
    ```

3.  **Build the project:**
    ```sh
    cmake --build build
    ```
    This will create an executable named `Aleng` (or `Aleng.exe` on Windows) inside the `build` directory.

### Running Aleng

Once built, you can run Aleng scripts in two ways:

1.  **Running a file:**
    The interpreter will automatically look for and execute a `main.aleng` file in the current directory or a specified path.
    ```sh
    ./build/Aleng /path/to/your/project
    ```

2.  **Interactive Mode (REPL):**
    Launch the Read-Eval-Print Loop (REPL) to experiment with Aleng code interactively.
    ```sh
    ./build/Aleng --repl
    ```
    To exit the REPL, type `.exit` and press Enter.

## Language Tour

Here is a brief overview of the Aleng language syntax and features.

### Variables and Types

Variables are dynamically typed. Use the assignment operator (`=`) to declare and initialize them.

```aleng
# Numbers (all numbers are floating-point internally)
age = 25
pi = 3.14159

# Strings
greeting = "Hello, Aleng!"

# Booleans
is_active = True
is_admin = False

# Lists (dynamic arrays)
items = [1, "apple", True]

# Maps (key-value pairs)
person = {
    "name": "Alex",
    "age": 30
}
```

### Control Flow

#### If/Else Statements
```aleng
If age >= 18
    Print("You are an adult.")
Else
    Print("You are a minor.")
End
```

#### Loops
Aleng supports `For` and `While` loops.

```aleng
# Numeric For loop (inclusive range)
sum = 0
For i = 1 .. 5
    sum = sum + i
End
Print("Sum from 1 to 5 is: " + sum) # Output: 15

# For loop with a custom step
For i = 10 .. 0 step -2
    Print(i) # Output: 10, 8, 6, 4, 2, 0
End

# For..in loop for iterating over collections
fruits = ["apple", "banana", "cherry"]
For fruit in fruits
    Print(fruit)
End

# While loop
counter = 0
While counter < 3
    Print("Iteration: " + counter)
    counter = counter + 1
End
```

### Functions

Functions are first-class citizens. They can be named or anonymous (lambdas).

```aleng
# Defining a named function
Fn greet(name)
    Return "Hello, " + name + "!"
End
Print(greet("World")) # Output: Hello, World!

# Closures: Inner functions capture their surrounding environment
Fn make_adder(x)
    Fn inner(y)
        Return x + y
    End
    Return inner
End

add_five = make_adder(5)
Print(add_five(3)) # Output: 8.0
```

### Data Structures

#### Lists
Access and modify list elements using zero-based indexing.

```aleng
numbers =
Print(numbers) # Output: 20

numbers = 25
Append(numbers, 40) # Built-in function to add elements
Print(numbers) # Output:
Print(numbers.length) # Output: 4
```

#### Maps
Access and modify map elements using dot notation or string indexing.

```aleng
user = { "name": "Sam", "status": "active" }
Print(user.name) # Output: Sam

user.status = "inactive"
user["new_prop"] = 123
Print(user.status) # Output: inactive
```

### Modules

Use the `Import` keyword to load other `.aleng` files or native libraries as modules. The result of an import is a map containing the module's exported variables and functions.

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

### Testing

Aleng includes a built-in test library for writing unit tests.

```aleng
Test = Import "std/test"
CoreSuite = Test.CreateSuite("My Awesome Feature")

Fn test_addition()
    Test.Assert.Equals(2 + 2, 4, "2+2 should equal 4")
End
CoreSuite.Add("should perform basic addition", test_addition)

Fn test_errors()
    Test.Assert.Throws(
        Fn() x = 1 / 0 End,
        "Division by zero should raise an error"
    )
End
CoreSuite.Add("should handle runtime errors", test_errors)

CoreSuite.Run()
```

---

## Roadmap

This project is actively under development. Here is a look at the planned features and improvements for the future:

*   **Phase 1: Core Language and Tooling**
    *   [ ] **Web Playground:** Develop a web-based environment (using WebAssembly) to allow users to write and run Aleng code directly in their browser.
    *   [ ] **Language Server Protocol (LSP):** Implement an LSP for Aleng to provide features like autocomplete, syntax highlighting, and error checking in modern code editors like VS Code.
    *   [ ] **Expanded Standard Library:** Add more built-in modules for file I/O, string manipulation, and system interaction.

*   **Phase 2: Advanced Language Features**
    *   [ ] **Object-Oriented Programming (OOP):** Introduce classes, methods, and inheritance to provide a more structured way of organizing code.
    *   [ ] **Concurrency Model:** Explore adding support for lightweight threads or async/await syntax.

*   **Phase 3: Quality and Performance**
    *   [ ] **Google Test Integration:** Integrate Google Test for more rigorous, low-level testing of the C++ interpreter core.
    *   [ ] **Performance Benchmarking:** Create a suite of benchmarks to identify and optimize performance bottlenecks in the interpreter.
    *   [ ] **JIT Compilation:** Investigate the possibility of adding a Just-In-Time (JIT) compiler to improve execution speed for performance-critical code.