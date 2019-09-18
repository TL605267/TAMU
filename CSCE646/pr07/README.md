# CSCE646 Project7: wrapping
### Tong Lu
## Table of Contents
1. [Problem Description](#problem-description)
2. [Project Requirements](#project-requirements)
3. [Build Instructions](#build-instructions)
4. [Demo Instructions](#demo-instructions)

## Problem Description
You will develop a warping tool that provide bilinear and a general transformation.

## Project Requirements
You are supposed to implement following methods to get the full credit. Each one of the is equally weighted. To get the credit, your results must not have unwanted artifacts such as holes. To avoid these artifacts, you will need to use inverse mapping along with antialasing techniques you used before.

1. Bilinear warping
2. Any interesting inverse transformation that can be provided by an inverse function

Bonus: Any additional work with bi-quadric or bi-cubic tranformations can give you bonus points up to half of the project credit. In this case, you need to use a forward transformation and open-GL.
For the required parts of the project, , implement your operations only using the basic programming operations such as while or for loops and basic mathematical operations such as addition, subtraction, multiplication, division and power. In other words, no high level operation provided by some programming languages such as matrix multiplication or matrix inversion is allowed.

## Code Description
For part one, my program can transform the image as following:

[0,0] =====> [187,33]

[510,0] ===> [363,134]

[0,340] ===> [114,158]

[510,340] => [282,225]

For part two, my program can generate a special effect on a given image using sine wave. It will also fix all the unwanted artifacts using inverse mapping. 

## Build Instructions

make

## Demo Instructions

(mellieha.ppm is the example picture in the folder for demostration purpose)

### Bilinear

./pr07 bilinear mellieha.ppm

### Special Effect

./pr07 special mellieha.ppm