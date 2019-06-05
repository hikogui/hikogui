//---------------------------------------------------------------------------
/// \file   abstract_variable.hpp
/// \brief  Base class for variables
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <string>
#include "errors.hpp"

namespace rhea
{

/** Base class for variables. */
class abstract_variable
{
public:
    abstract_variable()
        : id_{++count_}
    {
    }

    virtual ~abstract_variable() {}

    size_t id() const { return id_; }

    /** Return true if this is a floating point variable.
     * \sa float_variable */
    virtual bool is_float() const { return false; }

    /** Return true if this is a variable in a finite domain. */
    virtual bool is_fd() const { return false; }

    /** Return true if this a dummy variable.
     * Dummies are used as a marker variable for required equality
     * constraints.  Such variables aren't allowed to enter the basis
     *  when pivoting. \sa dummy_variable */
    virtual bool is_dummy() const { return false; }

    /** Return true if this a variable known outside the solver. */
    virtual bool is_external() const { return false; }

    /** Return true if we can pivot on this variable.
     * \sa simplex_solver::pivot() */
    virtual bool is_pivotable() const
    {
        throw too_difficult("variable not usable inside simplex_solver");
    }

    /** Return true if this is a restricted (or slack) variable.
     * Such variables are constrained to be non-negative and occur only
     * internally to the simplex solver.
     * \sa slack_variable */
    virtual bool is_restricted() const
    {
        throw too_difficult("variable not usable inside simplex_solver");
    }

    /** Get the value of this variable. */
    virtual double value() const { return 0.0; }

    /** Get the value of this variable as an integer */
    virtual int int_value() const { return 0; }

    // LCOV_EXCL_START
    virtual void set_value(double) { assert(false); }

    virtual void change_value(double) { assert(false); }
    // LCOV_EXCL_STOP

    /** Get the value as a string. */
    virtual std::string to_string() const { return "abstract"; }

private:
    // Not happy with this, but it appears the algorithm needs this to run
    // with the autosolver turned off.  (Expression terms need a stable
    // iteration order, see also Github issue #16.)
    static size_t count_;
    size_t id_;
};

} // namespace rhea
