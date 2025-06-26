# Aleng Programming Language

**Aleng** is a lightweight, dynamically-typed scripting language designed for simplicity and embeddability. It features a straightforward syntax, support for user-defined functions, modules, lists, booleans, for-loops, and a growing set of built-in capabilities.

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
        *   [Booleans](#booleans)
        *   [Lists](#lists)
    *   [Variables and Assignment](#variables-and-assignment)
    *   [Operators](#operators)
        *   [Arithmetic Operators](#arithmetic-operators)
        *   [Comparison Operators](#comparison-operators)
        *   [Logical Operators](#logical-operators)
        *   [Operator Precedence](#operator-precedence)
    *   [Control Flow](#control-flow)
        *   [If-Else Statements](#if-else-statements)
        *   [For Loops](#for-loops)
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

*   **Single-line comments:** Start with `#` and extend to the end of the line.
*   **Multi-line comments:** Enclosed between `##` and `##`.

```aleng
# This is a single-line comment
x = 10 # This is also a comment

##
This is a
multi-line comment.
It can span several lines.
##
y = 20
```

### Data Types

Aleng is dynamically typed. Variables can hold values of different types, and their type is determined at runtime.

#### Numbers

Numbers in Aleng are represented as double-precision floating-point values internally, though integer literals can be used.

```aleng
age = 30
price = 19.99
negative = -5.0
count = 100
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

#### Booleans

Boolean values are `True` and `False`. They are primarily used in conditional expressions.

```aleng
isValid = True
isComplete = False
canProceed = (age >= 18) # result will be True or False
```

#### Lists

Lists are ordered collections of items, enclosed in square brackets `[]`. Items can be of any data type, including other lists.

```aleng
numbers = [1, 2, 3, 4, 5]
mixed = [10, "apple", true, 3.14]
empty_list = []
nested_list = [1, [2, 3], 4]

# Accessing elements (0-indexed)
first_number = numbers[0] # 1
second_item = mixed[1]    # "apple"

# Modifying elements
numbers[0] = 100
Print(numbers) # [100, 2, 3, 4, 5]
```

### Variables and Assignment

Variables are declared and assigned values using the `=` operator. Variable names must start with a letter or underscore (`_`) and can be followed by letters, numbers, or underscores.

```aleng
myVariable = 123
user_name = "Alice"
_value = true
PI = 3.14159
dataList = [10, 20]

# Reassignment
myVariable = "Now I am a string"
dataList = []
```

### Operators

#### Arithmetic Operators

For `NUMBER` operands:
*   `+`: Addition
*   `-`: Subtraction
*   `*`: Multiplication
*   `/`: Division (always floating-point division)

```aleng
sum = 10 + 5       # 15.0
difference = 20 - 7  # 13.0
product = 4 * 6      # 24.0
quotient = 10 / 4    # 2.5
```

Special behavior with `STRING` and `NUMBER`:
*   `STRING + STRING`: Concatenation
    ```aleng
    greeting = "Hello, " + "world!" # "Hello, world!"
    ```
*   `STRING + NUMBER` (or `NUMBER + STRING`): Concatenation (number is converted to string)
    ```aleng
    label = "Value: " + 100 # "Value: 100"
    ```
*   `STRING * NUMBER`: String repetition
    ```aleng
    stars = "*" * 5 # "*****"
    ```
*   `LIST + LIST`: List concatenation
    ```aleng
    list1 = [1, 2]
    list2 = [3, 4]
    combined = list1 + list2 # [1, 2, 3, 4]
    ```

#### Comparison Operators

These operators return `true` or `false`.
*   `==`: Equal to
*   `!=`: Not equal to
*   `<`: Less than
*   `>`: Greater than
*   `<=`: Less than or equal to
*   `>=`: Greater than or equal to

```aleng
Print(5 == 5)     # true
Print(5 == 6)     # false
Print("hi" == "hi") # true
Print("a" != "b") # true
Print(10 < 20)    # true
Print(10 > 20)    # false
Print(5 >= 5)     # true
Print(5 <= 4)     # false

Print(10 == "10") # false (types must match for equality without coercion)
# List and Map comparisons currently check for reference equality, not deep content equality.
list_a = [1,2]
list_b = [1,2]
Print(list_a == list_b) # false (different objects in memory)
list_c = list_a
Print(list_a == list_c) # true (same object)
```
Comparison between different types (e.g., a number and a string) will generally result in `false` for `==` and `true` for `!=`.

#### Logical Operators

Logical operators combine boolean values. They use short-circuit evaluation.
*   `And`: Logical AND (e.g., `expr1 And expr2`)
*   `Or`: Logical OR (e.g., `expr1 Or expr2`)
*   `Not`: Logical NOT (e.g., `Not expr`)

```aleng
age = 25
isStudent = true

If age >= 18 And isStudent
    Print("Adult student")
End

If age < 18 Or isStudent
    Print("Minor or student")
End

isNotStudent = Not isStudent
Print(isNotStudent) # false
```

#### Operator Precedence

Aleng follows standard operator precedence (highest to lowest):
1.  Parentheses `()`
2.  `Not`
3.  Multiplication `*`, Division `/` (left-to-right)
4.  Addition `+`, Subtraction `-` (left-to-right)
5.  Comparison `==`, `!=`, `<`, `>`, `<=`, `>=` (left-to-right)
6.  `And` (left-to-right)
7.  `Or` (left-to-right)
8.  Assignment `=` (right-to-left, and is part of an expression)

```aleng
result = 10 + 5 * 2   # result is 20.0
result2 = (10 + 5) * 2 # result2 is 30.0
cond = true Or false And false # cond is true (And has higher precedence than Or)
cond2 = (true Or false) And false # cond2 is false
```

### Control Flow

#### If-Else Statements

`If` statements allow conditional execution of code. An optional `Else` branch can be provided. The structure must be closed with `End`.

The condition is considered "truthy" if it's `true`, a non-zero number, a non-empty string, or a non-empty list/map. It's "falsy" if it's `false`, `0.0`, an empty string, or an empty list/map.

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
If name # Truthy because non-empty string
    Print("Name is not empty: " + name)
Else
    Print("Name is empty")
End

items = []
If items # Falsy because empty list
    Print("List has items")
Else
    Print("List is empty")
End
```
Nested `If` statements are allowed.

#### For Loops

`For` loops provide ways to iterate over sequences or ranges.

**1. Iterating over a Collection (e.g., List):**
```aleng
myList = ["apple", "banana", "cherry"]
For fruit in myList
    Print("I like " + fruit)
End
# Output:
# I like apple
# I like banana
# I like cherry

numbers = [1, 2, 3]
sum = 0
For num in numbers
    sum = sum + num
End
Print("Sum of numbers: " + sum) # Sum of numbers: 6
```
The variable (`fruit`, `num`) takes on the value of each element in the collection successively.

**2. Numeric Range Loop:**
Syntax: `For variable = start_expr .. end_expr [step step_expr]`
Syntax: `For variable = start_expr until end_expr [step step_expr]`

*   `..` (range operator): Includes the `end_expr` if the step allows.
*   `until`: Excludes the `end_expr`.
*   `step step_expr` (optional): Specifies the increment/decrement. Defaults to `1` if `start_expr <= end_expr`, and `-1` if `start_expr > end_expr`.

```aleng
# Count from 1 to 3 (inclusive)
For i = 1 .. 3
    Print(i)
End
# Output: 1.0, 2.0, 3.0

# Count from 0 up to (but not including) 5
For k = 0 until 5
    Print(k)
End
# Output: 0.0, 1.0, 2.0, 3.0, 4.0

# Count down from 3 to 1 with explicit step
For j = 3 .. 1 step -1
    Print(j)
End
# Output: 3.0, 2.0, 1.0

# Count by 2s
For n = 0 .. 10 step 2
    Print(n)
End
# Output: 0.0, 2.0, 4.0, 6.0, 8.0, 10.0
```
The loop variable is local to the `For` loop's scope.

### Expressions and Statements

Aleng is largely an expression-oriented language. Many constructs, like assignments and function calls, are expressions that evaluate to a value. Statements are units of execution. A program is a sequence of statements.

```aleng
# Expression statement
10 + 5

# Assignment expression (evaluates to the assigned value)
a = (b = 20) + 5 # b becomes 20, a becomes 25

# Function call expression
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
    Print("Sum: " + sum) # Last expression is implicitly returned
    sum # Explicitly make 'sum' the last expression for clarity if needed
End
```

### Parameters

#### Untyped Parameters

By default, parameters are untyped and can accept any value.

```aleng
Fn Describe(item)
    Print("The item is: " + item)
End

Describe(123)       # "The item is: 123"
Describe("a book")  # "The item is: a book"
Describe(true)      # "The item is: true"
Describe([1,2])     # "The item is: [1, 2]"
```

#### Typed Parameters

You can optionally specify a type for a parameter using a colon (`:`) followed by the type name (`NUMBER`, `STRING`, `BOOLEAN`, `LIST`, `ANY`). If a type is specified, Aleng will perform a runtime type check when the function is called. If the argument's type doesn't match, a runtime error will occur. `ANY` skips the type check for that parameter.

```aleng
Fn WelcomeUser(name: STRING, age: NUMBER, data: LIST)
    Print("Welcome, " + name + "! You are " + age + " years old.")
    Print("Data received: " + data)
End

WelcomeUser("Alice", 30, [10, 20]) # OK

# WelcomeUser(30, "Alice", []) # This would cause a runtime error (type mismatch for name/age)
# WelcomeUser("Bob")           # Runtime error (not enough arguments)
```

#### Variadic Parameters

A function can accept a variable number of arguments using the `$` prefix on the *last* parameter. This parameter will "collect" all remaining arguments passed to the function into a list. An optional type can also be specified for variadic parameters (e.g., `$values: NUMBER`), in which case all collected arguments must match that type before being put into the list.

```aleng
Fn PrintAll($items) # $items will be a list of all passed arguments
    Print("Received items:")
    For item in items # Iterate over the collected list
        Print("- " + item)
    End
End

Fn SumNumbers(initialValue: NUMBER, $values: NUMBER)
    # $values will be a list containing only numbers
    currentSum = initialValue
    For val in values
        currentSum = currentSum + val
    End
    currentSum # Return the sum
End

PrintAll(1, "two", true, 3.0)
# Output:
# Received items:
# - 1
# - two
# - true
# - 3

total = SumNumbers(10, 20, 30, 5) # total will be 10 + 20 + 30 + 5 = 65
Print(total)
# SumNumbers(10, "oops", 30) # Runtime error: type mismatch for variadic $values
```

### Calling Functions

Functions are called by using their name followed by parentheses containing the arguments.

```aleng
Greet()

result = Add(10, 25) # 'result' will be 35.0 because Add implicitly returns 'sum'
Print("Result from Add: " + result)
```

### Return Values

Functions in Aleng implicitly return the value of the *last executed statement (expression)* in their body. There is no explicit `return` keyword.

```aleng
Fn GetValue()
    x = 100
    x * 2 # This value (200.0) will be returned
End

Fn GetGreeting(name: STRING)
    "Hello, " + name + "!" # This string is returned
End

Fn IsPositive(num: NUMBER)
    num > 0 # This boolean (true/false) is returned
End

val = GetValue()       # val is 200.0
msg = GetGreeting("Bob") # msg is "Hello, Bob!"
isPos = IsPositive(-5) # isPos is false
Print(val)
Print(msg)
Print(isPos)
```
If a function body is empty or its last statement doesn't produce a meaningful value (e.g., an `If` statement where the condition is false and there's no `Else`), the return value might be a default (like `0.0` or `false`).

### Scope

Aleng uses lexical scoping (also known as static scoping).
*   Variables defined inside a function (including parameters and `For` loop iterators) are local to that function/loop and are not accessible outside of it.
*   Functions can access variables defined in their containing (outer) scopes if not shadowed by a local variable.
*   Currently, all top-level definitions (variables and functions outside any other function) are in a global scope.

```aleng
globalVar = "I am global"

Fn MyFunction()
    localVar = "I am local to MyFunction"
    Print(localVar)
    Print(globalVar) # Can access globalVar
End

Fn Outer()
    outerVar = "I am in Outer"
    Fn Inner()
        innerVar = "I am in Inner"
        Print(innerVar)
        Print(outerVar)  # Can access outerVar from Outer's scope
        Print(globalVar) # Can access globalVar
    End
    Inner()
    # Print(innerVar) # Error: innerVar is not accessible here
End

MyFunction()
Outer()
# Print(localVar) # Error: localVar is not accessible here

For i = 0 .. 1
    loopVar = i * 10
    Print(loopVar)
End
# Print(loopVar) # Error: loopVar is not accessible here
# Print(i)       # Error: i is not accessible here
```

---

## 4. Built-in Functions

Aleng provides a set of built-in functions:

| Function                                        | Description                                                                                                                               |
| :---------------------------------------------- | :---------------------------------------------------------------------------------------------------------------------------------------- |
| `Print(arg1, arg2, ...)`                        | Prints one or more arguments to the console. Arguments are converted to strings if necessary. Each argument is printed on a new line.         |
| `IsString(value: ANY) -> BOOLEAN`               | Checks if the provided `value` is a string. Returns `true` or `false`.                                                                    |
| `IsNumber(value: ANY) -> BOOLEAN`               | Checks if the provided `value` is a number. Returns `true` or `false`.                                                                    |
| `IsBoolean(value: ANY) -> BOOLEAN`              | Checks if the provided `value` is a boolean. Returns `true` or `false`.                                                                   |
| `IsList(value: ANY) -> BOOLEAN`                 | Checks if the provided `value` is a list. Returns `true` or `false`.                                                                      |
| `ParseNumber(value: ANY) -> NUMBER`             | Attempts to parse the given `value` into a number. If `value` is already a number, it's returned directly. If `value` is a string that represents a valid number, it's converted and returned. Throws a runtime error if the string cannot be parsed into a number or if more/less than one argument is provided. |
| `len(collection: ANY) -> NUMBER`                | Returns the length of a string or the number of elements in a list. Throws an error if the argument is not a string or list.

Example:
```aleng
Print("Hello", 123, true, [1, "end"])

parsedNumber = ParseNumber("25.5")
Print(parsedNumber + 5) # 30.5

myString = "Aleng"
Print(len(myString)) # 5

myList = [10, 20, 30]
Print(len(myList)) # 3

append(myList, 40)
Print(myList) # [10, 20, 30, 40]

lastItem = pop(myList)
Print(lastItem) # 40
Print(myList) # [10, 20, 30]

itemAtIndex = pop(myList, 0)
Print(itemAtIndex) # 10
Print(myList) # [20, 30]
```

---

## 5. Modules

Aleng supports organizing code into modules, which are separate `.aleng` files.

### Importing Modules

You can import other Aleng files using the `Module` keyword followed by the module name (without the `.aleng` extension) as a string.

```aleng
# In main.aleng
Module "utils" # Imports utils.aleng

# Now functions/variables defined in utils.aleng (at the top level)
# are available in main.aleng's global scope.
DoSomethingUseful() # Assuming DoSomethingUseful is defined in utils.aleng
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
Module "math_utils" # This would try to find math_utils.aleng

result = AddNumbers(5,3) # If AddNumbers is in math_utils.aleng
Print(result)
```
The interpreter needs to be configured or started in a way that `my_project/` is considered the workspace so `math_utils` can be found.

---

## 6. Examples

**Example 1: Basic Hello World & Variables**
```aleng
# main.aleng
Print("Hello, Aleng!")

name = "Developer"
Print("Welcome, " + name)

isValid = true
Print("Is valid: " + isValid)
```

**Example 2: Functions, Conditions, and Lists**
```aleng
# main.aleng
Fn CheckAge(userAge: NUMBER)
    If userAge >= 18 And userAge < 65
        Print("Adult")
    Else If userAge >= 65
        Print("Senior")
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

scores = [88, 92, 75]
For score in scores
    Print("Score: " + score)
End
```

**Example 3: Using a Module and For Loop**
```aleng
# utils.aleng
Fn SayGoodbye()
    Print("Goodbye from utils module!")
End

PI = 3.14159
```

```aleng
# main.aleng
Module "utils"

Print("Accessing PI from utils: " + PI)
SayGoodbye()

Fn CalculateCircumference(radius: NUMBER)
    2 * PI * radius # Uses PI from the utils module
End

For r = 1 .. 3
    circum = CalculateCircumference(r)
    Print("Circumference for radius " + r + ": " + circum)
End
```

---

## 7. Roadmap (Potential Future Features)

*   **More Data Types:** Dictionaries/Maps.
*   **Looping Constructs:** `While` loops.
*   **Explicit `Return` statement:** For more control over function exit points.
*   **Error Handling:** `Try/Catch` blocks or similar mechanisms.
*   **Enhanced Variadic Argument Handling:** Done! (Collected as a list).
*   **Standard Library:** A richer set of built-in functions and modules for common tasks (file I/O, more math, string manipulation, etc.).
*   **Object-Oriented Features:** Basic classes and objects.
*   **Better Scoping for Blocks:** `If`/`Else`/`For` blocks creating their own scopes (Currently only Functions and `For` loop iterators have distinct inner scopes).
*   **Break/Continue:** For loops.
*   **Deep equality for lists/maps.**

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