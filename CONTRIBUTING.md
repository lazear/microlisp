# Contribution Guide

### For All Projects

* By contributing, you agree to release your contributions under the MIT License (default), or whatever the LICENSE file being used is.
* Please do not include any external libraries that are not already used by the project. Adding in new source files is OK, but in general, I don't like using external libraries for my projects. 
* Before submitting a PR, make sure that the code compiles. 
* Try to maintain format/style

### For C Projects

* Compilation: Typically, try to compile using the provided Makefile settings/CFLAGS. The goal is to compile most projects with `-Wall -Wextra -Werror`, although exceptions are allowed.
* Formatting: 4 spaces to a tab. If a .clang-format file is provided, please format code using clang-format before submitting a PR

### For Rust Projects

* Compilation: Code should compile without warnings or errors. 
* Testing: Code should have tests, where applicable
* Formatting: Use `rustfmt`