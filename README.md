# Aleng Language

Just a funny interpreted language made in C++, for fun and educational purposes.
Feel free to contribute or just play with it.

# Usage

See [How to build](#how-to-build) to get started.

## REPL Mode

To use *REPL* mode, just runs the program with the `--repl` argument as shown below.

```
Aleng --repl
```

## Run a project

Aleng will automaticaly runs a project if you execute it without any extra steps. it should recursivelly look for a `main.aleng` file and use it's parent folder as the workspace folder.

Alternativally you can pass the workspace dir or the main file to the first argument, as the example below:

```
Aleng /home/user/projects/aleng/
Aleng /home/user/projects/aleng/src/main.aleng

// Or just
cd /home/user/projects/aleng/
Aleng
```

# How to build?

## Arch

1. Install the required dependencies

    ```
    yay -S cmake gcc ninja glfw
    ```

2. Clone the repository

    ```
    git clone https://github.com/Aletropy/aleng
    ```

3. Build the project

    ```
    mkdir build && cd build
    cmake -G Ninja ..
    ninja
    ```

## Other Distributions

I'll not provide a step-by-step in how to build this in every distro. But you can look for cmake and ninja building and you're fine.