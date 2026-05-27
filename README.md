# OLM (Otro lenguage más)

## Description
Personal interpreter in C++ for my programming language.

```
-- Go to projects/ to see more
func fib(n)
  si n < 2 haz
    devolver n
  fin
  devolver fib(n - 1) + fib(n - 2)
fin

escribe("El resultado es:", fib(10))
```

## Installation

```bash
git clone https://github.com/lilthread/olm.git
cd olm
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
