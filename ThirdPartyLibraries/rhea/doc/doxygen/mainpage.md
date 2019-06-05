Rhea documentation {#mainpage}
==================

Rhea is an incremental constraint solver based on 
[Cassowary](http://www.cs.washington.edu/research/constraints/cassowary), 
originally developed by Greg J. Badros and Alan Borning. 

Example
-------

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <rhea/simplex_solver.hpp>
#include <rhea/iostream.hpp>

main()
{
    rhea::variable x (0), y (0);
    rhea::simplex_solver solver;

    solver.add_constraints(
    {
        x <= y,
        y == x + 3,
        x == 10
    });

    std::cout << x << " " << y << std::endl;
    // Prints "10 13"
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

