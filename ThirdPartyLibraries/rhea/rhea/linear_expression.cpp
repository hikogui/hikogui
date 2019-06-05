//---------------------------------------------------------------------------
// linear_expression.cpp
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#include "linear_expression.hpp"

#include <algorithm>
#include <cmath>

#include "approx.hpp"
#include "tableau.hpp"

namespace rhea
{

linear_expression::linear_expression(double constant)
    : constant_{constant}
{
}

linear_expression::linear_expression(const variable& v, double mul,
                                     double constant)
    : constant_{constant}
{
    terms_[v] = mul;
}

linear_expression& linear_expression::operator*=(double x)
{
    constant_ *= x;
    for (auto& p : terms_)
        p.second *= x;

    return *this;
}

linear_expression& linear_expression::operator/=(double x)
{
    constant_ /= x;
    for (auto& p : terms_)
        p.second /= x;

    return *this;
}

linear_expression& linear_expression::operator*=(const linear_expression& x)
{
    if (is_constant())
        return *this = x * constant();

    if (!x.is_constant())
        throw nonlinear_expression();

    return operator*=(x.constant());
}

linear_expression& linear_expression::operator/=(const linear_expression& x)
{
    if (!x.is_constant())
        throw nonlinear_expression();

    return operator/=(x.constant());
}

linear_expression& linear_expression::operator+=(const linear_expression& x)
{
    constant_ += x.constant_;
    for (auto& p : x.terms_)
        operator+=(p);

    return *this;
}

linear_expression& linear_expression::operator+=(const term& x)
{
    auto i = terms_.find(x.first);
    if (i == terms_.end()) {
        if (!near_zero(x.second))
            terms_[x.first] = x.second;
    } else if (near_zero(i->second += x.second)) {
        terms_.erase(i);
    }
    return *this;
}

linear_expression& linear_expression::operator-=(const linear_expression& x)
{
    constant_ -= x.constant_;
    for (auto& p : x.terms_)
        operator-=(p);

    return *this;
}

linear_expression& linear_expression::operator-=(const term& x)
{
    auto i = terms_.find(x.first);
    if (i == terms_.end()) {
        if (!near_zero(x.second))
            terms_[x.first] = -x.second;
    } else if (near_zero(i->second -= x.second)) {
        terms_.erase(i);
    }
    return *this;
}

linear_expression& linear_expression::add(const linear_expression& x,
                                          const variable& subject,
                                          tableau& solver)
{
    constant_ += x.constant_;
    for (const auto& p : x.terms_)
        add(p.first, p.second, subject, solver);

    return *this;
}

linear_expression& linear_expression::add(const variable& v, double c,
                                          const variable& subject,
                                          tableau& solver)
{
    auto i = terms_.find(v);
    if (i == terms_.end()) {
        if (!near_zero(c)) {
            terms_[v] = c;
            solver.note_added_variable(v, subject);
        }
    } else if (near_zero(i->second += c)) {
        terms_.erase(i);
        solver.note_removed_variable(v, subject);
    }

    return *this;
}

variable linear_expression::find_pivotable_variable() const
{
    assert(!is_constant());
    auto found = std::find_if(
        terms_.begin(), terms_.end(),
        [&](const value_type& x) { return x.first.is_pivotable(); });

    return found == terms_.end() ? variable::nil_var() : found->first;
}

double linear_expression::evaluate() const
{
    double result = constant_;
    for (const term& p : terms_)
        result += p.first.value() * p.second;

    return result;
}

double linear_expression::new_subject(const variable& subj)
{
    auto i = terms_.find(subj);
    assert(i != terms_.end());
    double reciprocal(1.0 / i->second);
    terms_.erase(i);
    operator*=(-reciprocal);

    return reciprocal;
}

void linear_expression::change_subject(const variable& old_subj,
                                       const variable& new_subj)
{
    assert(!new_subj.is_nil());
    if (old_subj.is(new_subj))
        return;

    double tmp = new_subject(new_subj);
    terms_[old_subj] = tmp;
}

void linear_expression::substitute_out(const variable& var,
                                       const linear_expression& expr,
                                       const variable& subj, tableau& solver)
{
    auto it = terms_.find(var);
    if (it == terms_.end()) {
        throw std::runtime_error(
            "substitute variable is not part of the expression");
    }
    double multiplier = it->second;
    terms_.erase(it);

    if (near_zero(multiplier))
        return;

    increment_constant(multiplier * expr.constant());
    for (auto& p : expr.terms()) {
        const variable& v = p.first;
        double mc = multiplier * p.second;

        auto oc = terms_.find(v);
        if (oc != terms_.end()) {
            if (near_zero(oc->second += mc)) {
                solver.note_removed_variable(oc->first, subj);
                terms_.erase(oc);
            }
        } else {
            terms_[v] = mc;
            solver.note_added_variable(v, subj);
        }
    }
}

} // namespace rhea
