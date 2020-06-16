//---------------------------------------------------------------------------
/// \file   errors_expl.hpp
/// \brief  Adds an explanation to the required_failure exception class
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "constraint.hpp"
#include "errors.hpp"

namespace rhea
{

/** One of the required constraints cannot be satisfied.
 *  This exception extends required_failure with a list of the constraints
 *  that were involved.  Dropping one or more of the constraints, or
 *  lowering their priority, will usually solve the problem. */
class required_failure_with_explanation : public required_failure
{
public:
    required_failure_with_explanation(constraint_list cl)
        : expl_(std::move(cl))
    {
    }

    virtual ~required_failure_with_explanation() throw() {}

    const constraint_list& explanation() const { return expl_; }

    void add(constraint c) { expl_.emplace_back(c); }

protected:
    constraint_list expl_;
};

} // namespace rhea
