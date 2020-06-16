//---------------------------------------------------------------------------
/// \file   stay_constraint.hpp
/// \brief  Stay constraint
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "edit_or_stay_constraint.hpp"

namespace rhea
{

/** Each variable that is to stay at an old value needs an explicit stay
 ** constraint.
 * These stay constraints need to be added before any other constraints,
 * since otherwise the variable's value is likely to be changed
 * inappropriately to satisfy the other constraints while initially building
 * the tableau.
 *
 * Stay constraints will be represented as equations of the form
 * \f$v = \alpha + \delta_v^{+} - \delta_v^{-}\f$, where
 * \f$\delta_v^{+}\f$ and \f$\delta_v^{-}\f$ are non-negative variables
 * representing the deviation of $v$ from the desired value $\alpha$.  If
 * the constraint is satisfied both \f$\delta_v^{+}\f$ and
 * \f$\delta_v^{-}\f$ will be 0. Otherwise, \f$\delta_v^{+}\f$ will be
 * positive and \f$\delta_v^{-}\f$ will be 0 if \f$v\f$ is too big,
 * or vice versa if \f$v\f$ is too small.
 * Since we want \f$\delta_v^{+}\f$ and \f$\delta_v^{-}\f$ to be 0 if
 * possible, we make them part of the objective function, with larger
 * coefficients for the error variables for stronger constraints.
 */
class stay_constraint : public edit_or_stay_constraint
{
public:
    stay_constraint(const variable& v, strength s = strength::weak(),
                    double weight = 1.0)
        : edit_or_stay_constraint{v, s, weight}
    {
    }

    virtual ~stay_constraint() {}

    virtual bool is_stay_constraint() const { return true; }

    virtual bool is_satisfied() const { return false; }
};

} // namespace rhea
