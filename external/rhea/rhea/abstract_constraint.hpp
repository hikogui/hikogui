//---------------------------------------------------------------------------
/// \file   abstract_constraint.hpp
/// \brief  Base class for constraints
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <memory>
#include <string>
#include "linear_expression.hpp"
#include "strength.hpp"
#include "variable.hpp"

namespace rhea
{

// Forward declaration
class solver;

/** Base class for constraints. */
class abstract_constraint
{
public:
    abstract_constraint(strength s = strength::required(), double weight = 1.0)
        : strength_{std::move(s)}
        , weight_{weight}
    {
        if (weight_ == 0.0)
            throw std::runtime_error("constraint weight cannot be zero");
    }

    virtual ~abstract_constraint() {}

    virtual linear_expression expression() const = 0;

    /** Check if this is an edit_constraint. */
    virtual bool is_edit_constraint() const { return false; }

    /** Check if this is a linear_inequality. */
    virtual bool is_inequality() const { return false; }

    /** Check if this is a required constraint. */
    virtual bool is_required() const { return strength_.is_required(); }

    /** Check if this is a stay_constraint. */
    virtual bool is_stay_constraint() const { return false; }

    /** Get the strength of this constraint. */
    const strength& get_strength() const { return strength_; }

    /** Get the weight of this constraint. */
    virtual double weight() const { return weight_; }

public:
    /** Returns true iff this constraint is satisfied. */
    virtual bool is_satisfied() const = 0;

public:
    /** Change the strength.
     *  Note that Rhea does not allow changing the weight of a constraint
     *  that is already part of a solver. */
    void change_strength(const strength& new_strength)
    {
        strength_ = new_strength;
    }

    /** Change the weight.
     *  Note that Rhea does not allow changing the weight of a constraint
     *  that is already part of a solver. */
    void change_weight(double new_weight) { weight_ = new_weight; }

    /** Get the symbolic weight. */
    symbolic_weight get_symbolic_weight() const { return strength_.weight(); }

    /** Get the symbolic weight after adjusting it for weight. */
    double adjusted_symbolic_weight() const
    {
        return get_symbolic_weight().as_double() * weight();
    }

    /** Set a new strength without any checks.
     * \see change_strength */
    void set_strength(const strength& n) { strength_ = n; }

    /** Set a new weight without any checks.
     * \see change_weight */
    void set_weight(double n) { weight_ = n; }

protected:
    strength strength_;
    double weight_;
};

} // namespace rhea
