Constraint solving systems
==========================

Constraint solving systems are an algorithmic approach to solving linear
programming problems. A linear programming problem is a mathematical problem
where you have a set of non-negative, real-valued variables (:math:`x_1, x_2,
\dotsc x_n`), and a series of linear constraints on those variables. These
constraints are expressed as a set of equations of the form:

.. math::

    \begin{align*}
    a_1 x_1 + a_2 x_2 + \dotsc + a_n x_n &= b \\
    &\leq b , or: \\
    &\geq b , or:
    \end{align*}

Given these constraints, the problem is to find the values of :math:`x_i` that
minimizes or maximizes the value of an **objective function**:

.. math::

    c + d_1 x_1 + d_2 x_2 + \dotsc + d_n x_n

Cassowary is an algorithm designed to solve linear programming problems of
this type. Published in 1997, it now forms the basis fo the UI layout  tools
in OS X Lion, and iOS 6+ (the approach known as `Auto Layout`_). The Cassowary
algorithm (and this implementation of it) provides the tools to describe a set
of constraints, and then find an optimal solution for it.

.. _Auto Layout: https://developer.apple.com/library/ios/documentation/userexperience/conceptual/AutolayoutPG/Introduction/Introduction.html

Variables
---------

At the core of the constraint problem are the variables in the system.
In the formal mathematical system, these are the :math:`x_i` terms; in C++,
these are instances of the :class:`~rhea::variable` class::

    #include <rhea/variable.hpp>

    // Create a variable with a default value.
    rhea::variable x1;

    // Create a variable with a specific value.
    rhea::variable x2 {42.0};

Any value provided for the variable is just a starting point. When constraints
are imposed, this value can and will change, subject to the requirements of the
constraints. However, providing an initial value may affect the search process;
if there's an ambiguity in the constraints (i.e., there's more than one
possible solution), the initial value provided to variables will affect the
solution on which the system converges.

Constraints
-----------

A constraint is a mathematical equality or inequality that defines the linear
programming system. Its declaration in C++ looks essentially the same as the
mathematical expression::

    rhea::variable x1, x2, x3;

    // Define the constraint
    auto constraint = x1 + 3 * x2 <= 4 * x3 + 2;

In this example, `constraint` holds the defintion for the constraint system.
Although the statement uses the C++ comparison operator `<=`, the result is
*not* a boolean. The comparison operators `<=`, `>=`, and `==` have
been overridden for instances of :class:`~rhea::linear_expression` to enable
you to easily define constraints.

In a similar way, the operators `+`, `-`, `*`, and `/` have been overridden
for :class:`~rhea::variable` to easily create linear expressions.

Solvers
-------

The solver is the engine that resolves the linear constraints into a solution.
There are many approaches to this problem, and the development of algorithmic
approaches has been the subject of math and computer science research for over
70 years. Rhea provides one implementation -- a :class:`~rhea::simplex_solver`,
implementing the Simplex algorithm defined by Dantzig in the 1940s.

The solver takes no arguments during constructions; once constructed, you simply
add constraints to the system.

As a simple example, let's solve the problem posed in Section 2 of the `Badros
& Borning's paper on Cassowary`_. In this problem, we have a 1-dimensional
number line spanning from 0 to 100. There are three points on it (left, middle
and right), with the following constraints:

* The middle point must be halfway between the left and right point;
* The left point must be at least 10 to the left of the right point;
* All points must fall in the range 0-100.

This system can be defined in C++ as follows::

    #include <rhea/simplex_solver.hpp>

    rhea::simplex_solver solver;
    rhea::variable left, middle, right;

    solver.add_constraints({
        middle == (left + right) / 2,
        right == left + 10,
        right <= 100,
        left >= 0
    });

There are an infinite number of possible solutions to this system; if we
interrogate the variables, you'll see that the solver has provided one
possible solution::

    left = 90
    middle = 95
    right = 100

.. _Badros & Borning's paper on Cassowary: http://www.cs.washington.edu/research/constraints/cassowary/cassowary-tr.pdf

Stay constraints
----------------

If we want a particular solution to our left/right/middle problem, we need to
fix a value somewhere. To do this, we add a `stay` -- a special constraint that
says that the value should not be altered.

For example, we might want to enforce the fact that the middle value should
stay at a value of 45. We construct the system as before, but add::

    middle = 45.0;
    solver.add_stay(middle);

Now when we interrogate the solver, we'll get values that reflect this fixed
point::

    left = 40
    middle = 45
    right = 50

Constraint strength
-------------------

Not all constraints are equal. Some are absolute requirements -- for example, a
requirement that all values remain in a specific range. However, other
constraints may be suggestions, rather than hard requirements.

To accomodate this, Rhea allows all constraints to have a **strength**. Strength
can be one of:

* ``required``
* ``strong``
* ``medium``
* ``weak``

``required`` constraints **must** be satisfied; the remaining strengths will
be satisfied with declining priority.

To define a strength, provide the strength value as an argument when adding
the constraint (or stay)::

    using namespace rhea;

    solver solver;
    variable x;

    // Define some non-required constraints
    solver.add_constraint(x <= 100, strength::medium());
    solver.add_stay(x, strength::strong());

By default:

* Explicit constraints are ``required``
* Edit constraints are ``strong``
* Stay constraints are ``weak``

Constraint weight
-----------------

If you have multiple constraints of the same strength, you may want to have a
tie-breaker between them. To do this, you can set a **weight**, in addition to
a strength::

    // Define some non-required constraints
    solver.add_constraint(x <= 100, strength::strong(), 10);
    solver.add_constraint(x >= 50, strength::medium(), 20);

Editing constraints
-------------------

Any constraint can be removed from a system; just retain the reference provided
when you add the constraint::

    // Define a constraint
    constraint c1 {x <= 100};
    solver.add_constraint(c1);

    // Remove it again
    solver.remove_constraint(c1);

Once a constraint is removed, the system will be automatically re-evaluated,
with the possible side effect that the values in the system will change.

But what if you want to change a variable's value without introducing a
new constraint? In this case, you can ``suggest`` a new value for it.

Here's an example of an edit context in practice::

    // Add a stay to x - try to keep it the same
    solver.add_stay(x);

    // Suggest a new value for x:
    solver.suggest(x, 42.0);

When the edit context exits, the system will re-evaluate itself, and the
variable will have the new value. However, the variable isn't guaranteed
to have the value you provided - in this case it will, but if your
constraint system has other constraints, they may affect the value of
the variable after the suggestion has been applied.
