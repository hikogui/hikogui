
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE rhea
#include <boost/test/unit_test.hpp>

#include <random>
#include <boost/range/algorithm.hpp>

#include "../rhea/simplex_solver.hpp"
#include "../rhea/linear_equation.hpp"
#include "../rhea/iostream.hpp"
#include "../rhea/errors_expl.hpp"
#include "../rhea/link_variable.hpp"

using namespace rhea;

struct point
{
    variable x, y;

    point(double a = 0, double b = 0)
        : x(a)
        , y(b)
    {
    }

    bool operator==(const std::pair<int, int>& p) const
    {
        return x.value() == p.first && y.value() == p.second;
    }
};

namespace std
{
template <typename a, typename b>
std::ostream& operator<<(std::ostream& s, const std::pair<a, b>& p)
{
    return s << p.first << "," << p.second;
}

inline std::ostream& operator<<(std::ostream& s, const point& p)
{
    return s << p.x << " " << p.y;
}
}

BOOST_AUTO_TEST_CASE(strength_test)
{
    BOOST_CHECK(strength::required().is_required());
    BOOST_CHECK(!strength::strong().is_required());
    BOOST_CHECK(!strength::medium().is_required());
    BOOST_CHECK(!strength::weak().is_required());

    BOOST_CHECK(strength::required() > strength::strong());
    BOOST_CHECK(strength::strong() > strength::medium());
    BOOST_CHECK(strength::medium() > strength::weak());

    double z = 10000000.0;
    BOOST_CHECK(strength(0, 0, z) < strength(0, 1, 0));
    BOOST_CHECK(strength(0, z, z) < strength(1, 0, 0));
}

BOOST_AUTO_TEST_CASE(variable_test)
{
    variable a;
    variable m(variable::nil_var()), n(variable::nil_var());
    variable x(3.0);
    variable y(x);
    variable z(3.0);

    BOOST_CHECK(n.is_nil());
    n = x;
    a = y;
    BOOST_CHECK(m.is_nil());
    BOOST_CHECK(!n.is_nil());
    BOOST_CHECK(!x.is_nil());
    BOOST_CHECK(!y.is_nil());
    BOOST_CHECK(!x.is_fd());
    BOOST_CHECK(!x.is_dummy());
    BOOST_CHECK(x.is_float());
    BOOST_CHECK(x.is_external());

    BOOST_CHECK_EQUAL(x.value(), 3);
    BOOST_CHECK_EQUAL(x.int_value(), 3);
    BOOST_CHECK_EQUAL(y.value(), 3);
    BOOST_CHECK_EQUAL(a.value(), 3);

    std::hash<variable> h;
    BOOST_CHECK_EQUAL(h(x), h(y));
    BOOST_CHECK(x.is(y));
    BOOST_CHECK(h(x) != h(z));
    BOOST_CHECK(!x.is(z));
    BOOST_CHECK(a.is(x));

    y.set_value(3.7);
    BOOST_CHECK_EQUAL(n.value(), 3.7);
    BOOST_CHECK_EQUAL(x.value(), 3.7);
    BOOST_CHECK_EQUAL(x.int_value(), 4);

    y.set_value(-3.7);
    BOOST_CHECK_EQUAL(x.int_value(), -4);

    variable_set s;
    s.insert(x);
    BOOST_CHECK(s.count(x) > 0);
    BOOST_CHECK(s.count(y) > 0);

    s.erase(y);
    BOOST_CHECK(s.empty());

    BOOST_CHECK(!objective_variable().is_float());
    BOOST_CHECK(!objective_variable().is_fd());
    BOOST_CHECK(!objective_variable().is_external());
    BOOST_CHECK_EQUAL(objective_variable().value(), 0.0);
    BOOST_CHECK_EQUAL(objective_variable().int_value(), 0);
    BOOST_CHECK_EQUAL(objective_variable().to_string(), "objective");
}

BOOST_AUTO_TEST_CASE(variable_stream_test)
{
    std::stringstream s;
    s << variable{3};
    BOOST_CHECK_EQUAL("{var10:3}", s.str());
}

BOOST_AUTO_TEST_CASE(constraint_stream_test)
{
    std::stringstream s;
    s << constraint{ variable{1} + 42 == variable{2} };
    BOOST_CHECK(   s.str() == "linear [required, 1] {var11:2}*-1 + {var12:1}*1 + 42 == 0"
                || s.str() == "linear [required, 1] {var11:1}*1 + {var12:2}*-1 + 42 == 0");
}

BOOST_AUTO_TEST_CASE(strength_stream_test)
{
    std::stringstream s;
    s << strength::required();
    BOOST_CHECK_EQUAL(s.str(), "required");
}

BOOST_AUTO_TEST_CASE(linearexpr1_test)
{
    linear_expression expr1(5);
    BOOST_CHECK_EQUAL(expr1.evaluate(), 5);
    expr1 *= -1;
    BOOST_CHECK_EQUAL(expr1.evaluate(), -5);

    variable x(3.0), y(2.0);
    linear_expression expr2(x, 2.0, 1.0);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 7);
    BOOST_CHECK_EQUAL((expr2 + 2.0).evaluate(), 9);
    BOOST_CHECK_EQUAL((expr2 - 1.0).evaluate(), 6);

    expr2 += x;
    BOOST_CHECK_EQUAL(expr2.evaluate(), 10);
    expr2 -= x;
    BOOST_CHECK_EQUAL(expr2.evaluate(), 7);

    expr2 += linear_expression::term(y, 5);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 17);

    y.set_value(1);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 12);
    x.set_value(10);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 26);

    expr2 *= -1;
    BOOST_CHECK_EQUAL(expr2.evaluate(), -26);
}

BOOST_AUTO_TEST_CASE(linearexpr2_test)
{
    variable x(3);
    linear_expression test1(x, 5, 2);
    linear_expression test2(test1);

    BOOST_CHECK_EQUAL(test1.evaluate(), 17);
    BOOST_CHECK_EQUAL(test1.evaluate(), 17);

    linear_expression test3(std::move(test1));
    BOOST_CHECK_EQUAL(test3.evaluate(), 17);
}

BOOST_AUTO_TEST_CASE(linearexpr3_test)
{
    variable x(5), y(2);

    linear_expression expr(x * 2 + y - 1);
    BOOST_CHECK_EQUAL(expr.evaluate(), 11);

    x.set_value(4);
    BOOST_CHECK_EQUAL(expr.evaluate(), 9);

    BOOST_CHECK_EQUAL((x + 3).evaluate(), 7);
    BOOST_CHECK_EQUAL((x - 2).evaluate(), 2);
    BOOST_CHECK_EQUAL((x + y).evaluate(), 6);
    BOOST_CHECK_EQUAL((x - y).evaluate(), 2);
}

BOOST_AUTO_TEST_CASE(linear_equation1_test)
{
    variable x(2.0);
    linear_expression expr(x, 4.0, 1.0);
    variable answer(9.0);

    linear_equation eq1(expr, answer);
    BOOST_CHECK(eq1.is_satisfied());

    linear_expression expr2(x, 3.0, 3.0);
    linear_equation eq2(expr, expr2);
    BOOST_CHECK(eq2.is_satisfied());

    linear_equation eq3(expr, variable(42.0));
    BOOST_CHECK(!eq3.is_satisfied());
}

BOOST_AUTO_TEST_CASE(linear_equation2_test)
{
    variable x(2.0), y(3.0);

    BOOST_CHECK((x == y - 1).is_satisfied());
    BOOST_CHECK(!(x == y).is_satisfied());
    BOOST_CHECK((x * 2 == y + 1).is_satisfied());
    BOOST_CHECK(!(x * 3 == y * 4).is_satisfied());
}

BOOST_AUTO_TEST_CASE(linear_inequality1_test)
{
    variable x(2.0);
    linear_expression expr(x, 4.0, 1.0);
    variable answer(5.0);

    linear_inequality eq1(answer, relation::leq, expr);
    BOOST_CHECK(eq1.is_satisfied());
    x.set_value(0);
    BOOST_CHECK(!eq1.is_satisfied());
}

BOOST_AUTO_TEST_CASE(linear_inequality2_test)
{
    variable x(2.0), y(3.0);
    BOOST_CHECK((x <= y).is_satisfied());
    BOOST_CHECK((x + 1 <= y).is_satisfied());
    BOOST_CHECK((x * 2 + y >= 4).is_satisfied());
    BOOST_CHECK((x * 3 >= y * 2).is_satisfied());
    BOOST_CHECK(!(x >= y).is_satisfied());
}

BOOST_AUTO_TEST_CASE(constraint_map_test)
{
    variable x;
    constraint c1{x == 1};
    std::unordered_map<constraint, int> map;
    map[c1] = 5;

    constraint c2 = c1;
    BOOST_CHECK(c1 == c2);

    std::hash<constraint> hasher;
    BOOST_CHECK(hasher(c1) == hasher(c2));
    BOOST_CHECK(map.count(c1) == 1);
    BOOST_CHECK(map.count(c2) == 1);
}

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(simple1_test)
{
    variable x(167);
    variable y(2);

    simplex_solver solver;

    constraint c(std::make_shared<linear_equation>(x, linear_expression(y)));
    solver.add_stay(x);
    solver.add_stay(y);
    solver.add_constraint(c);

    BOOST_CHECK(solver.is_valid());
    BOOST_CHECK_EQUAL(x.value(), y.value());

    BOOST_CHECK(!edit_constraint(x).is_satisfied());
    BOOST_CHECK(!stay_constraint(x).is_satisfied());
}

BOOST_AUTO_TEST_CASE(simple2_test)
{
    variable x(167);

    simplex_solver solver;

    BOOST_CHECK_THROW((solver.begin_edit(),
                       solver.suggest_value(x, 100), solver.end_edit()),
                      edit_misuse);

    BOOST_CHECK_EQUAL(std::string(edit_misuse().what()),
                      "edit protocol usage violation");
}

BOOST_AUTO_TEST_CASE(constraint1_test)
{
    variable x(0);
    simplex_solver solver;
    solver.add_constraint(
        std::make_shared<linear_equation>(x, 10, strength::weak()));
    BOOST_CHECK_EQUAL(x.value(), 10.0);
}

BOOST_AUTO_TEST_CASE(juststay1_test)
{
    variable x(5), y(10);
    simplex_solver solver;

    solver.add_stay(x).add_stay(y);

    BOOST_CHECK_EQUAL(x.value(), 5);
    BOOST_CHECK_EQUAL(y.value(), 10);

    solver.suggest({{x, 6}, {y, 7}});

    BOOST_CHECK_EQUAL(x.value(), 6);
    BOOST_CHECK_EQUAL(y.value(), 7);
}

BOOST_AUTO_TEST_CASE(juststaylink1_test)
{
    float ox = 5.0f, oy = 10.0f;
    variable x(ox, linked()), y(oy, linked());
    simplex_solver solver;

    BOOST_CHECK_EQUAL(x.value(), 5);

    solver.add_stay(x).add_stay(y);

    BOOST_CHECK_EQUAL(ox, 5);
    BOOST_CHECK_EQUAL(oy, 10);

    solver.suggest({{x, 6}, {y, 7}});

    BOOST_CHECK_EQUAL(x.value(), 6);
    BOOST_CHECK_EQUAL(ox, 6);
    BOOST_CHECK_EQUAL(oy, 7);
}

BOOST_AUTO_TEST_CASE(juststaylink2_test)
{
    int ox = 5, oy = 10;
    variable x(ox, linked()), y(oy, linked());
    simplex_solver solver;

    solver.add_stay(x).add_stay(y);

    BOOST_CHECK_EQUAL(ox, 5);
    BOOST_CHECK_EQUAL(oy, 10);

    solver.suggest({{x, 6.2}, {y, 7.4}});

    BOOST_CHECK_EQUAL(ox, 6);
    BOOST_CHECK_EQUAL(oy, 7);
}

BOOST_AUTO_TEST_CASE(juststaylink3_test)
{
    double ox = 5, oy = 10;
    variable x([&](double v) { ox = v; }, 5.0);
    variable y([&](double v) { oy = v; }, 10.0);
    simplex_solver solver;

    solver.add_stay(x).add_stay(y);

    BOOST_CHECK_EQUAL(ox, 5);
    BOOST_CHECK_EQUAL(oy, 10);

    solver.suggest({{x, 6}, {y, 7}});

    BOOST_CHECK_EQUAL(ox, 6);
    BOOST_CHECK_EQUAL(oy, 7);
}

BOOST_AUTO_TEST_CASE(editleak1_test)
{
    variable x(0);
    simplex_solver solver;
    solver.add_stay(x);

    BOOST_CHECK_EQUAL(solver.columns().size(), 2);
    BOOST_CHECK_EQUAL(solver.rows().size(), 2);

    solver.add_edit_var(x);
    solver.begin_edit();
    solver.suggest_value(x, 2);

    BOOST_CHECK_EQUAL(solver.columns().size(), 3);
    BOOST_CHECK_EQUAL(solver.rows().size(), 3);

    solver.end_edit();

    BOOST_CHECK_EQUAL(x.value(), 2.0);
    BOOST_CHECK_EQUAL(solver.columns().size(), 2);
    BOOST_CHECK_EQUAL(solver.rows().size(), 2);
}

BOOST_AUTO_TEST_CASE(editleak2_test)
{
    variable x(0), y(0);
    simplex_solver solver;
    solver.add_stay(x).add_stay(y);

    BOOST_CHECK_EQUAL(solver.columns().size(), 4);
    BOOST_CHECK_EQUAL(solver.rows().size(), 3);

    solver.add_edit_var(x).add_edit_var(y);

    solver.begin_edit();
    solver.suggest_value(x, 2);
    solver.suggest_value(y, 4);

    BOOST_CHECK_EQUAL(solver.columns().size(), 6);
    BOOST_CHECK_EQUAL(solver.rows().size(), 5);

    solver.end_edit();

    BOOST_CHECK_EQUAL(x.value(), 2.0);
    BOOST_CHECK_EQUAL(y.value(), 4.0);
    BOOST_CHECK_EQUAL(solver.columns().size(), 4);
    BOOST_CHECK_EQUAL(solver.rows().size(), 3);
}

BOOST_AUTO_TEST_CASE(delete1_test)
{
    variable x(0);
    simplex_solver solver;

    constraint init(x == 100, strength::weak());
    solver.add_constraint(init);
    BOOST_CHECK_EQUAL(x.value(), 100);

    constraint c10(x <= 10), c20(x <= 20);

    solver.add_constraint(c10).add_constraint(c20);

    BOOST_CHECK_EQUAL(x.value(), 10.0);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 20.0);
    solver.remove_constraint(c20);
    BOOST_CHECK_EQUAL(x.value(), 100.0);

    solver.add_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 10.0);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 100.0);

    solver.remove_constraint(init);

    BOOST_CHECK_EQUAL(solver.columns().size(), 0);
    BOOST_CHECK_EQUAL(solver.rows().size(), 1);
}

BOOST_AUTO_TEST_CASE(delete2_test)
{
    variable x(0), y(0);
    simplex_solver solver;

    solver.add_constraint(x == 100, strength::weak())
        .add_constraint(y == 120, strength::strong());

    BOOST_CHECK_EQUAL(x.value(), 100);
    BOOST_CHECK_EQUAL(y.value(), 120);

    constraint c10(x <= 10), c20(x <= 20);

    solver.add_constraint(c10).add_constraint(c20);

    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 20);

    constraint cxy(x * 2 == y);
    solver.add_constraint(cxy);

    BOOST_CHECK_EQUAL(x.value(), 20);
    BOOST_CHECK_EQUAL(y.value(), 40);

    solver.remove_constraint(c20);
    BOOST_CHECK_EQUAL(x.value(), 60);
    BOOST_CHECK_EQUAL(y.value(), 120);

    solver.remove_constraint(cxy);
    BOOST_CHECK_EQUAL(x.value(), 100);
    BOOST_CHECK_EQUAL(y.value(), 120);
}

BOOST_AUTO_TEST_CASE(delete3_test)
{
    variable x(0);
    simplex_solver solver;

    solver.add_constraint(x == 100, strength::weak());
    ;

    BOOST_CHECK_EQUAL(x.value(), 100);

    constraint c10(x <= 10), c10b(x <= 10);

    solver.add_constraint(c10).add_constraint(c10b);

    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10b);
    BOOST_CHECK_EQUAL(x.value(), 100);
}

BOOST_AUTO_TEST_CASE(casso1_test)
{
    variable x(0), y(0);
    simplex_solver solver;

    solver.add_constraint(x <= y)
        .add_constraint(y == x + 3)
        .add_constraint(x == 10.0, strength::weak())
        .add_constraint(y == 10.0, strength::weak());

    BOOST_CHECK((x.value() == 10 && y.value() == 13)
                || (x.value() == 7 && y.value() == 10));
}

BOOST_AUTO_TEST_CASE(casso2_test)
{
    variable x(0), y(0);
    simplex_solver solver;

    solver.add_constraints({x <= y, y == x + 3, x == 10});
    BOOST_CHECK_EQUAL(x.value(), 10);
    BOOST_CHECK_EQUAL(y.value(), 13);
}

BOOST_AUTO_TEST_CASE(inconsistent1_test)
{
    variable x(0);
    simplex_solver solver;

    solver.add_constraint(x == 10);

    BOOST_CHECK_THROW(solver.add_constraint(x == 5), required_failure);
}

BOOST_AUTO_TEST_CASE(inconsistent2_test)
{
    variable x(0);
    simplex_solver solver;

    BOOST_CHECK_THROW(solver.add_constraints({x >= 10, x <= 5}),
                      required_failure);
}

BOOST_AUTO_TEST_CASE(inconsistent3_test)
{
    variable v(0), w(0), x(0), y(0);
    simplex_solver solver;

    solver.add_constraints({v >= 10, w >= v, x >= w, y >= x});
    BOOST_CHECK_THROW(solver.add_constraint(y <= 5), required_failure);
}

BOOST_AUTO_TEST_CASE(inconsistent4_test)
{
    variable v(0), w(0), x(0), y(0);
    simplex_solver solver;

    solver.set_explaining(true);
    solver.add_constraints({v >= 10, w >= v, x >= w, y >= x});

    try {
        solver.add_constraint(y <= 5);
    } catch (required_failure_with_explanation& e) {
        BOOST_CHECK_EQUAL(e.explanation().size(), 5);
    } catch (...) {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_CASE(multiedit1_test)
{
    variable x(3), y(-5), w(0), h(0);
    simplex_solver solver;

    solver.add_stay(x).add_stay(y).add_stay(w).add_stay(h);
    solver.add_edit_var(x).add_edit_var(y);
    {
        scoped_edit outer_edit(solver);

        solver.suggest_value(x, 10).suggest_value(y, 20);
        solver.resolve();

        BOOST_CHECK_EQUAL(x.value(), 10);
        BOOST_CHECK_EQUAL(y.value(), 20);
        BOOST_CHECK_EQUAL(w.value(), 0);
        BOOST_CHECK_EQUAL(h.value(), 0);

        solver.add_edit_var(w).add_edit_var(h);
        {
            scoped_edit inner_edit(solver);
            solver.suggest_value(w, 30).suggest_value(h, 40);
        }

        BOOST_CHECK_EQUAL(x.value(), 10);
        BOOST_CHECK_EQUAL(y.value(), 20);
        BOOST_CHECK_EQUAL(w.value(), 30);
        BOOST_CHECK_EQUAL(h.value(), 40);

        solver.suggest_value(x, 50).suggest_value(y, 60);
    }

    BOOST_CHECK_EQUAL(x.value(), 50);
    BOOST_CHECK_EQUAL(y.value(), 60);
    BOOST_CHECK_EQUAL(w.value(), 30);
    BOOST_CHECK_EQUAL(h.value(), 40);
}

BOOST_AUTO_TEST_CASE(multiedit2_test)
{
    variable x(3), y(0), w(0), h(0);
    simplex_solver solver;

    solver.add_stay(x).add_stay(y).add_stay(w).add_stay(h);
    solver.add_edit_var(x).add_edit_var(y);
    {
        scoped_edit outer_edit(solver);

        solver.suggest_value(x, 10).suggest_value(y, 20);
        solver.resolve();

        BOOST_CHECK_EQUAL(x.value(), 10);
        BOOST_CHECK_EQUAL(y.value(), 20);
        BOOST_CHECK_EQUAL(w.value(), 0);
        BOOST_CHECK_EQUAL(h.value(), 0);

        solver.add_edit_var(x).add_edit_var(y).add_edit_var(w).add_edit_var(h);

        {
            scoped_edit inner_edit(solver);
            solver.suggest_value(w, 30).suggest_value(h, 40);
        }

        BOOST_CHECK_EQUAL(x.value(), 10);
        BOOST_CHECK_EQUAL(y.value(), 20);
        BOOST_CHECK_EQUAL(w.value(), 30);
        BOOST_CHECK_EQUAL(h.value(), 40);

        solver.suggest_value(x, 50).suggest_value(y, 60);
    }

    BOOST_CHECK_EQUAL(x.value(), 50);
    BOOST_CHECK_EQUAL(y.value(), 60);
    BOOST_CHECK_EQUAL(w.value(), 30);
    BOOST_CHECK_EQUAL(h.value(), 40);
}

BOOST_AUTO_TEST_CASE(a_variable_can_be_made_editable_multiple_times_stack_like_test)
{
    variable x;
    simplex_solver solver;

    solver.add_stay(x);
    solver.add_edit_var(x);
    {
        solver.add_edit_var(x);
        solver.suggest_value(x, 10);
        solver.resolve();
        solver.remove_edit_var(x);
    }
    BOOST_CHECK_EQUAL(x.value(), 10);

    solver.suggest_value(x, 20);
    solver.resolve();
    BOOST_CHECK_EQUAL(x.value(), 20);

    {
        solver.add_edit_var(x);
        solver.suggest_value(x, 30);
        solver.resolve();
        solver.remove_edit_var(x);
    }
    BOOST_CHECK_EQUAL(x.value(), 30);
}

BOOST_AUTO_TEST_CASE(manually_adding_multiple_edit_constraints_for_same_variable_test)
{
    variable x;
    simplex_solver solver;

    constraint e1 = std::make_shared<edit_constraint>(x);
    constraint e2 = std::make_shared<edit_constraint>(x);

    solver.add_stay(x);

    solver.add_constraint(e1);
    solver.suggest_value(x, 1);
    solver.resolve();
    BOOST_CHECK_EQUAL(x.value(), 1);

    solver.add_constraint(e2);
    solver.suggest_value(x, 2);
    solver.resolve();
    BOOST_CHECK_EQUAL(x.value(), 2);

    solver.remove_constraint(e1);
    solver.suggest_value(x, 3);
    solver.resolve();
    BOOST_CHECK_EQUAL(x.value(), 3);

    solver.remove_constraint(e2);
    solver.add_constraint(e1);
    solver.add_constraint(e2);
    solver.suggest_value(x, 5);
    solver.resolve();
    BOOST_CHECK_EQUAL(x.value(), 5);

    solver.remove_constraint(e2);
    solver.suggest_value(x, 6);
    solver.resolve();
    BOOST_CHECK_EQUAL(x.value(), 6);

    solver.remove_constraint(e1);
    BOOST_CHECK(solver.is_valid());
}

BOOST_AUTO_TEST_CASE(bounds_test)
{
    variable x(1);
    simplex_solver solver;

    solver.add_stay(x).add_bounds(x, 0, 10);

    BOOST_CHECK_EQUAL(x.value(), 1);
    solver.add_edit_var(x);
    solver.begin_edit().suggest_value(x, 20).end_edit();
    BOOST_CHECK_EQUAL(x.value(), 10);
}

BOOST_AUTO_TEST_CASE(bug0_test)
{
    variable x(7), y(8), z(9);
    simplex_solver solver;

    solver.add_stay(x).add_stay(y).add_stay(z);

    solver.add_edit_var(x).add_edit_var(y).add_edit_var(z);
    solver.begin_edit();

    solver.suggest_value(x, 1);
    solver.suggest_value(z, 2);

    solver.remove_edit_var(y);

    solver.suggest_value(x, 3);
    solver.suggest_value(z, 4);

    solver.end_edit();
}

BOOST_AUTO_TEST_CASE(quad_test)
{
    std::vector<point> c{{50, 50}, {50, 250}, {250, 250}, {250, 50}};
    std::vector<point> m(4);
    simplex_solver solver;

    double factor(1);
    for (point& corner : c) {
        solver.add_stay(corner.x, strength::weak(), factor);
        solver.add_stay(corner.y, strength::weak(), factor);
        factor *= 2;
    }

    // Midpoint constraints
    for (int i(0); i < 4; ++i) {
        int j((i + 1) % 4);
        solver.add_constraint(m[i].x == (c[i].x + c[j].x) / 2)
            .add_constraint(m[i].y == (c[i].y + c[j].y) / 2);
    }

    // Don't turn inside out
    typedef std::vector<std::pair<int, int>> pairs;
    for (auto a : pairs{{0, 2}, {0, 3}, {1, 2}, {1, 3}})
        solver.add_constraint(c[a.first].x + 1 <= c[a.second].x);

    for (auto a : pairs{{0, 1}, {0, 2}, {3, 1}, {3, 2}})
        solver.add_constraint(c[a.first].y + 1 <= c[a.second].y);

    // Limits
    for (auto& corner : c) {
        solver.add_bounds(corner.x, 0, 300);
        solver.add_bounds(corner.y, 0, 300);
    }

    // Now for the actual tests
    BOOST_CHECK_EQUAL(c[0], std::make_pair(50, 50));
    BOOST_CHECK_EQUAL(m[0], std::make_pair(50, 150));
    BOOST_CHECK_EQUAL(c[1], std::make_pair(50, 250));
    BOOST_CHECK_EQUAL(m[1], std::make_pair(150, 250));
    BOOST_CHECK_EQUAL(c[2], std::make_pair(250, 250));
    BOOST_CHECK_EQUAL(m[2], std::make_pair(250, 150));

    // Move one of the corners
    solver.suggest({{c[0].x, 100}});

    BOOST_CHECK_EQUAL(c[0], std::make_pair(100, 50));
    BOOST_CHECK_EQUAL(m[0], std::make_pair(75, 150));
    BOOST_CHECK_EQUAL(c[1], std::make_pair(50, 250));
    BOOST_CHECK_EQUAL(m[1], std::make_pair(150, 250));
    BOOST_CHECK_EQUAL(c[3], std::make_pair(250, 50));
    BOOST_CHECK_EQUAL(m[3], std::make_pair(175, 50));

    // Move one of the midpoints
    solver.suggest({{m[0].x, 50}, {m[0].y, 150}});

    BOOST_CHECK_EQUAL(m[0], std::make_pair(50, 150));
    BOOST_CHECK_EQUAL(c[0], std::make_pair(50, 50));
    BOOST_CHECK_EQUAL(m[0], std::make_pair(50, 150));
    BOOST_CHECK_EQUAL(m[3], std::make_pair(150, 50));
}

BOOST_AUTO_TEST_CASE(required_strength) // issue 18
{
    variable v(0);
    simplex_solver solver;

    solver.add_stay(v, strength::strong());

    BOOST_CHECK_EQUAL(v.value(), 0);

    solver.add_edit_var(v, strength::required());
    solver.begin_edit();
    solver.suggest_value(v, 2);
    solver.end_edit();

    BOOST_CHECK_EQUAL(v.value(), 2);
}

BOOST_AUTO_TEST_CASE(required_strength2)
{
    variable v(0);
    simplex_solver solver;

    solver.add_stay(v, strength::required());
    solver.resolve();

    BOOST_CHECK_EQUAL(v.value(), 0);

    solver.add_edit_var(v, strength::strong());
    solver.begin_edit();
    solver.suggest_value(v, 2);
    solver.end_edit();

    BOOST_CHECK_EQUAL(v.value(), 0);
}

BOOST_AUTO_TEST_CASE(bug_16)
{
    variable a(1), b(2);
    simplex_solver solver;

    solver.set_autosolve(false);
    solver.add_stay(a);
    BOOST_CHECK(solver.is_valid());

    solver.add_constraints({a == b});

    BOOST_CHECK(solver.is_valid());
    solver.add_edit_var(a);

    solver.begin_edit();
    solver.suggest_value(a, 3);
    solver.end_edit();

    BOOST_CHECK(solver.is_valid());

    BOOST_CHECK_EQUAL(a.value(), 3);
    BOOST_CHECK_EQUAL(b.value(), 3);
}

BOOST_AUTO_TEST_CASE(bug_16b)
{
    simplex_solver solver;
    variable a, b, c;

    solver.set_autosolve(false);

    solver.add_stays({a, c});

    solver.add_constraints({a == 10, b == c});

    solver.suggest({{c, 100}});

    BOOST_CHECK_EQUAL(a.value(), 10);
    BOOST_CHECK_EQUAL(b.value(), 100);
    BOOST_CHECK_EQUAL(c.value(), 100);

    solver.suggest({{c, 90}});

    BOOST_CHECK_EQUAL(a.value(), 10);
    BOOST_CHECK_EQUAL(b.value(), 90);
    BOOST_CHECK_EQUAL(c.value(), 90);
}

BOOST_AUTO_TEST_CASE(nonlinear) // issue 26
{
    variable x, y;
    simplex_solver solver;

    BOOST_CHECK_THROW(solver.add_constraint(x == 5 / y), nonlinear_expression);
    BOOST_CHECK_THROW(solver.add_constraint(x == y * y), nonlinear_expression);
}

BOOST_AUTO_TEST_CASE(change_strength_test) // issue 33
{
    variable x;
    simplex_solver solver;

    constraint c1{x == 1, strength::weak()};
    constraint c2{x == 2, strength::medium()};
    solver.add_constraints({c1, c2});
    BOOST_CHECK_EQUAL(x.value(), 2);

    solver.change_strength(c1, strength::strong());
    BOOST_CHECK_EQUAL(x.value(), 1);
}

BOOST_AUTO_TEST_CASE(change_weight_test) // issue 33
{
    variable x;
    simplex_solver solver;

    constraint c1{x == 1, strength::strong(), 1};
    constraint c2{x == 2, strength::strong(), 2};
    solver.add_constraints({c1, c2});
    BOOST_CHECK_EQUAL(x.value(), 2);

    solver.change_weight(c1, 3);
    BOOST_CHECK_EQUAL(x.value(), 1);
}

BOOST_AUTO_TEST_CASE(edit_unconstrained_variable)
{
    auto v = variable{ 0 };
    auto solver = simplex_solver{};

    solver.add_edit_var(v);
    BOOST_CHECK_EQUAL(v.value(), 0);
    BOOST_CHECK(solver.is_valid());

    solver.suggest_value(v, 2);
    solver.resolve();
    BOOST_CHECK_EQUAL(v.value(), 2);
    BOOST_CHECK(solver.is_valid());
}

BOOST_AUTO_TEST_CASE(add_constraints_after_marking_edit_variable)
{
    auto v = variable{ 0 };
    auto solver = simplex_solver{};

    solver.add_edit_var(v);
    solver.suggest_value(v, 2);
    solver.resolve();
    BOOST_CHECK_EQUAL(v.value(), 2);
    BOOST_CHECK(solver.is_valid());

    // Constraint overrides users desire
    auto fixed = constraint { v == 42 };
    solver.add_constraint(fixed);
    BOOST_CHECK_EQUAL(v.value(), 42);
    solver.suggest_value(v, 3);
    solver.resolve();
    BOOST_CHECK_EQUAL(v.value(), 42);
    BOOST_CHECK(solver.is_valid());

    // Goes back to last edited value
    solver.remove_constraint(fixed);
    BOOST_CHECK_EQUAL(v.value(), 3);
    BOOST_CHECK(solver.is_valid());
}

BOOST_AUTO_TEST_CASE(contains_constraint)
{
    auto c = constraint{ variable{} == 42 };
    auto solver = simplex_solver{};

    BOOST_CHECK(!solver.contains_constraint(c));

    solver.add_constraint(c);
    BOOST_CHECK(solver.contains_constraint(c));

    solver.remove_constraint(c);
    BOOST_CHECK(!solver.contains_constraint(c));
}

BOOST_AUTO_TEST_CASE(independent_values_can_be_suggested_for_concurrent_edits)
{
    simplex_solver s;
    variable v;
    constraint e1 = std::make_shared<edit_constraint>(v);
    constraint e2 = std::make_shared<edit_constraint>(v, strength::medium());

    s.add_constraint(e1);
    s.add_constraint(e2);

    s.suggest_value(e1, 42);
    s.suggest_value(e2, 21);
    s.resolve();
    BOOST_CHECK_EQUAL(v.value(), 42);

    // Other edit becomes visible after highest priority one is
    // removed from the solver
    s.remove_constraint(e1);
    BOOST_CHECK_EQUAL(v.value(), 21);

    // Edits are only remembered while the constraint is active
    s.add_constraint(e1);
    BOOST_CHECK_EQUAL(v.value(), 21);

    // Multiple edits respect strength changes
    s.suggest_value(e1, 50);
    s.resolve();
    BOOST_CHECK_EQUAL(v.value(), 50);
    s.change_strength(e1, strength::weak());
    BOOST_CHECK_EQUAL(v.value(), 21);
}
