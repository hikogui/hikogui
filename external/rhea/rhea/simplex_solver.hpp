//---------------------------------------------------------------------------
/// \file   simplex_solver.hpp
/// \brief  Implementation of a solver using a simplex algorithm
//
// Copyright 2012-2015, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <functional>
#include <list>
#include <stack>
#include <vector>

#include "edit_constraint.hpp"
#include "linear_expression.hpp"
#include "linear_inequality.hpp"
#include "solver.hpp"
#include "stay_constraint.hpp"
#include "tableau.hpp"
#include "objective_variable.hpp"

namespace rhea
{

/** Solver that implements the Cassowary incremental simplex algorithm. */
class simplex_solver : public solver, public tableau
{
public:
    typedef std::function<void(simplex_solver&)> event_cb;
    typedef std::function<void(const variable&, simplex_solver&)> variable_cb;

    /** Gets called whenever the tableau is resolved. */
    event_cb on_resolve;
    /** Gets called whenever a variable has changed. */
    variable_cb on_variable_change;

public:
    /** This struct is used as a parameter for the suggest() function. */
    struct suggestion
    {
        const variable& v;
        double suggested_value;
    };

public:
    simplex_solver();

    virtual ~simplex_solver() {}

    /** Add an edit constraint for a given variable.
     * The application should call this for every variable it is planning
     * to suggest a new value for, before calling begin_edit(). */
    simplex_solver& add_edit_var(const variable& v,
                                 const strength& s = strength::strong(),
                                 double weight = 1.0)
    {
        add_constraint(std::make_shared<edit_constraint>(v, s, weight));
        return *this;
    }

    /** Begin suggesting new values for edit variables.
     * The application should call add_edit_var() first for every variable
     * it is planning to call suggest_value() for. In most cases, it is
     * more convenient to use suggest() instead. */
    simplex_solver& begin_edit();

    /** We're done with the edit variables, resolve the constraints. */
    simplex_solver& end_edit();

    simplex_solver& remove_edit_var(const variable& v);

    simplex_solver& remove_edit_vars_to(size_t n);

    simplex_solver& remove_all_edit_vars() { return remove_edit_vars_to(0); }

    void resolve();

    /** Suggest a new value for an edit variable.
     *  The variable needs to be added as an edit variable,
     *  and begin_edit() needs to be called first.
     *  The tableau will not be solved completely until
     *  after resolve() or end_edit() has been called. */
    simplex_solver& suggest_value(const variable& v, double x);

    /** Suggest a new value for an edit constraint.
     *  The constraint needs to be an edit constraint and needs to
     *  have been added before.  The tableau will not be solved
     *  completely until resolve() or end_edit() has been called. */
    simplex_solver& suggest_value(const constraint& v, double x);

    /** Suggest a new value for a variables.
     *  This function calls add_edit_variable(), begin_edit(), and
     *  end_edit() as well.
     * \code
     solver.suggest(width, 200);
     * \endcode */
    simplex_solver& suggest(const variable& v, double x);

    /** Suggest new values for a list of variables.
     *  This function calls add_edit_variable(), begin_edit(), and
     *  end_edit() as well.
     * \code
     solver.suggest({{ width, 200 }, { height, 150 }});
     * \endcode */
    simplex_solver& suggest(const std::list<suggestion>& suggestions);

    /** If autosolving has been turned off, client code needs to explicitly
     ** call this function before accessing variables values. */
    simplex_solver& solve();

    /** Check if the solver knows of a given variable.
     * \param v The variable to check for
     * \return True iff v is a column in the tableau or a basic variable */
    bool contains_variable(const variable& v)
    {
        return columns_has_key(v) || is_basic_var(v);
    }

    /** Check if the solver knows of a given constraint.
     * \param c The constraint to check for
     * \return True iff c has been added to the solver */
    bool contains_constraint(const constraint& c)
    {
        return marker_vars_.find(c) != marker_vars_.end();
    }

    /** Check if this constraint was satisfied. */
    bool is_constraint_satisfied(const constraint& c) const;

    /** Reset all external variables to their current values.
     * Note: this triggers all callbacks, which might be used to copy the
     * variable's value to another variable. */
    void update_external_variables() { set_external_variables(); }

    void change_strength_and_weight(constraint c, const strength& s,
                                    double weight);
    void change_strength(constraint c, const strength& s);
    void change_weight(constraint c, double weight);

    /** Reset all stay constraint constants.
     * Each of the non-required stays will be represented by the equation
     * \f$v = v' + e_{plus} - e_{minus}\f$, where \f$v\f$ is the variable
     * associated with the stay, \f$v'\f$ is the previous value of
     * \f$v\f$, and \f$e_{plus}\f$ and \f$e_{minus}\f$ are slack variables
     * that hold the error for satisfying the constraint.
     *
     * We are about to change something, and we want to fix the constants
     * in the equations representing the stays.  If both \f$e_{plus}\f$
     * and \f$e_{minus}\f$ are nonbasic, they are zero in the current
     * solution, meaning the previous stay was exactly satisfied.  In this
     * case nothing needs to be changed.  Otherwise one of them is basic,
     * and the other must occur only in the expression for that basic error
     * variable.  In that case, the constant in the expression is set to
     * zero. */
    void reset_stay_constants();

    simplex_solver& set_auto_reset_stay_constants(bool f = true)
    {
        auto_reset_stay_constants_ = f;
        if (f)
            reset_stay_constants();

        return *this;
    }

    bool is_auto_reset_stay_constants() const
    {
        return auto_reset_stay_constants_;
    }

    void set_explaining(bool flag) { explain_failure_ = flag; }

    bool is_explaining() const { return explain_failure_; }

protected:
    solver& add_constraint_(const constraint& c);
    solver& remove_constraint_(const constraint& c);

    /** This is a privately-used struct that bundles a constraint, its
     ** positive and negative error variables, and its prior edit constant.
     */
    struct edit_info
    {
        edit_info(const variable& v_, constraint c_, variable plus_,
                  variable minus_, double prev_constant_)
            : v{v_}
            , c{c_}
            , plus{plus_}
            , minus{minus_}
            , prev_constant{prev_constant_}
        {
        }

        bool operator==(const variable& comp) const { return v.is(comp); }
        bool operator==(const constraint& comp) const { return c == comp; }

        variable v;
        constraint c;
        variable plus;
        variable minus;
        double prev_constant;
    };

    /** Bundles an expression, a plus and minus slack variable, and a
     ** prior edit constant.
     *  This struct is only used as a return variable of make_epression().*/
    struct expression_result
    {
        linear_expression expr;
        variable minus;
        variable plus;
        double previous_constant;

        expression_result()
            : minus{variable::nil_var()}
            , plus{variable::nil_var()}
            , previous_constant{0.0}
        {
        }
    };

    /** Make a new linear expression representing the constraint c,
     ** replacing any basic variables with their defining expressions.
     * Normalize if necessary so that the constant is non-negative.  If
     * the constraint is non-required, give its error variables an
     * appropriate weight in the objective function. */
    expression_result make_expression(const constraint& c);

    /** Add the constraint \f$expr = 0\f$ to the inequality tableau using
     ** an artificial variable.
     * To do this, create an artificial variable \f$a_0\f$, and add the
     * expression \f$a_0 = expr\f$ to the inequality tableau.
     * Then we try to solve for \f$a_0 = 0\f$, the return value indicates
     * whether this has succeeded or not.
     * @return True iff the expression could be added.
     *         False and a list of the constraints involved if not */
    std::pair<bool, constraint_list>
    add_with_artificial_variable(linear_expression& expr);

    /** Add the constraint \f$expr = 0\f$ to the inequality tableau.
     * @return True iff the expression could be added */
    bool try_adding_directly(linear_expression& expr);

    /** Try to choose a subject (that is, a variable to become basic) from
     ** among the current variables in \a expr.
     * If expr contains any unrestricted variables, then we must choose an
     * unrestricted variable as the subject.  Also, if the subject is new to
     * the solver, we won't have to do any substitutions, so we prefer new
     * variables to ones that are currently noted as parametric.
     *
     * If expr contains only restricted variables, if there is a restricted
     * variable with a negative coefficient that is new to the solver we can
     * make that the subject.  Otherwise we return nil, and have to add an
     * artificial variable and use that variable as the subject -- this is
     * done outside this method though.
     *
     * Note: in checking for variables that are new to the solver, we
     * ignore whether a variable occurs in the objective function, since
     * new slack variables are added to the objective function by
     * make_expression(), which is called before this method.
     *
     * \param expr  The expression that is being added to the solver
     * \return An appropriate subject, or nil */
    variable choose_subject(linear_expression& expr);

    void delta_edit_constant(double delta, const variable& v1,
                             const variable& v2);

    /** Optimize using the dual algorithm. */
    void dual_optimize();

    /** Minimize the value of an objective.
     * \pre The tableau is feasible.
     * \param z The objective to optimize for */
    void optimize(const variable& z);

    /** Perform a pivot operation.
     *  Move entry into the basis (i.e. make it a basic variable), and move
     *  exit out of the basis (i.e., make it a parametric variable).
     */
    void pivot(const variable& entry, const variable& exit);

    /** Set the external variables known to this solver to their appropriate
     ** values.
     * Set each external basic variable to its value, and set each
     * external parametric variable to zero.  Variables that are internal
     * to the solver don't actually store values &mdash; their
     * values are just implicit in the tableu &mdash; so we don't need to
     * set them. */
    void set_external_variables();

    void solve_();

    void change(variable& v, double n)
    {
        if (n != v.value()) {
            v.change_value(n);
            if (on_variable_change)
                on_variable_change(v, *this);
        }
    }

    constraint_list build_explanation(const variable& v,
                                      const linear_expression& expr) const;

private:
    typedef std::unordered_map<constraint, variable_set>
        constraint_to_varset_map;
    typedef std::unordered_map<constraint, variable> constraint_to_var_map;
    typedef std::unordered_map<variable, constraint> var_to_constraint_map;

    // The arrays of positive and negative error vars for the stay
    // constraints.  (We need to keep positive and negative separate,
    // since the error vars are always non-negative.)
    std::vector<variable> stay_minus_error_vars_;
    std::vector<variable> stay_plus_error_vars_;

    constraint_to_varset_map error_vars_;
    constraint_to_var_map marker_vars_;
    var_to_constraint_map constraints_marked_;

    variable objective_;

    // Map edit variables to their constraints, errors, and prior value.
    std::list<edit_info> edit_info_list_;

    bool auto_reset_stay_constants_;
    bool needs_solving_;
    bool explain_failure_;

    std::stack<size_t> cedcns_;
};

/** Scoped edit action.
 * This class calls begin_edit() on a simplex_solver upon construction,
 * and end_edit() as it goes out of scope.  This can be used as an
 * alternative to calling these two functions manually.
 *
 * \code

variable x(4), y(6);
simplex_solver solv;

solv.add_edit_variable(x).add_edit_variable(y);
{
scoped_edit user_input(solv);
solv.suggest_value(x, 2)
    .suggest_value(y, 7);
}
// 'user_input' goes out of scope here and calls solv.end_edit()
 * \endcode */
class scoped_edit
{
public:
    scoped_edit(simplex_solver& s)
        : s_(s.begin_edit())
    {
    }
    ~scoped_edit() { s_.end_edit(); }

private:
    simplex_solver& s_;
};

} // namespace rhea
