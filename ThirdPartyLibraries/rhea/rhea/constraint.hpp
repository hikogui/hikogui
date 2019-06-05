//---------------------------------------------------------------------------
/// \file   constraint.hpp
/// \brief  Base class for constraints
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <list>
#include <memory>
#include "abstract_constraint.hpp"
#include "linear_equation.hpp"
#include "linear_inequality.hpp"

namespace rhea
{

/** An equation or inequality involving one or more variables.
 * Constraints can be defined as "normal" C++ expressions:
 * \code

 variable x (1), y (2);

 constraint a (x + 4 <= y * 2);
 constraint b (x * 2 == y * 3);

 * \endcode
 */
class constraint
{
public:
    constraint() {}

    template <typename T>
    constraint(std::shared_ptr<T>&& p)
        : p_{std::move(p)}
    {
    }

    constraint(const linear_equation& eq)
        : p_{std::make_shared<linear_equation>(eq)}
    {
    }

    constraint(const linear_equation& eq, strength s, double weight = 1)
        : p_{std::make_shared<linear_equation>(eq.expression(), std::move(s),
                                               weight)}
    {
    }

    constraint(const linear_inequality& eq)
        : p_{std::make_shared<linear_inequality>(eq)}
    {
    }

    constraint(const linear_inequality& eq, strength s, double weight = 1)
        : p_{std::make_shared<linear_inequality>(eq.expression(), std::move(s),
                                                 weight)}
    {
    }

public:
    linear_expression expression() const { return p_->expression(); }

    bool is_edit_constraint() const { return p_->is_edit_constraint(); }

    bool is_inequality() const { return p_->is_inequality(); }

    bool is_required() const { return p_->is_required(); }

    bool is_stay_constraint() const { return p_->is_stay_constraint(); }

    const strength& get_strength() const { return p_->get_strength(); }

    double weight() const { return p_->weight(); }

    bool is_satisfied() const { return p_->is_satisfied(); }

    void change_strength(const strength& new_strength)
    {
        p_->change_strength(new_strength);
    }

    void change_weight(double new_weight) { p_->change_weight(new_weight); }

    symbolic_weight get_symbolic_weight() const
    {
        return p_->get_symbolic_weight();
    }

    double adjusted_symbolic_weight() const
    {
        return p_->adjusted_symbolic_weight();
    }

    void set_strength(const strength& n) { p_->set_strength(n); }

    void set_weight(double n) { p_->set_weight(n); }

    template <typename T>
    T& as()
    {
        return dynamic_cast<T&>(*p_);
    }

    template <typename T>
    const T& as() const
    {
        return dynamic_cast<const T&>(*p_);
    }

    bool is_nil() const { return !p_; }

public:
    constraint& operator=(const constraint& assign)
    {
        p_ = assign.p_;
        return *this;
    }

    virtual bool operator==(const constraint& other) const
    {
        return p_ == other.p_;
    }

    virtual bool operator!=(const constraint& other) const
    {
        return p_ != other.p_;
    }

    size_t hash() const
    {
        return std::hash<std::shared_ptr<abstract_constraint>>()(p_);
    }

private:
    std::shared_ptr<abstract_constraint> p_;
};

/** Convenience typedef for bundling constraints. */
typedef std::list<constraint> constraint_list;

} // namespace rhea

namespace std
{

/** Hash function, required for std::unordered_map. */
template <>
struct hash<rhea::constraint>
{
    size_t operator()(const rhea::constraint& c) const { return c.hash(); }
};

} // namespace std
