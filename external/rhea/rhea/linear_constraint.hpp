//---------------------------------------------------------------------------
/// \file   linear_constraint.hpp
/// \brief  Linear constraint
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "abstract_constraint.hpp"
#include "linear_expression.hpp"

namespace rhea
{

/** A constraint based on a linear expression.
 *  Used as a base class for linear_equation and linear_inequality. */
class linear_constraint : public abstract_constraint
{
public:
    linear_constraint(const linear_expression& expr = linear_expression(),
                      const strength& s = strength::required(),
                      double weight = 1.0)
        : abstract_constraint{s, weight}
        , expr_{expr}
    {
    }

    virtual ~linear_constraint() {}

    linear_expression expression() const { return expr_; }

    void change_constant(double c) { expr_.set_constant(c); }

protected:
    linear_expression expr_;
};

} // namespace rhea
