//---------------------------------------------------------------------------
/// \file   linear_expression.hpp
/// \brief  A linear expression
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "approx.hpp"
#include "variable.hpp"
#include <boost/container/flat_map.hpp>

namespace rhea
{

class tableau;

/** Linear expression.
 * Expressions have the form \f$av_0 + bv_1 + \ldots + c\f$, where \f$v_n\f$
 * is a variable, \f$a, b, \ldots{}\f$ are non-zero coefficients, and
 * \f$c\f$ is a constant.
 *
 * Expressions can be built from variables in a natural way:
 *
 * \code

variable x (3), y (5);
auto expr (x * 5 + y + 2);
expr.evaluate(); // Returns '22'

 * \endcode
 */
class linear_expression
{
public:
    // It would be nice to use an unordered_map here, but it appears
    // the algorithm is sensitive to the order in which the terms are
    // iterated. (Github issue #16.)
    typedef boost::container::flat_map<variable, double> terms_map;

    typedef terms_map::value_type value_type;
    typedef terms_map::value_type term;

public:
    linear_expression(double num = 0);

    linear_expression(const variable& clv, double value = 1,
                      double constant = 0);

    linear_expression& operator*=(double x);
    linear_expression& operator/=(double x);
    linear_expression& operator*=(const linear_expression& x);
    linear_expression& operator/=(const linear_expression& x);
    linear_expression& operator+=(const linear_expression& x);
    linear_expression& operator-=(const linear_expression& x);
    linear_expression& operator+=(const term& x);
    linear_expression& operator-=(const term& x);

    linear_expression& operator+=(const variable& x)
    {
        return operator+=(term(x, 1));
    }

    linear_expression& operator-=(const variable& x)
    {
        return operator+=(term(x, -1));
    }

    linear_expression& operator+=(double x)
    {
        constant_ += x;
        return *this;
    }

    linear_expression& operator-=(double x)
    {
        constant_ -= x;
        return *this;
    }

    linear_expression& set(const variable& v, double x)
    {
        if (!near_zero(x))
            terms_[v] = x;
        return *this;
    }

    /** Add \a expr to this expression.
     * Notifies the solver if a variable is added or deleted from this
     * expression. */
    linear_expression& add(const linear_expression& expr,
                           const variable& subject, tableau& solver);

    /** Add a term \f$c\cdot{}v\f$ to this expression.
     *  If the expression already contains a term involving v, it adds c
     *  to the existing coefficient. If the new coefficient is approximately
     *  zero, v is removed from the expression. The solver is notified if
     *  v is added or removed.
     * \param v     The variable to be added
     * \param c     The coefficient
     * \param subj  The subject to report back to the solver
     * \param solver The solver that will be informed of any added or
     *               removed variables */
    linear_expression& add(const variable& v, double c,
                           const variable& subject, tableau& solver);

    /** Erase a variable from the expression. */
    void erase(const variable& v) { terms_.erase(v); }

    /** Return a pivotable variable.
     * \pre is_constant() == false
     * \throws internal_error
     * \return A pivotable variable, or variable::nil() if no variable
     *         was found. */
    variable find_pivotable_variable() const;

    /** Replace \a var with a symbolic expression that is equal to it.
     * If a variable has been added to this expression that wasn't there
     * before, or if a variable has been dropped from this expression
     * because it now has a coefficient of 0, inform the solver.
     * \param v     The variable to be replaced
     * \param expr  The expression to replace it with
     * \param subj  The subject to report back to the solver
     * \param solver The solver that will be informed of any added or
     *               removed variables */
    void substitute_out(const variable& v, const linear_expression& expr,
                        const variable& subj, tableau& solver);

    /** This linear expression currently represents the equation
     ** oldSubject=self, destructively modify it so that it represents
     ** the equation NewSubject=self.
     * Suppose this expression is \f$av_{new} + bv_0 + \ldots + c\f$.
     * Then, if the current equation is:
     * \f[v_{old} = av_{new} + bv_0 + \ldots + c\f]
     * The new equation will become:
     * \f[v_{new} = -\frac{1}{a}v_{old} - \frac{b}{a}v_0 - \ldots -
     *\frac{c}{a}\f]
     *
     * \pre \a new_subj has a nonzero coefficient in this expression.
     */
    void change_subject(const variable& old_subj, const variable& new_subj);

    /** If this linear expression currently represents the equation
     ** \f$expr = 0\f$,destructively modify it so that
     ** \f$expr = subj\f$ represents an equivalent equation.
     *
     * Suppose this expression represents:
     * \f[av_{subject} + bv_0 + \ldots + xv_n + c = 0\f]
     * Then the modified expression will become:
     * \f[-\frac{b}{a}v_0 - \ldots - \frac{x}{a}v_n - \frac{c}{a}\f]
     *
     * \pre \a new_subj has a nonzero coefficient in this expression.
     * \param new_subj  The new subject variable
     * \return The reciprocal */
    double new_subject(const variable& new_subj);

    /** Evaluate the expression.
     * \return The result of the evaluation */
    double evaluate() const;

    /** Get the coefficient of one of the terms.
     * \param v  The variable to look for
     * \return The term's coefficient, or zero if \a v does not occur
     *         in this expression */
    double coefficient(const variable& v) const
    {
        auto i(terms_.find(v));
        return i == terms_.end() ? 0.0 : i->second;
    }

    /** Get the constant \f$c\f$ of the expression. */
    double constant() const { return constant_; }

    /** Set the constant \f$c\f$ to a new value. */
    void set_constant(double c) { constant_ = c; }

    /** Add a given value to the constant \f$c\f$. */
    void increment_constant(double c) { constant_ += c; }

    /** Get a map of all terms and their coefficients. */
    const terms_map& terms() const { return terms_; }

    /** Returns true iff this expression is constant. */
    bool is_constant() const { return terms_.empty(); }

private:
    /** The expression's constant term. */
    double constant_;
    /** A map of all variables and their coefficients. */
    terms_map terms_;
};

//--------------------------------------------------------------------------

inline linear_expression operator*(linear_expression e, double x)
{
    return e *= x;
}

inline linear_expression operator*(double x, linear_expression e)
{
    return e *= x;
}

inline linear_expression operator/(linear_expression e, double x)
{
    return e /= x;
}

inline linear_expression operator*(linear_expression e,
                                   const linear_expression& x)
{
    return e *= x;
}

inline linear_expression operator/(linear_expression e,
                                   const linear_expression& x)
{
    return e /= x;
}

inline linear_expression operator+(linear_expression e,
                                   const linear_expression& x)
{
    return e += x;
}

inline linear_expression operator-(linear_expression e,
                                   const linear_expression& x)
{
    return e -= x;
}

//--------------------------------------------------------------------------

inline linear_expression operator*(const variable& v, double x)
{
    return linear_expression(v, x);
}

inline linear_expression operator*(const variable& v, int x)
{
    return linear_expression(v, x);
}

inline linear_expression operator*(double x, const variable& v)
{
    return linear_expression(v, x);
}

inline linear_expression operator*(int x, const variable& v)
{
    return linear_expression(v, x);
}

inline linear_expression operator/(const variable& v, double x)
{
    return linear_expression(v, 1.0 / x);
}

inline linear_expression operator+(const variable& v, double x)
{
    return linear_expression(v, 1, x);
}

inline linear_expression operator+(const variable& v, int x)
{
    return linear_expression(v, 1, x);
}

inline linear_expression operator-(const variable& v, double x)
{
    return linear_expression(v, 1, -x);
}

inline linear_expression operator-(const variable& v, int x)
{
    return linear_expression(v, 1, -x);
}

inline linear_expression operator-(double x, const variable& v)
{
    return linear_expression(v, -1, x);
}

inline linear_expression operator-(int x, const variable& v)
{
    return linear_expression(v, -1, x);
}

inline linear_expression operator+(const variable& v, const variable& w)
{
    return linear_expression(v) += w;
}

inline linear_expression operator-(const variable& v, const variable& w)
{
    return linear_expression(v) -= w;
}

} // namespace rhea
