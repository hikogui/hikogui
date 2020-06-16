//---------------------------------------------------------------------------
/// \file   edit_constraint.hpp
/// \brief  Edit constraint
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "edit_or_stay_constraint.hpp"

namespace rhea
{

/** Edit constraints are added to a tableau on a variable, so that a
 ** new value can be suggested for that variable later on. */
class edit_constraint : public edit_or_stay_constraint
{
public:
    edit_constraint(const variable& v, const strength& s = strength::strong(),
                    double weight = 1.0)
        : edit_or_stay_constraint{v, s, weight}
    {
    }

    virtual ~edit_constraint() {}

    virtual bool is_edit_constraint() const { return true; }

    virtual bool is_satisfied() const { return false; }
};

} // namespace rhea
