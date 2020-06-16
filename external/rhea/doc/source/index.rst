Rhea
====

A C++11 implementation of the `Cassowary constraint-solving algorithm`_.
Cassowary is the algorithm that forms the core of the OS X and iOS `user
interface layout mechanism`_. Rhea is a lighter, friendlier version of the
original implementation by Greg J. Badros and Alan Borning.

This manual is based on `PyBee Cassowary`_'s documentation, written by Russell
Keith-Magee.

.. _Cassowary constraint-solving algorithm: http://www.cs.washington.edu/research/constraints/cassowary/
.. _user interface layout mechanism: https://developer.apple.com/library/ios/documentation/UserExperience/Conceptual/AutolayoutPG/Introduction/Introduction.html
.. _Pybee Cassowary: https://github.com/pybee/cassowary

Quickstart
----------

Grabbing, building, and installing the library works pretty much like any
other CMake project::

    $ git clone https://github.com/Nocte-/rhea
    $ mkdir rhea-build
    $ cd rhea-build
    $ cmake ../rhea
    $ make
    $ sudo make install

main.cpp::

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
    }

Build and run::

    $ g++ --std=c++11 main.cpp -lrhea
    $ ./a.out
    0.000 5.000 10.000

User guide
----------

.. toctree::
   :maxdepth: 2

   theory
   examples

Contribute
----------

* Browse the source on Github_.
* `Issue tracker`_.

.. _Github: https://github.com/Nocte-/rhea
.. _Issue tracker: https://github.com/Nocte-/rhea/issues


API reference
-------------

.. toctree::
   ref/class_variable
   ref/class_constraint
   ref/class_dummy_variable
   ref/class_abstract_variable
   ref/class_linear_equation
   ref/class_linear_expression
   ref/class_simplex_solver
