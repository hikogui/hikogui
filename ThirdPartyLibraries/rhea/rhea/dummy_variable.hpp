//---------------------------------------------------------------------------
/// \file   dummy_variable.hpp
/// \brief  Dummy placeholder used when solving a tableau
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "abstract_variable.hpp"

namespace rhea
{

/** Dummy variables are inserted by the simplex solver as a marker when
 ** incrementally removing a required equality constraint.  */
class dummy_variable : public abstract_variable
{
public:
    dummy_variable()
        : abstract_variable()
    {
    }

    virtual ~dummy_variable() {}

    virtual bool is_dummy() const { return true; }
    virtual bool is_external() const { return false; }
    virtual bool is_pivotable() const { return false; }
    virtual bool is_restricted() const { return true; }

    virtual std::string to_string() const { return "dummy"; }
};

} // namespace rhea
