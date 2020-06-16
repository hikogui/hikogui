//---------------------------------------------------------------------------
/// \file   errors.hpp
/// \brief  Exception classes
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <string>
#include <stdexcept>

namespace rhea
{

class variable;

/** Base class for all Rhea exceptions. */
class error : public std::exception
{
public:
    virtual ~error() throw() {}

    // LCOV_EXCL_START
    virtual const char* what() const throw() { return "unspecified error"; }
    // LCOV_EXCL_STOP
};

/** Signals an internal inconsistency in the solver. */
class internal_error : public error
{
    std::string msg;

public:
    internal_error(std::string m)
        : msg(m)
    {
    }

    virtual ~internal_error() throw() {}

    virtual const char* what() const throw() { return msg.c_str(); }
};

/** Thrown whenever the usual ordering of setting up edit constraints is
 ** not observed.
 * The usual order is:
 * - simplex_solver::add_edit_var()
 * - simplex_solver::begin_edit()
 * - simplex_solver::suggest_value()
 * - simplex_solver::end_edit()
 *
 * This is done automatically by simplex_solver::suggest(). */
class edit_misuse : public error
{
    const variable* var_;

public:
    edit_misuse()
        : var_{nullptr}
    {
    }
    edit_misuse(const rhea::variable& v)
        : var_{&v}
    {
    }
    virtual ~edit_misuse() throw() {}

    virtual const char* what() const throw()
    {
        return "edit protocol usage violation";
    }

    const variable& var() const { return *var_; }
};

/** The constraints are too difficult to solve. */
class too_difficult : public error
{
    std::string msg;

public:
    too_difficult() {}

    too_difficult(std::string m)
        : msg(m)
    {
    }

    virtual ~too_difficult() throw() {}

    virtual const char* what() const throw()
    {
        return msg.empty() ? "the constraints are too difficult to solve"
                           : msg.c_str();
    }
};

/** Cyclic dependencies between constraints are not allowed. */
class cycle_not_allowed : public too_difficult
{
public:
    virtual ~cycle_not_allowed() throw() {}

    virtual const char* what() const throw()
    {
        return "a cyclic constraint graph is not permitted by the solver";
    }
};

/** One of the required constraints cannot be satisfied. */
class required_failure : public error
{
public:
    virtual ~required_failure() throw() {}

    virtual const char* what() const throw()
    {
        return "a required constraint cannot be satisfied";
    }
};

/** Not enough stay constraints were specified to give specific values
 ** to every variable. */
class not_enough_stays : public error
{
public:
    virtual ~not_enough_stays() throw() {}

    virtual const char* what() const throw()
    {
        return "there are not enough stays to give specific values to every "
               "variable";
    }
};

/** The resulting expression would be nonlinear.
 * This usually happens when multiplying two expressions that have the
 * same variable in them, resulting in a quadratic expression. */
class nonlinear_expression : public error
{
public:
    virtual ~nonlinear_expression() throw() {}

    virtual const char* what() const throw()
    {
        return "the resulting expression would be nonlinear";
    }
};

/** The application tried to remove a constraint that doesn't exist in
 ** the solver. */
class constraint_not_found : public error
{
public:
    virtual ~constraint_not_found() throw() {}

    virtual const char* what() const throw()
    {
        return "tried to remove a constraint that was never added";
    }
};

/** The application tried to remove a row that doesn't exist. */
class row_not_found : public error
{
public:
    virtual ~row_not_found() throw() {}

    virtual const char* what() const throw() { return "row does not exist"; }
};

} // namespace rhea
