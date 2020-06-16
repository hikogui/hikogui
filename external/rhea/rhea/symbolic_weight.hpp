//---------------------------------------------------------------------------
/// \file   symbolic_weight.hpp
/// \brief  A 3-tuple weight for constraints
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <array>

namespace rhea
{

/** A 3-tuple weight for constraint strengths.
 * In the original implementation this was an n-tuple, but it has been fixed
 * at 3 in Rhea.  The three elements correspond to the strong, medium and
 * weak constraints.  Every constraint can also have a weight (1 by
 * default).  Symbolic weights are then ordered lexicographically: strong
 * weights always outclass medium weights, no matter what the values.
 *
 * The end effect is that strong constraints are satisfied before the
 * medium ones, and the weak constraints are satisfied last.  Within each
 * of the three classes of constraints, you can make further adjustments
 * by changing the weight. */
class symbolic_weight
{
public:
    symbolic_weight();
    symbolic_weight(double w1, double w2, double w3);

    static symbolic_weight zero();

    symbolic_weight& negate();
    symbolic_weight& operator*=(double n);
    symbolic_weight& operator/=(double n);
    symbolic_weight& operator+=(const symbolic_weight& n);
    symbolic_weight& operator-=(const symbolic_weight& n);

    bool operator<(const symbolic_weight& comp) const;
    bool operator<=(const symbolic_weight& comp) const;
    bool operator==(const symbolic_weight& comp) const;
    bool operator!=(const symbolic_weight& comp) const;
    bool operator>(const symbolic_weight& comp) const;
    bool operator>=(const symbolic_weight& comp) const;

    bool is_negative() const;

    double as_double() const
    {
        return values_[2] + values_[1] * 10000. + values_[0] * 10000000.;
    }

    size_t levels() const { return values_.size(); }

private:
    std::array<double, 3> values_;
};

inline symbolic_weight operator*(symbolic_weight w, double n)
{
    return w *= n;
}

inline symbolic_weight operator/(symbolic_weight w, double n)
{
    return w /= n;
}

inline symbolic_weight operator+(symbolic_weight w, const symbolic_weight& n)
{
    return w += n;
}

inline symbolic_weight operator-(symbolic_weight w, const symbolic_weight& n)
{
    return w -= n;
}

} // namespace rhea
