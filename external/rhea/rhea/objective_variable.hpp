//---------------------------------------------------------------------------
/// \file   objective_variable.hpp
/// \brief  The objective for a solver to work towards
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "abstract_variable.hpp"
#include "tableau.hpp"

namespace rhea
{

/** A special variable that is used internally by the solver as the
 ** objective to solve for. */
class objective_variable : public abstract_variable
{
public:
    objective_variable() {}

    virtual ~objective_variable() {}

    virtual bool is_pivotable() const { return false; }
    virtual bool is_restricted() const { return false; }

    std::string to_string() const { return "objective"; }
};

} // namespace rhea
