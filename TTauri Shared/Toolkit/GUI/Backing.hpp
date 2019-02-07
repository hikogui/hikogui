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
#include <typeinfo>
#include <typeindex>
#include <boost/functional/hash.hpp>
#include <vulkan/vulkan.hpp>
#include "Vector.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Window;
class Instance;

/** Backing of a Windget.
 * The Backing contains static data and drawing code. Backings are shared by a View.
 *
 * All static data that is needed to render images of a Widget needs to be initialized
 * in the constructor and are const. The hash() and operator==() need to include all
 * static data of a Backing. This allows sharing between Views and caching of image
 * rendering.
 */
class Backing {
public:
    const VkExtent2D size;

     //! Convenient reference to the GUI.
    Instance *instance;

    //! Convenient reference to the GUI.
    Window *window;

    /** Construct a backing of a certain size.
     * \param size Size of the backing image, will be rounded to nearest integer.
     */
    Backing(Window *window, VkExtent2D size);

    virtual ~Backing();

    virtual size_t hash(void) const {
        size_t seed = 0;
        boost::hash_combine(seed, boost::hash_value(size.width));
        boost::hash_combine(seed, boost::hash_value(size.height));
        return seed;
    }

    virtual bool operator==(const Backing &other) const {
        auto &this_type_id = typeid(*this);
        auto &other_type_id = typeid(other);
        auto this_type_index = std::type_index(this_type_id);
        auto other_type_index = std::type_index(other_type_id);

        return (
            (this_type_index == other_type_index) and
            (size.width != other.size.width) and
            (size.height != other.size.height)
        );
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
