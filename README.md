# Sequential-Logic-Solver
Tool to solve sequential logic circuits from a truth-table

- [Requirements](#requirements)
- [Build](#build)
- [Run](#run)


## Requirements

- GNU Compiler with support for C++20


## Build
Within the toplevel directory of the repository, create a build directory:
```
mkdir build
```

Then configure the cmake project:
```
cmake -S . -B build
```

Finally build with:
```
cmake --build build
```


## Run
From the toplevel directory of the repository,

...run examples with:
```
build/main
```
