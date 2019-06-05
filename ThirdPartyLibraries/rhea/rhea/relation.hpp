//---------------------------------------------------------------------------
/// \file   relation.hpp
/// \brief  Relation between two sides in an equation or inequality
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

namespace rhea
{

/** An (in)equality relation. */
class relation
{
public:
    /** This enumeration is set up in such a way that additive inverse flips
     ** the direction of the inequality. */
    typedef enum {
        /** Equal to. */
        eq = 0,
        /** Less than or equal to. */
        leq = 1,
        /** Greater than or equal to. */
        geq = -1,
    } comp_t;

public:
    relation(comp_t type = eq)
        : type_(type)
    {
    }

    comp_t type() const { return type_; }

    relation reverse_inequality() const { return comp_t(-(int)type_); }

    std::string to_string() const
    {
        switch (type_) {
        case eq:
            return "==";
        case leq:
            return "<=";
        case geq:
            return ">=";
        default:
            assert(false);
            return "?";
        }
    }

    bool operator==(comp_t c) const { return type_ == c; }

private:
    comp_t type_;
};

} // namespace rhea
