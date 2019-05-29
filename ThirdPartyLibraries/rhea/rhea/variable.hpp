//---------------------------------------------------------------------------
/// \file   variable.hpp
/// \brief  A variable as used in an expression
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "approx.hpp"
#include "abstract_variable.hpp"
#include "float_variable.hpp"
#include "link_variable.hpp"
#include "action_variable.hpp"

namespace rhea
{

/** This tag is used in \a variable to link to external variables. */
struct linked
{
};

/** A variable as used in an expression.
 * Variables don't use the normal C++ copy semantics: objects are actually
 * counted references to an abstract_variable.  The following example
 * illustrates this:
 *
 * \code

variable x(1), y(0);

y = x;
// y is now 1

x.set_value(2);
// both x and y are now 2

 * \endcode
 *
 * Also note that a variable is nullable.  A variable that has been
 * constructed without a type cannot be used in expressions.
 *
 * Another caveat: "x == y" is not a boolean, but a linear_equality that
 * can be evaluated and used in constraints.  There are two ways to compare
 * two variables, depending on whether you want to test for equality or
 * equivalence:
 *
 * \code

variable x(2), y(x), z(2);

x.is(y); // True: y was constructed from x
x.is(z); // False: x and z both have the value 2, but they are different
variables

x.value() == y.value(); // True
x.value() == z.value(); // Also true

 * \endcode
 * */
class variable
{
public:
    variable()
        : p_{std::make_shared<float_variable>(0.0)}
    {
    }

    /** An explicit nil variable.
     *  This function only serves to make code more readable. */
    static variable nil_var() { return {nil_()}; }

    /** Wrap an abstract variable on the heap.
     * \param p  Shared pointer to a variable.
     */
    template <typename T>
    variable(std::shared_ptr<T>&& p)
        : p_{std::move(p)}
    {
    }

    /** "Copy" a variable.
     *  The resulting variable won't be a true copy, but rather another
     *  counted reference to the same variable. */
    variable(const variable& copy)
        : p_{copy.p_}
    {
    }

    /** Move constructor. */
    variable(variable&& copy)
        : p_{std::move(copy.p_)}
    {
    }

    /** Create a new floating pointe variable.
     * \param value  The variable's initial value
     */
    variable(int value)
        : p_{std::make_shared<float_variable>(value)}
    {
    }

    /** Create a new floating point variable.
     * \param value  The variable's initial value
     */
    variable(unsigned int value)
        : p_{std::make_shared<float_variable>(value)}
    {
    }

    /** Create a new floating point variable.
     * \param value  The variable's initial value
     */
    variable(float value)
        : p_{std::make_shared<float_variable>(value)}
    {
    }

    /** Create a new floating point variable.
     * \param value  The variable's initial value
     */
    variable(double value)
        : p_{std::make_shared<float_variable>(value)}
    {
    }

    /** Create variable that is linked to an existing integer.
     *  It is up to you to make sure the linked variable isn't destroyed
     *  while the solver is still using it.
     * \param value  This variable will be automatically updated
     */
    variable(int& value, const linked&)
        : p_{std::make_shared<link_int>(value)}
    {
    }

    /** Create variable that is linked to an existing float.
     *  It is up to you to make sure the linked variable isn't destroyed
     *  while the solver is still using it.
     * \param value  This variable will be automatically updated
     */
    variable(float& value, const linked&)
        : p_{std::make_shared<link_variable<float>>(value)}
    {
    }

    /** Create variable that is linked to an existing double.
     *  It is up to you to make sure the linked variable isn't destroyed
     *  while the solver is still using it.
     * \param value  This variable will be automatically updated
     */
    variable(double& value, const linked&)
        : p_{std::make_shared<link_variable<double>>(value)}
    {
    }

    /** Create a variable that calls a function whenever it is updated. */
    variable(std::function<void(double)> callback, double init_val = 0.0)
        : p_{std::make_shared<action_variable>(callback, init_val)}
    {
    }

    variable& operator=(const variable& assign)
    {
        p_ = assign.p_;
        return *this;
    }

    variable& operator=(variable&& move)
    {
        p_ = std::move(move.p_);
        return *this;
    }

    /** Check if this variable is of the type float_variable. */
    bool is_float() const { return p_->is_float(); }

    /** Check if this variable is used in the finite domain subsolver. */
    bool is_fd() const { return p_->is_fd(); }

    /** Check if this variable is a dummy variable. */
    bool is_dummy() const { return p_->is_dummy(); }

    /** Check if this variable is used outside the solver. */
    bool is_external() const { return p_->is_external(); }

    /** Check if this variable can be used as a pivot element in a tableau. */
    bool is_pivotable() const { return p_->is_pivotable(); }

    /** Check if this variable is restricted, or in other words, if it is
     ** a dummy or a slack variable. */
    bool is_restricted() const { return p_->is_restricted(); }

    /** Get the value of this variable. */
    double value() const
    {
        assert(!is_nil());
        return p_->value();
    }

    /** Get the value of this variable, converted to an integer. */
    int int_value() const { return p_->int_value(); }

    /** Set this variable to a new value. */
    void set_value(double x) { p_->set_value(x); }

    /** Change this variable's value. */
    void change_value(double x) { p_->change_value(x); }

    /** Check if this is a nil variable. */
    bool is_nil() const { return p_ == nullptr; }

    /** Calculate a hash value.
     *  This function is only used for placing variables in hash tables. */
    size_t hash() const { return id(); }

    /** Get a string representation.
     *  For ordinary variables, this will be the value.  Special variables
     *  will print 'dummy', 'slack', or 'edit'. */
    std::string to_string() const
    {
        return is_nil() ? "NIL" : p_->to_string();
    }

    /** Check if two variables refer to the same abstract_variable.
     *  This will not return 'true' for two distinct variables that happen
     *  to have the same value.  Example:
     * \code
     variable x(3), y(3), z;
     x.is(y); // False!
     z = x;  // z now refers to x
     z.set_value(5);
     x.is(z); // True (x.value() == 5 as well)
     * \endcode
     */
    bool is(const variable& x) const { return p_ == x.p_; }

    /** Helper function so rhea::variable can be used in a std::map */
    bool is_less(const variable& x) const { return id() < x.id(); }

    /** Get the variable's unique ID.
     *  Do not use: this function may disappear in future versions. */
    size_t id() const { return p_->id(); }

private:
    struct nil_
    {
    };

    variable(const nil_&) {}

private:
    /** Reference counted pointer to the actual variable. */
    std::shared_ptr<abstract_variable> p_;
};

/** Convenience typedef for sets of variables. */
typedef std::unordered_set<variable> variable_set;

} // namespace rhea

//-------------------------------------------------------------------------

namespace std
{

/** Hash function, required for std::unordered_map and -set. */
template <>
struct hash<rhea::variable>
{
    size_t operator()(const rhea::variable& v) const { return v.hash(); }
};

/** Equality test, required for std::unordered_map and -set. */
template <>
struct equal_to<rhea::variable>
{
    bool operator()(const rhea::variable& a, const rhea::variable& b) const
    {
        return a.is(b);
    }
};

/** Strict weak ordering, required for std::map and set. */
template <>
struct less<rhea::variable>
{
    bool operator()(const rhea::variable& a, const rhea::variable& b) const
    {
        return a.is_less(b);
    }
};

/** Get a string representation of a variable. */
inline string to_string(const rhea::variable& v)
{
    return v.to_string();
}

} // namespace std
