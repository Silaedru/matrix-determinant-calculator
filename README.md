# Matrix determinant calculator

Rather simple calculator of a matrix determinant written in C++11. 

This project uses [boost multiprecision library](http://www.boost.org/doc/libs/1_65_1/libs/multiprecision/doc/html/index.html) for calculations so if you want to build it you will need boost libraries (tested with version 1.65.1), or you can just change the typedef in ```Matrix.h``` to anything you prefer.

The calculator contains *no optimizations* of the actual calculation, however the implemented gauss elimination should work with any matrix.
