#!/bin/bash
set -ev
cd scheme
make
build/scheme src/fib.scm
