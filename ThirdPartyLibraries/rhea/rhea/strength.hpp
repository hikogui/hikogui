//---------------------------------------------------------------------------
/// \file   strength.hpp
/// \brief  The strength of a constraint
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <limits>
#include <string>
#include "symbolic_weight.hpp"

namespace rhea
{

/** Every constraint has a strength that determines where it sits in the
 ** hierarchy; strong constraints are always satisfied in preference
 ** to weaker ones.
 *
 * Since we want to be able to express preferences as well as requirements
 * in the constraint system, we need a specification for how conflicting
 * preferences are to be traded off.  In a constraint hierarchy each
 * constraint has a strength.  The 'required' strength is special, in that
 * required constraints must be satisfied.  The other strengths
 * all label non-required constraints.  A constraint of a given strength
 * completely dominates any constraint with a weaker strength.  In the
 * hierarchy theory, a comparator is used to compare different possible
 * solutions to the constraints and select among them.
 *
 * Within this framework a number of variations are possible.  One decision
 * is whether we only compare solutions on a constraint-by-constraint basis
 * (a \e local comparator), or whether we take some aggregate measure of the
 * unsatisfied constraints of a given strength (a \e global comparator).  A
 * second choice is whether we are concerned only whether a constraint is
 * satisfied or not (a \e predicate comparator), or whether we also want to
 * know how nearly satisfied it is (a \e metric comparator).
 *
 * Constraints whose domain is a metric space, for example the reals, can
 * have an associated error function.  The error in satisfying a constraint
 * is zero if and only if the constraint is satisfied, and becomes larger
 * the less nearly satisfied it is.  For a given collection of constraints,
 * Cassowary finds a \e locally-error-better or a \e weighted-sum-better
 * solution.
 */
class strength
{
public:
    /** */
    strength(const symbolic_weight& weight)
        : weight_{weight}
    {
    }

    /** Construct a strength from three weight factors.
     * \param a  'Strong' weight
     * \param b  'Medium weight
     * \param c  'Weak' weight */
    strength(double a, double b, double c)
        : weight_{a, b, c}
    {
    }

    /** Constraints with this strength must be satisfied.
     *  Used by default for constraints provided by the programmer. */
    static strength required()
    {
        const double z = std::numeric_limits<double>::max();
        return {z, z, z};
    }

    /** The default strength for edit constraints. */
    static strength strong() { return {1, 0, 0}; }

    /** Medium constraint strength. */
    static strength medium() { return {0, 1, 0}; }

    /** The default strength for stay constraints. */
    static strength weak() { return {0, 0, 1}; }

    /** Check if this strength signals a required constraint. */
    virtual bool is_required() const { return weight_ == required().weight_; }

    /** Get the 3-tuple symbolic weight. */
    const symbolic_weight& weight() const { return weight_; }

    bool operator==(const strength& c) const { return weight_ == c.weight_; }
    bool operator!=(const strength& c) const { return weight_ != c.weight_; }
    bool operator<=(const strength& c) const { return weight_ <= c.weight_; }
    bool operator<(const strength& c) const { return weight_ < c.weight_; }
    bool operator>=(const strength& c) const { return weight_ >= c.weight_; }
    bool operator>(const strength& c) const { return weight_ > c.weight_; }

private:
    symbolic_weight weight_;
};

} // namespace rhea
