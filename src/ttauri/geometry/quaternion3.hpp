
#pragma once

#include "../numeric_array.hpp"

namespace tt {

class quaternion3 {
public:
    quaternion3(vec3 axis, float angle) noexcept : v()
    {
        ttlet C = std::cos(angle / 2.0);
        ttlet S = std::sin(angle / 2.0);

        v = axis.normalize().v * S;
        v.w() = C;
    }

    std::pair<vec3,float> axis_and_angle() const noexcept
    {
        ttlet rcp_length = rcp_hypot<3>(v);
        ttlet length = 1.0f / inv_length;

        return {
            vec3{v.xyz0 * rcp_length},
            2.0f * std::atan2(length)
        };
    }

private:
    /**
     * For a rotation quartenion:
     *     xi + yj + zk + w
     *
     * 'w' is the angle over the unit vector (x, y, z).
     */
    f32x4 v;
};


}
