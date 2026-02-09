# 🧮 Numerical Analysis & Expression Solver (C)

This repository contains a **Numerical Analysis Solver** written in **C**, including an **expression tokenizer/parser system** and a collection of classical numerical methods used in engineering mathematics.

The project focuses on implementing core numerical algorithms from scratch while also supporting mathematical expression handling and evaluation.

---

## 🎯 Project Overview

This project is designed to solve various mathematical problems using numerical methods such as:

- Root finding
- Numerical differentiation
- Numerical integration
- Linear system solving
- Matrix inversion and decomposition
- Polynomial interpolation

It is a practical implementation of numerical analysis topics commonly taught in university courses.

---

## 🚀 Implemented Numerical Methods

### 🔹 Root Finding Methods
- **Bisection Method**
- **Regula Falsi (False Position) Method**
- **Newton-Raphson Method**

### 🔹 Linear Algebra / Matrix Operations
- **Matrix Inverse Calculation**
- **Cholesky Decomposition**
- **LU Decomposition**
- **Gauss-Seidel Method**

### 🔹 Numerical Differentiation
- **Numerical Derivative Approximation**

### 🔹 Numerical Integration
- **Trapezoidal Rule**
- **Simpson’s Rule**

### 🔹 Interpolation
- **Gregory-Newton Interpolation**

---

## 🧠 Expression Parser & Evaluator

In addition to numerical methods, the project includes a mathematical expression system capable of:

- Tokenizing mathematical expressions
- Parsing nested parentheses
- Supporting functions such as:
  - `sin`, `cos`, `tan`, `cot`
  - inverse trigonometric functions
  - variable-base logarithms like `log_x(...)`
- Evaluating expressions correctly with operator precedence

This makes the project more than just a numerical solver — it also includes parser-style logic similar to a lightweight math interpreter.

---

## 🛠️ Technologies Used
- **C Programming Language**
- **Dynamic Memory Allocation**
- **File Handling (if used)**
- **Linked List / Stack logic (for parsing)**
- **math.h library**

---

```bash
gcc main.c -o solver -lm
