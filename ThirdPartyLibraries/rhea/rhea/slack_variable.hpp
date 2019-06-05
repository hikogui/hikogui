//---------------------------------------------------------------------------
/// \file   slack_variable.hpp
/// \brief  Slack variable
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "abstract_variable.hpp"

namespace rhea
{

/** Slack variables are used to turn inequalities into equations.
 * For example, this inequality:
 * \f$
 * 3x + 5y \leq 10
 * \f$
 * becomes the equation:
 * \f$
 * 3x + 5y + s = 10
 * \f$ by introducing the slack variable \f$s\f$.
 */
class slack_variable : public abstract_variable
{
public:
    slack_variable()
        : abstract_variable{}
    {
    }
    ~slack_variable() {}

    virtual bool is_external() const { return false; }
    virtual bool is_pivotable() const { return true; }
    virtual bool is_restricted() const { return true; }

    std::string to_string() const { return "slack"; }
};

} // namespace rhea
