//---------------------------------------------------------------------------
/// \file   edit_or_stay_constraint.hpp
/// \brief  Base class for edit- and stay-constraints
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "abstract_constraint.hpp"
#include "linear_expression.hpp"

namespace rhea
{

/** Abstract constraint that can be related to a variable, used only as
 ** a base class for edit_constraint and stay_constraint. */
class edit_or_stay_constraint : public abstract_constraint
{
public:
    edit_or_stay_constraint(const variable& v,
                            strength s = strength::required(),
                            double weight = 1.0)
        : abstract_constraint{s, weight}
        , var_{v}
    {
    }

    virtual ~edit_or_stay_constraint() {}

    const variable& var() const { return var_; }

    linear_expression expression() const
    {
        return linear_expression(var_, -1, var_.value());
    }

protected:
    variable var_;
};

} // namespace rhea
