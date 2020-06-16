//---------------------------------------------------------------------------
/// \file   tableau.hpp
/// \brief  Tableau for holding and manipulating linear expressions
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <iostream>
#include "errors.hpp"
#include "variable.hpp"
#include "linear_expression.hpp"

namespace rhea
{

/** A tableau, or augmented matrix, represents the coefficients and
 ** solution of a set of equations.
 * For example, given the following set of equations:
 * \f[\begin{align}
 * a + 2b + 3c &= 0 \\
 * 3a + 4b + 7c &= 2 \\
 * 6a + 5b + 9c &= 11
 * \end{align}\f]
 * The tableau would be:
 * \f[\left[\begin{array}{ccc|c}
 * 1 & 2 & 3 & 0 \\
 * 3 & 4 & 7 & 2 \\
 * 6 & 5 & 9 & 11
 * \end{array}\right]\f]
 * So every column corresponds to a variable, and every row to a
 * linear equation.  If the first row is the objective, and the first
 * column the objective variable, we get a tableau of the form:
 * \f[\left[\begin{array}{cc|c}
 * 1 & -c^T & 0 \\
 * 0 & A & b
 * \end{array}\right]\f]
 * If \f$A\f$ contains an identity matrix, the tableau is in canonical
 * form.  The variables corresponding to the identity matrix are the
 * basic variables, the others are the free variables.  (Since it is an
 * identity matrix, every row is also associated with exactly one basic
 * variable.)
 * If the free variables are assumed to be zero, the solution can be read
 * from the first row.
 */
class tableau
{
public:
    typedef std::unordered_map<variable, variable_set> columns_map;
    typedef std::unordered_map<variable, linear_expression> rows_map;

public:
    /** This function should be invoked when v has been removed from an
     ** expression, so the column indices can be updated. */
    void note_removed_variable(const variable& v, const variable& subj);

    /** This function should be invoked when v has been added to an
     ** expression, so the column indices can be updated. */
    void note_added_variable(const variable& v, const variable& subj);

    /** Check the internal consistency of this data structure. */
    bool is_valid() const;

public:
    tableau() {}

    virtual ~tableau() {}

    /** Add a new row to the tableau. */
    void add_row(const variable& v, const linear_expression& e);

    /** Remove a variable from the tableau.
     * \return True iff the variable was known */
    bool remove_column(const variable& v);

    /** Remove a row from the tableau.
     * \param v  The basic variable that is used to index the row
     * \return The expression represented by the removed row */
    linear_expression remove_row(const variable& v);

    /** Replace all occurrences of \a old_var with \a expr, and update
     ** column cross indices.
     *  \a old_var should now be a basic variable.
     *  This function calls substitute_out on each row that has old_var
     *  in it.
     * @post old_var is no longer a basic variable */
    void substitute_out(const variable& old_var,
                        const linear_expression& expr);

    const columns_map& columns() const { return columns_; }

    const rows_map& rows() const { return rows_; }

    bool columns_has_key(const variable& subj) const
    {
        return columns_.count(subj) > 0;
    }

    /** Get the linear expression that the given row represents. */
    const linear_expression& row_expression(const variable& v) const
    {
        auto i = rows_.find(v);
        if (i == rows_.end())
            throw row_not_found();

        return i->second;
    }

    /** Get the linear expression that the given row represents. */
    linear_expression& row_expression(const variable& v)
    {
        auto i = rows_.find(v);
        if (i == rows_.end())
            throw row_not_found();

        return i->second;
    }

    /** Check if v is one of the basic variables. */
    bool is_basic_var(const variable& v) const { return rows_.count(v) > 0; }

    /** Check if f is one of the parametric (aka. free) variables. */
    bool is_parametric_var(const variable& v) const
    {
        return rows_.count(v) == 0;
    }

protected:
    /** A mapping from variables which occur in expressions to the
     ** rows whose expressions contain them. */
    columns_map columns_;

    /** A mapping from the basic variables to the expressions for that
     ** row in the tableau. */
    rows_map rows_;

    /** The collection of basic variables that have infeasible rows.
     *  This is used internally when optimizing. */
    variable_set infeasible_rows_;

    /** A map to quickly find rows with external basic variables. */
    variable_set external_rows_;

    /** A map to quickly find rows with external parametric variables. */
    variable_set external_parametric_vars_;
};

} // namespace rhea
