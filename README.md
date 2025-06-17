# Aleng Language

Welcome to Aleng, a fun interpreted language built with C++! This project was created for enjoyment and educational purposes. Feel free to contribute or simply experiment with it!

You can find the project repository here: https://github.com/Aletropy/aleng 

## Getting Started

To begin using Aleng, you first need to build the project. See the [Building Aleng](#building-aleng) section for instructions.

Once built, you can use Aleng in two primary ways:

### Interactive Mode (REPL)

The Read-Eval-Print Loop (REPL) allows you to execute Aleng code line by line interactively. To start the REPL, run the Aleng executable with the --repl argument:

```
Aleng --repl
```

Example:

```bash  
Aleng --repl

Aleng$ Print("Hello, Aleng!")
Hello, Aleng!
Aleng$ x = 10
Aleng$ Print(x * 2)
20
Aleng$ .exit
```

### Running a Project or Script

Aleng can automatically detect and run a project by looking for a main.aleng file in the current directory or a specified directory.

If you run the Aleng executable without any arguments, it will search recursively for main.aleng in the current and parent directories and use that file's directory as the project workspace.

```bash
# Assuming you are in the project root containing main.aleng
Aleng
```

Alternatively, you can explicitly specify the project directory or the main file:

```bash      
# Specify the project directory
Aleng /path/to/your/aleng/project/

# Specify the main file
Aleng /path/to/your/aleng/project/src/main.aleng
``` 

Example (Assuming you have a file named my_script.aleng):

```bash
# my_script.aleng
name = "World"
Print("Hello, " + name + "!")
```

To run this script:

```bash      
Aleng my_script.aleng
```

Output:

```
Hello, World!
```

## Building Aleng

To build the Aleng interpreter from source, follow the instructions for your operating system. You will need cmake, gcc (or another C++ compiler), and ninja installed. glfw is also required.

### On Arch Linux

1. **Install Dependencies**: Use yay or your preferred AUR helper to install the necessary packages:

```bash          
yay -S cmake gcc ninja glfw
```

2. **Clone the Repository**: Get the source code from GitHub:

```bash
git clone https://github.com/Aletropy/aleng.git
```

3. **Build**: Create a build directory, navigate into it, configure the build with CMake, and compile using Ninja:

```bash
mkdir build && cd build
cmake -G Ninja ..
ninja
```

The Aleng executable will be found in the build directory.

### For Other Distributions

While specific steps may vary between Linux distributions (like Debian, Ubuntu, Fedora, etc.), the general process is the same:

1. Install `cmake`, a C++ compiler (like `g++`), `ninja`, and `glfw3`. Package names might differ (e.g., `libglfw3-dev` on Debian/Ubuntu).

2. Clone the repository.

3. Follow the build steps outlined for Arch Linux (creating a build directory, running `cmake` and `ninja`).

Refer to the documentation for your specific distribution on how to install these packages.