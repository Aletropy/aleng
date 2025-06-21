# Aleng Programming Language

**Aleng** is a lightweight, dynamically-typed scripting language designed for simplicity and embeddability. It features a straightforward syntax, support for user-defined functions, modules, and a growing set of built-in capabilities.

## Table of Contents

1.  [Getting Started](#getting-started)
    *   [Prerequisites](#prerequisites)
    *   [Building](#building)
    *   [Running Aleng](#running-aleng)
        *   [REPL (Read-Eval-Print Loop)](#repl-read-eval-print-loop)
        *   [Executing Files](#executing-files)
2.  [Language Syntax](#language-syntax)
    *   [Comments](#comments)
    *   [Data Types](#data-types)
        *   [Numbers](#numbers)
        *   [Strings](#strings)
    *   [Variables and Assignment](#variables-and-assignment)
    *   [Operators](#operators)
        *   [Arithmetic Operators](#arithmetic-operators)
        *   [Comparison Operators](#comparison-operators)
        *   [Operator Precedence](#operator-precedence)
    *   [Control Flow](#control-flow)
        *   [If-Else Statements](#if-else-statements)
    *   [Expressions and Statements](#expressions-and-statements)
3.  [Functions](#functions)
    *   [Defining Functions](#defining-functions)
    *   [Parameters](#parameters)
        *   [Untyped Parameters](#untyped-parameters)
        *   [Typed Parameters](#typed-parameters)
        *   [Variadic Parameters](#variadic-parameters)
    *   [Calling Functions](#calling-functions)
    *   [Return Values](#return-values)
    *   [Scope](#scope)
4.  [Built-in Functions](#built-in-functions)
5.  [Modules](#modules)
    *   [Importing Modules](#importing-modules)
    *   [Module Resolution](#module-resolution)
6.  [Examples](#examples)
7.  [Roadmap (Potential Future Features)](#roadmap-potential-future-features)
8.  [Contributing](#contributing)

---

## 1. Getting Started

### Prerequisites

*   A C++17 compliant compiler (e.g., GCC, Clang, MSVC).
*   CMake (for building the project - assuming you'll use it).
*   Git (for cloning the repository).

### Building

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/Aletropy/aleng
    cd aleng
    ```

2.  **Configure and build using CMake (example):**
    ```bash
    mkdir build
    cd build
    cmake ..
    make  # Or your specific build command (e.g., cmake --build .)
    ```
    After a successful build, the `aleng` executable (or similarly named) will be available, likely in the `build` directory or a subdirectory.

### Running Aleng

#### REPL (Read-Eval-Print Loop)

You can interact with Aleng directly using its REPL:

```bash
./aleng --repl
```

This will start an interactive session:

```
Aleng$ Print("Hello, Aleng!")
Hello, Aleng!
Aleng$ x = 10 + 5
Aleng$ Print(x)
15
Aleng$ .exit
Exiting...
```

Type `.exit` to quit the REPL.

#### Executing Files

Aleng scripts are typically saved in files with the `.aleng` extension.

*   **Running a specific file:**
    ```bash
    ./aleng path/to/your/script.aleng
    ```

*   **Running `main.aleng` in a workspace:**
    If you provide a directory, Aleng will look for `main.aleng` inside it:
    ```bash
    ./aleng path/to/your/workspace/
    ```
    If no arguments are given, Aleng will search recursively from the current directory for a `main.aleng` file to execute.

---

## 2. Language Syntax

### Comments

Single-line comments start with `//` and extend to the end of the line.

```aleng
// This is a single-line comment
x = 10 // This is also a comment
```

> [!WARNING]  
> Comments are not currently supported.

### Data Types

Aleng is dynamically typed. Variables can hold values of different types, and their type is determined at runtime. The primary built-in data types are:

#### Numbers

Numbers in Aleng are represented as double-precision floating-point values internally, though integer literals can be used.

```aleng
age = 30
price = 19.99
negative = -5
```

#### Strings

Strings are sequences of characters enclosed in double quotes (`"`).

```aleng
message = "Hello, world!"
name = "Aleng"
empty_string = ""
```

Escape sequences within strings:
*   `\n`: Newline
*   `\t`: Tab
*   `\"`: Double quote
*   `\\`: Backslash

```aleng
multiline = "First line\nSecond line"
quoted_string = "She said, \"Hi!\""
```

### Variables and Assignment

Variables are declared and assigned values using the `=` operator. Variable names must start with a letter or underscore (`_`) and can be followed by letters, numbers, or underscores.

```aleng
myVariable = 123
user_name = "Alice"
_value = "data"
PI = 3.14159

// Reassignment
myVariable = "Now I am a string"
```

### Operators

#### Arithmetic Operators

For `NUMBER` operands:
*   `+`: Addition
*   `-`: Subtraction
*   `*`: Multiplication
*   `/`: Division (always floating-point division)

```aleng
sum = 10 + 5       // 15
difference = 20 - 7  // 13
product = 4 * 6      // 24
quotient = 10 / 4    // 2.5
```

Special behavior with `STRING` and `NUMBER`:
*   `STRING + STRING`: Concatenation
    ```aleng
    greeting = "Hello, " + "world!" // "Hello, world!"
    ```
*   `STRING + NUMBER` (or `NUMBER + STRING`): Concatenation (number is converted to string)
    ```aleng
    label = "Value: " + 100 // "Value: 100"
    ```
*   `STRING * NUMBER`: String repetition
    ```aleng
    stars = "*" * 5 // "*****"
    ```

#### Comparison Operators

These operators return `1.0` (true) or `0.0` (false).
*   `==`: Equal to
*   `!=`: Not equal to

```aleng
Print(5 == 5)     // 1.0 (true)
Print(5 == 6)     // 0.0 (false)
Print("hi" == "hi") // 1.0 (true)
Print("a" != "b") // 1.0 (true)

Print(10 == "10") // 0.0 (false - types must match for equality without coercion)
```
Comparison between different types (e.g., a number and a string) will generally result in `0.0` (false) for `==` and `1.0` (true) for `!=`.

#### Operator Precedence

Aleng follows standard operator precedence:
1.  Parentheses `()`
2.  Multiplication `*`, Division `/` (left-to-right)
3.  Addition `+`, Subtraction `-` (left-to-right)
4.  Comparison `==`, `!=` (left-to-right)
5.  Assignment `=` (right-to-left, and is part of an expression)

```aleng
result = 10 + 5 * 2   // result is 20 (5*2 first, then 10+)
result2 = (10 + 5) * 2 // result2 is 30
```

### Control Flow

#### If-Else Statements

`If` statements allow conditional execution of code. An optional `Else` branch can be provided. The structure must be closed with `End`.

The condition is considered "truthy" if it's a non-zero number or a non-empty string. It's "falsy" if it's `0.0` or an empty string.

```aleng
x = 10
If x == 10
    Print("x is 10")
End

y = 5
If y > 10
    Print("y is greater than 10")
Else
    Print("y is not greater than 10")
End

name = "Aleng"
If name
    Print("Name is not empty: " + name)
Else
    Print("Name is empty")
End
```
Nested `If` statements are allowed.

### Expressions and Statements

Aleng is largely an expression-oriented language. Many constructs, like assignments and function calls, are expressions that evaluate to a value. Statements are units of execution. A program is a sequence of statements.

```aleng
// Expression statement
10 + 5

// Assignment expression (evaluates to the assigned value)
a = (b = 20) + 5 // b becomes 20, a becomes 25

// Function call expression
Print("Result: " + (compute() * 2))
```

Statements don't strictly require semicolons, but they can be used as separators, especially if you put multiple statements on one line (though this is not common practice).

---

## 3. Functions

Functions are first-class citizens in Aleng (though not fully exploited yet, e.g., passing functions as arguments is not directly supported by syntax). They allow you to group code into reusable blocks.

### Defining Functions

Functions are defined using the `Fn` keyword, followed by the function name, a list of parameters in parentheses, the function body, and an `End` keyword.

```aleng
Fn Greet()
    Print("Hello from the Greet function!")
End

Fn Add(a, b)
    sum = a + b
    Print("Sum: " + sum) // Last expression is implicitly returned
    sum // Explicitly make 'sum' the last expression for clarity if needed
End
```

### Parameters

#### Untyped Parameters

By default, parameters are untyped and can accept any value.

```aleng
Fn Describe(item)
    Print("The item is: " + item)
End

Describe(123)       // "The item is: 123"
Describe("a book")  // "The item is: a book"
```

#### Typed Parameters

You can optionally specify a type for a parameter using a colon (`:`) followed by the type name (`NUMBER` or `STRING`). If a type is specified, Aleng will perform a runtime type check when the function is called. If the argument's type doesn't match, a runtime error will occur.

```aleng
Fn WelcomeUser(name: STRING, age: NUMBER)
    Print("Welcome, " + name + "! You are " + age + " years old.")
End

WelcomeUser("Alice", 30) // OK

// WelcomeUser(30, "Alice") // This would cause a runtime error
// WelcomeUser("Bob")       // Runtime error (not enough arguments)
```

#### Variadic Parameters

A function can accept a variable number of arguments using the `$` prefix on the *last* parameter. This parameter will "collect" all remaining arguments passed to the function. An optional type can also be specified for variadic parameters, in which case all collected arguments must match that type.

```aleng
Fn PrintAll($items) // Untyped variadic
    // Currently, direct access to 'items' as a list within Aleng script is not supported.
    Print("Received multiple items (how to access them is TBD in script)")
End

Fn SumNumbers($values: NUMBER)
    // The interpreter checks that all extra arguments are numbers.
    // Actual summing logic would require language support for iterating variadic args or built-ins.
    Print("All additional arguments were numbers.")
End

PrintAll(1, "two", 3.0)
SumNumbers(10, 20, 30)
// SumNumbers(10, "oops", 30) // Runtime error: type mismatch for variadic $values
```
**Note:** How user-defined Aleng code accesses the collected variadic arguments (e.g., as a list or via special built-ins) is an area for future language development. Currently, its primary use in user functions is for type-safe "catch-all" for trailing arguments.

### Calling Functions

Functions are called by using their name followed by parentheses containing the arguments.

```aleng
Greet()

result = Add(10, 25) // 'result' will be 35 because Add implicitly returns 'sum'
Print("Result from Add: " + result)
```

### Return Values

Functions in Aleng implicitly return the value of the *last executed statement (expression)* in their body. There is no explicit `return` keyword.

```aleng
Fn GetValue()
    x = 100
    x * 2 // This value (200) will be returned
End

Fn GetGreeting(name: STRING)
    "Hello, " + name + "!" // This string is returned
End

val = GetValue()       // val is 200
msg = GetGreeting("Bob") // msg is "Hello, Bob!"
Print(val)
Print(msg)
```
If a function body is empty or its last statement doesn't produce a meaningful value (e.g., an `If` statement where the condition is false and there's no `Else`), the return value might be a default (like `0.0`).

### Scope

Aleng uses lexical scoping (also known as static scoping).
*   Variables defined inside a function (including parameters) are local to that function and are not accessible outside of it.
*   Functions can access variables defined in their containing (outer) scopes if not shadowed by a local variable.
*   Currently, all top-level definitions (variables and functions outside any other function) are in a global scope.

```aleng
globalVar = "I am global"

Fn MyFunction()
    localVar = "I am local to MyFunction"
    Print(localVar)
    Print(globalVar) // Can access globalVar
End

Fn Outer()
    outerVar = "I am in Outer"
    Fn Inner()
        innerVar = "I am in Inner"
        Print(innerVar)
        Print(outerVar)  // Can access outerVar from Outer's scope
        Print(globalVar) // Can access globalVar
    End
    Inner()
    // Print(innerVar) // Error: innerVar is not accessible here
End

MyFunction()
Outer()
// Print(localVar) // Error: localVar is not accessible here
```

---

## 4. Built-in Functions

Aleng provides a set of built-in functions:

| Function                                        | Description                                                                                                                               |
| :---------------------------------------------- | :---------------------------------------------------------------------------------------------------------------------------------------- |
| `Print(arg1, arg2, ...)`                        | Prints one or more arguments to the console. Arguments are converted to strings if necessary. Each argument is printed on a new line.         |
| `IsString(arg1, arg2, ...)`                     | Checks if **all** provided arguments are strings. Returns `1.0` (true) if all arguments are strings, otherwise `0.0` (false).             |
| `IsNumber(arg1, arg2, ...)`                     | Checks if **all** provided arguments are numbers. Returns `1.0` (true) if all arguments are numbers, otherwise `0.0` (false).             |
| `ParseNumber(value: ANY) -> NUMBER`             | Attempts to parse the given `value` into a number. If `value` is already a number, it's returned directly. If `value` is a string that represents a valid number, it's converted and returned. Throws a runtime error if the string cannot be parsed into a number or if more/less than one argument is provided. |
| *(More built-ins can be added here)*           |                                                                                                                                           |                                                                                                                                        |

Example:
```aleng
Print("Hello", 123, "Aleng user")

parsedNumber = ParseNumber("25")
Print(parsedNumber + 5) // 30
```

---

## 5. Modules

Aleng supports organizing code into modules, which are separate `.aleng` files.

### Importing Modules

You can import other Aleng files using the `Module` keyword followed by the module name (without the `.aleng` extension) as a string.

```aleng
// In main.aleng
Module "utils" // Imports utils.aleng

// Now functions/variables defined in utils.aleng (at the top level)
// are available in main.aleng's global scope.
DoSomethingUseful() // Assuming DoSomethingUseful is defined in utils.aleng
```

When a module is imported:
*   Its code is executed in the context of the current visitor (interpreter state).
*   Top-level function definitions and variable assignments in the module become part of the importer's global scope.
*   A module is imported and executed only once, even if `Module "name"` is encountered multiple times.

### Module Resolution

When `Module "mymodule"` is encountered:
1.  The interpreter looks for a file named `mymodule.aleng`.
2.  The search path for modules is typically within the same workspace/directory structure as the main script being executed. The interpreter collects all `.aleng` files (except `main.aleng`) in the workspace and maps their stem (filename without extension) to their path.

Example structure:

```
my_project/
├── main.aleng
└── lib/
    └── math_utils.aleng
```

In `main.aleng`:
```aleng
Module "math_utils" // This would try to find math_utils.aleng

result = AddNumbers(5,3) // If AddNumbers is in math_utils.aleng
Print(result)
```
The interpreter needs to be configured or started in a way that `my_project/` is considered the workspace so `math_utils` can be found.

---

## 6. Examples

**Example 1: Basic Hello World & Variables**
```aleng
// main.aleng
Print("Hello, Aleng!")

name = "Developer"
Print("Welcome, " + name)

age = ReadFile("age.txt") // Assuming age.txt contains a number
Print(name + " is " + age + " years old.")
```

**Example 2: Functions and Conditions**
```aleng
// main.aleng
Fn CheckAge(userAge: NUMBER)
    If userAge >= 18
        Print("Adult")
    Else
        Print("Minor")
    End
End

Fn GetFullName(first: STRING, last: STRING)
    first + " " + last
End

myAge = 25
CheckAge(myAge)

fullName = GetFullName("Aleng", "Script")
Print("Full name: " + fullName)
```

**Example 3: Using a Module**
```aleng
// utils.aleng
Fn SayGoodbye()
    Print("Goodbye from utils module!")
End

PI = 3.14159
```

```aleng
// main.aleng
Module "utils"

Print("Accessing PI from utils: " + PI)
SayGoodbye()

Fn CalculateCircumference(radius: NUMBER)
    2 * PI * radius // Uses PI from the utils module
End

circum = CalculateCircumference(10)
Print("Circumference: " + circum)
```

---

## 7. Roadmap (Potential Future Features)

*   **More Data Types:** Booleans, Lists/Arrays, Dictionaries/Maps.
*   **Looping Constructs:** `For` loops, `While` loops.
*   **Explicit `Return` statement:** For more control over function exit points.
*   **Error Handling:** `Try/Catch` blocks or similar mechanisms.
*   **Enhanced Variadic Argument Handling:** Ways for Aleng script to iterate or access collected variadic arguments.
*   **Standard Library:** A richer set of built-in functions and modules for common tasks (file I/O, math, string manipulation, etc.).
*   **Object-Oriented Features:** Basic classes and objects.
*   **Better Scoping for Blocks:** `If`/`Else` blocks creating their own scopes.

---

## 8. Contributing

Contributions to Aleng are welcome! If you'd like to contribute, please:
1.  Fork the repository.
2.  Create a new branch for your feature or bug fix.
3.  Make your changes.
4.  Add tests for your changes, if applicable.
5.  Ensure your code adheres to the existing style.
6.  Submit a pull request with a clear description of your changes.

If you find any bugs or have suggestions, please open an issue on GitHub.