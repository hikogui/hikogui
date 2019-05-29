//---------------------------------------------------------------------------
/// \file   iostream.hpp
/// \brief  Standard Library iostream support for variables
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "float_variable.hpp"
#include "linear_expression.hpp"
#include "tableau.hpp"
#include "strength.hpp"
#include "constraint.hpp"

namespace std
{

inline ostream& operator<<(ostream& str, const rhea::variable& v)
{
    return v.is_nil()
        ? str << "NIL"
        : str << "{" << v.to_string() << v.id() << ":" << v.value() << "}";
}

inline ostream& operator<<(ostream& str, const rhea::linear_expression& v)
{
    for (auto& t : v.terms())
        str << t.first << "*" << t.second << " + ";

    return str << v.constant();
}

inline ostream& operator<<(ostream& str, const rhea::strength& s)
{
    return
        s == rhea::strength::required() ? str << "required" :
        s == rhea::strength::strong()   ? str << "strong" :
        s == rhea::strength::medium()   ? str << "medium" :
        s == rhea::strength::weak()     ? str << "weak" :
        /* else */                        str << s.weight().as_double();
}

inline ostream& operator<<(ostream& str, const rhea::abstract_constraint& c)
{
    return str
        << (c.is_edit_constraint() ? "edit" :
            c.is_stay_constraint() ? "stay" :
            /* else */               "linear")
        << " [" << c.get_strength() << ", " << c.weight() << "] "
        << c.expression()
        << (c.is_inequality()        ? " >= " :
            /* else */                 " == ")
        << "0";
}

inline ostream& operator<<(ostream& str, const rhea::constraint& c)
{
    if (c.is_nil())
        str << "NIL";
    else
        str << c.as<rhea::abstract_constraint>();

    return str;
}

inline ostream& operator<<(ostream& str, const rhea::tableau& v)
{
    str << "Tableau columns" << std::endl;
    for (auto& col : v.columns()) {
        str << "  " << col.first << " : ";
        for (auto& var : col.second)
            str << var << "  ";

        str << std::endl;
    }

    str << "Tableau rows" << std::endl;
    for (auto& row : v.rows())
        str << "  " << row.first << " : " << row.second << std::endl;

    return str;
}

} // namespace std

