//---------------------------------------------------------------------------
/// \file   float_variable.hpp
/// \brief  A floating point variable that can be used in an expression
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "abstract_variable.hpp"

namespace rhea
{

/** A plain-old-datatype variable. */
template <typename T>
class pod_variable : public abstract_variable
{
    typedef abstract_variable super;

public:
    pod_variable(T value)
        : abstract_variable{}
        , value_{value}
    {
    }

    virtual ~pod_variable() {}

    virtual bool is_dummy() const { return false; }
    virtual bool is_external() const { return true; }
    virtual bool is_pivotable() const { return false; }
    virtual bool is_restricted() const { return false; }

    virtual void set_value(T new_value) { value_ = new_value; }

    virtual void change_value(T new_value) { value_ = new_value; }

    virtual std::string to_string() const { return "var"; }

protected:
    T value_;
};

/** A floating-point variable. */
class float_variable : public pod_variable<double>
{
public:
    float_variable()
        : pod_variable{0.0}
    {
    }

    float_variable(double value)
        : pod_variable{value}
    {
    }

    virtual ~float_variable() {}

    virtual bool is_float() const { return true; }

    virtual double value() const { return value_; }

    virtual int int_value() const
    {
        return static_cast<int>(value_ + (value_ > 0.0 ? 0.5 : -0.5));
    }
};

} // namespace rhea
