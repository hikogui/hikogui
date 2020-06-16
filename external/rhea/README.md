Rhea
====
[![Linux build status](https://travis-ci.org/Nocte-/rhea.png?branch=master)](https://travis-ci.org/Nocte-/rhea) [![Windows build status](https://ci.appveyor.com/api/projects/status/vnt20tikld91en78?svg=true)](https://ci.appveyor.com/project/Nocte-/rhea) [![Coverage Status](https://coveralls.io/repos/Nocte-/rhea/badge.png)](https://coveralls.io/r/Nocte-/rhea) [![Readthedocs build](https://readthedocs.org/projects/rhea/badge/?version=latest)](https://readthedocs.org/projects/rhea/)
[![Slack Status](https://overconstrained-slack.herokuapp.com/badge.svg)](https://overconstrained-slack.herokuapp.com)

About
-----
Rhea is an incremental constraint solver based on 
[Cassowary](http://www.cs.washington.edu/research/constraints/cassowary), 
originally developed by Greg J. Badros and Alan Borning.  The main
differences are:

 * Allows the programmer to write constraints in a natural way
 * Rewritten in C++11
 * CMake instead of GNU Autoconfig
 * Unit tests use the Boost Test Framework
 * Uses Doxygen for documentation
 * Expression parser based on Boost Spirit
 * Does not have a finite domain subsolver 


Quick example
-------------

```c++
#include <rhea/simplex_solver.hpp>
#include <rhea/iostream.hpp>

main()
{
    rhea::variable left, mid, right;
    rhea::simplex_solver solver;

    solver.add_constraints(
    {
        mid == (left + right) / 2,
        right == left + 10,
        right <= 100,
        left >= 0
    });
    solver.suggest(mid, 2);

    std::cout << left << " " << mid << " " << right << std::endl;
    // Prints "0 5 10"
}
```

This is the line example from the original paper.  The constraints make sure
the line is at least 10 wide, fits inside the 0..100 range, and that the mid
point is halfway the left and the right.

Note that even though we suggest the mid point to be at 2, the solver decides
to move it over to 5 so all the constraints can be met.


Status
------
This software is beta.  It does pass all unit tests, it is in active use
by several applications, but the interface is not stable yet.

License
-------
Rhea is free software: you can redistribute it and/or modify it under the
terms of the [MIT/Expat license](https://opensource.org/licenses/MIT).

Links
-----
Curious about similar projects? Head over to [overconstrained.io](http://overconstrained.io).
