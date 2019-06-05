//---------------------------------------------------------------------------
/// \file   linear_inequality.hpp
/// \brief  A linear inequality constraint
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "errors.hpp"
#include "linear_constraint.hpp"
#include "relation.hpp"

namespace rhea
{

/** A constraint of the form \f$expr \geq 0\f$. */
class linear_inequality : public linear_constraint
{
public:
    linear_inequality()
        : linear_constraint{0.0, strength::required(), 1.0}
    {
    }

    linear_inequality(linear_expression expr,
                      strength s = strength::required(), double weight = 1.0)
        : linear_constraint{std::move(expr), s, weight}
    {
    }

    linear_inequality(const variable& v, relation op, linear_expression expr,
                      strength s = strength::required(), double weight = 1.0)
        : linear_constraint{std::move(expr), s, weight}
    {
        switch (op.type()) {
        case relation::geq:
            expr_ *= -1;
            expr_ += v;
            break;

        case relation::leq:
            expr_ -= v;
            break;

        default:
            throw edit_misuse(); // LCOV_EXCL_LINE
        };
    }

    linear_inequality(linear_expression lhs, relation op,
                      linear_expression rhs, strength s = strength::required(),
                      double weight = 1.0)
        : linear_constraint{std::move(rhs), s, weight}
    {
        switch (op.type()) {
        case relation::geq:
            expr_ *= -1.0;
            expr_ += lhs;
            break;

        case relation::leq:
            expr_ -= lhs;
            break;

        default:
            throw edit_misuse(); // LCOV_EXCL_LINE
        };
    }

    virtual ~linear_inequality() {}

    virtual bool is_inequality() const { return true; }

    virtual bool is_satisfied() const { return expr_.evaluate() >= 0; }
};

//-------------------------------------------------------------------------

inline linear_inequality operator<=(const linear_expression& lhs,
                                    const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality operator>=(const linear_expression& lhs,
                                    const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}

//-------------------------------------------------------------------------

inline linear_inequality operator<=(const variable& lhs,
                                    const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality operator>=(const variable& lhs,
                                    const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}

//-------------------------------------------------------------------------

inline linear_inequality operator<=(const variable& lhs, const variable& rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality operator>=(const variable& lhs, const variable& rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}

//-------------------------------------------------------------------------

inline linear_inequality operator<=(const variable& lhs, double rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality operator>=(const variable& lhs, double rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}

//-------------------------------------------------------------------------

inline linear_inequality operator<=(const variable& lhs, int rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality operator>=(const variable& lhs, int rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}

} // namespace rhea
