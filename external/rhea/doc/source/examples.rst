Examples
========

The following examples demonstrate the use of Rhea in practical
constraint-solving problems.

Quadrilaterals
--------------

The "Bounded Quadrilateral" demo is the `online example`_ provided for
Cassowary.  The online example is implemented in JavaScript, but the
implementation doesn't alter the way the Cassowary algoritm is used.

.. _online example: http://www.badros.com/greg/cassowary/js/quaddemo.html

The Bounded Quadrilateral problem starts with a bounded, two-dimensional
canvas. We want to draw a quadilateral on this plane, subject to a number of
constraints.

Firstly, we set up the solver system itself::

    #include <rhea/simplex_solver.hpp>

    rhea::simplex_solver solver;

Then, we set up a convenience class for holding information about points
on a 2D plane::

    struct point
    {
        rhea::variable x, y;
    };


Now we can set up a set of points to describe the initial location of our
quadrilateral - a 190×190 square::

    auto p = std::vector<point>{{10, 10}, {10, 200}, {200, 200}, {200, 10}};
    auto m = std::vector<point>(4);

Note that even though we're drawing a quadrilateral, we have 8 points. We're
tracking the position of the midpoints independent of the corners of our
quadrilateral. However, we don't need to define the position of the midpoints.
The position of the midpoints will be set by defining constraints.

Next, we set up some stays. A stay is a constraint that says that a particular
variable shouldn't be modified unless it needs to be - that it should "stay"
as is unless there is a reason not to. In this case, we're going to set a stay
for each of the four corners - that is, don't move the corners unless you have
to::

    for (auto& i : p)
        solver.add_stays({i.x, i.y});

Now we can set up the constraints to define where the midpoints fall. By
definition, each midpoint **must** fall exactly halfway between two points
that form a line, and that's exactly what we describe - an expression that
computes the position of the midpoint. This expression is used to construct a
:class:`constraint`, describing that the value of the midpoint must
equal the value of the expression. The :class:`constraint` is then
added to the solver system::

    for (auto i = 0; i < 4; ++i)
        solver.add_constraints({
            m[i].x == p[i].x / 2 + p[i + 1 % 4].x / 2,
            m[i].y == p[i].y / 2 + p[i + 1 % 4].y / 2
        });

Next, lets add some constraints to ensure that the left side of the quadrilateral
stays on the left, and the top stays on top::

    solver.add_constraints({
        p[0].x + 20 <= p[2].x,
        p[0].x + 20 <= p[3].x,

        p[1].x + 20 <= p[2].x,
        p[1].x + 20 <= p[3].x,

        p[0].y + 20 <= p[1].y,
        p[0].y + 20 <= p[1].y,

        p[3].y + 20 <= p[1].y,
        p[3].y + 20 <= p[1].y
    });

Each of these constraints is posed as an :class:`~cassowary.Constraint`. For
example, the first expression describes a point 20 pixels to the right of the
x coordinate of the top left point. This :class:`~cassowary.Constraint` is
then added as a constraint on the x coordinate of the bottom right (point 2)
and top right (point 3) corners - the x coordinate of these points must be at
least 20 pixels greater than the x coordinate of the top left corner (point
0).

Lastly, we set the overall constraints -- the constraints that limit how large
our 2D canvas is. We'll constraint the canvas to be 500x500 pixels, and
require that all points fall on that canvas::

    for (auto& i : p)
        solver.add_constraints({
            i.x >= 0,
            i.y >= 0,
            i.x <= 500,
            i.y <= 500
        });

This gives us a fully formed constraint system. Now we can use it to answer
layout questions. The most obvious question -- where are the midpoints?

::

    #include <rhea/iostream.hpp>

    namespace std
    {
        ostream& operator<< (ostream& in, const point& p)
        {
            return in << "(" << p.x << ", " << p.y << ")";
        }
    }

    // ...

    for (auto& i : m)
        std::cout << i << std::endl;

Output::

    (10.0, 105.0)
    (105.0, 200.0)
    (200.0, 105.0)
    (105.0, 10.0)

You can see from this that the midpoints have been positioned exactly where
you'd expect - half way between the corners - without having to explicitly
specify their positions.

These relationships will be maintained if we then edit the position of the
corners. Lets move the position of the bottom right corner (point 2). We mark
the variables associated with that corner as being **Edit variables**::

    solver.suggest({{p[2].x, 300}, {p[2].y, 400}});

As a result of this edit, the midpoints have automatically been updated::

    for (auto& i : m)
        std::cout << i << std::endl;

Output::

    (10.0, 105.0)
    (155.0, 300.0)
    (250.0, 205.0)
    (105.0, 10.0)

If you want, you can now repeat the edit process for any of the points -
including the midpoints.

GUI layout
----------

The most common usage (by deployment count) of the Cassowary algoritm is as
the Autolayout mechanism that underpins GUIs in OS X Lion and iOS6. Although
there's lots of code required to make a full GUI toolkit work, the layout
problem is a relatively simple case of solving constraints regarding the size
and position of widgets in a window.

In this example, we'll show a set of constraints used to determine the
placement of a pair of buttons in a GUI. To simplify the problem, we'll only
worry about the X coordinate; expanding the implementation to include the Y
coordinate is a relatively simple exercise left for the reader.

When laying out a GUI, widgets have a width; however, widgets can also change
size. To accomodate this, a widget has two size constraints in each dimension:
a minimum size, and a preferred size. The miniumum size is a ``required``
constraint that must be met; the preferred size is a ``strong`` constraint
that the solver should try to accomodate, but may break if necessary.

The GUI also needs to be concerned about the size of the window that is being
laid out. The size of the window can be handled in two ways:

* a ``required`` constraint -- i.e., this *is* the size of the window;
  show me how to lay out the widgets; or

* a ``weak`` constraint -- i.e., come up with a value for the window size that
  accomodates all the other widget constraints. This is the interpretation used
  to determine an initial window size.

As with the Quadrilateral demo, we start by creating the solver, and creating
a storage mechanism to hold details about buttons::

    using namespace rhea;
    simplex_solver solver;

    struct button
    {
        variable left, width;

        linear_expression right() const { return left + width; }
        linear_expression center() const { return left + width * 0.5; }
    };

The button's edges, width, and center are all interrelated, so we don't have
to create variables for everything. We could also make variables for left and
right, and define width as an expression.

We then define our two buttons, and the variables describing the size of the
window on which the buttons will be placed::

    button b1, b2;
    variable left_limit{0}, right_limit;

    solver.add_constraint(left_limit == 0);
    solver.add_stay(right_limit, strength::weak());

The left limit is set as a required constraint -- the left border can't
move from coordinate 0. However, the window can expand if necessary to
accomodate the widgets it contains, so the right limit is a ``weak`` stay
constraint.

Now we can define the constraints on the button layouts::

    solver.add_constraints({
        // The two buttons are the same width.
        b1.width == b2.width,

        // Button1 starts 50 from the left margin.
        b1.left == left_limit + 50,

        // Button2 ends 50 from the right margin.
        left_limit + right_limit == b2.right() + 50,

        // Button2 starts at least 100 from the end of Button1. This is the
        // "elastic" constraint in the system that will absorb extra space
        // in the layout.
        b2.left == b1.right() + 100,

        // Button1 has a minimum width of 87.
        b1.width >= 87,

        // Button1's preferred width is 87.
        b1.width == 87 | strength::strong(),

        // Button2's minimum width is 113.
        b2.width >= 113,

        // Button2's preferred width is 113.
        b2.width == 113 | strength::strong()
    });

Since we haven't imposed a hard constraint on the right hand side, the
constraint system will give us the smallest window that will satisfy these
constraints::

    namespace std
    {
        ostream& operator<< (ostream& str, const button& btn)
        {
            return str << "left=" << str.left << ", width=" << str.width;
        }
    }

    std::cout << b1 << std::endl;
    // left=50, width=113

    std::cout << b2 << std::endl;
    // left=263, width=113

    std::cout << right_limit.value() << std::endl;
    // 426

That is, the smallest window that can accomodate these constraints is 426
pixels wide. However, if the user makes the window larger, we can still lay
out widgets. We impose a new ``required`` constraint with the size of the
window::

    constraint right_limit_stay {right_limit == 500};
    solver.add_constraint(right_limit_stay);

    std::cout << b1 << std::endl;
    // left=50, width=113

    std::cout << b2 << std::endl;
    // left=337, width=113

    std::cout << right_limit.value() << std::endl;
    // 500

That is - if the window size is 500 pixels, the layout will compensate by
putting ``button2`` a little further to the right. The ``weak`` stay on the
right limit that we established at the start is ignored in preference for the
``required`` stay.

If the window is then resized again, we can remove the 500 pixel limit, and
impose a new limit::

    solver.remove_constraint(right_limit_stay);
    right_limit_stay = (right_limit == 475);
    solver.add_constraint(right_limit_stay);

    std::cout << b2 << std::endl;
    // left=312, width=113

    std::cout << right_limit.value() << std::endl;
    // 475

Again, ``button2`` has been moved, this time to the left, compensating for the
space that was lost by the contracting window size.
