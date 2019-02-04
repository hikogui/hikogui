//
//  Backing.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <memory>
#include <functional>
#include <cmath>
#include <boost/functional/hash.hpp>
#include "Vector.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Window;
class Device;

/** Shared backing of a View.
 * Non modifiable, can be used in a std::set.
 */
class Backing {
public:
    const float2 size;

     //! Convenient reference to the GUI.
    std::weak_ptr<Device> device;

    //! Convenient reference to the GUI.
    std::weak_ptr<Window> window;

    /** Construct a backing of a certain size.
     * \param size Size of the backing image, will be rounded to nearest integer.
     */
    Backing(std::weak_ptr<Window> window, float2 size);

    virtual ~Backing();

    virtual size_t hash(void) const {
        size_t seed = 0;
        boost::hash_combine(seed, boost::hash_value(size.x));
        boost::hash_combine(seed, boost::hash_value(size.y));
        return seed;
    }

    virtual bool operator==(const Backing &other) const {
        bool result = true;
        result &= size.x == other.size.x;
        result &= size.y == other.size.y;
        return result;
    }

};

}}}

namespace std {
  template <> struct hash<std::shared_ptr<TTauri::Toolkit::GUI::Backing>>
  {
    size_t operator()(const std::shared_ptr<TTauri::Toolkit::GUI::Backing> &x) const
    {
        return x->hash();
    }
  };

  template <> struct equal_to<std::shared_ptr<TTauri::Toolkit::GUI::Backing>>
  {
    size_t operator()(const std::shared_ptr<TTauri::Toolkit::GUI::Backing> &a, const std::shared_ptr<TTauri::Toolkit::GUI::Backing> &b) const
    {
        return *a == *b;
    }
  };
}
