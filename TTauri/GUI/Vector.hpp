//
//  Vector.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <boost/qvm/vec_access.hpp>
#include <boost/qvm/vec_traits.hpp>
#include <boost/qvm/vec_traits_defaults.hpp>
#include <cmath>

namespace TTauri {
namespace GUI {

struct float2 {
    float x;
    float y;

    float2 round(void) {
        return {::round(x), ::round(y)};
    }
};

struct float3 {
    float x;
    float y;
    float z;
};

}}

#pragma mark Traits
namespace boost {
namespace qvm {

template <>
struct vec_traits<TTauri::GUI::float2>: vec_traits_defaults<TTauri::GUI::float2,float,2> {
    template <int I>
    static inline scalar_type & write_element( TTauri::GUI::float2 & v ) {
        switch (I) {
        case 0: return v.x;
        case 1: return v.y;
        }
    }
};

template <>
struct vec_traits<TTauri::GUI::float3>: vec_traits_defaults<TTauri::GUI::float3,float,3> {
    template <int I>
    static inline scalar_type & write_element( TTauri::GUI::float3 & v ) {
        switch (I) {
        case 0: return v.x;
        case 1: return v.y;
        case 2: return v.z;
        }
    }
};

}}
