//
//  BackingCache.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <boost/exception/all.hpp>
#include "Backing.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

struct BackingCacheError: virtual boost::exception, virtual std::exception {};

/*! A cache of Backings.
 * Used by simular Views to share identical Backings.
 *
 * It is important to balance calls to emplace() and erase().
 */
class BackingCache {
    std::unordered_map<std::shared_ptr<Backing>, int> backings;
public:

    /*! Check if an backing was already in use and return it or add it to the cache.
     */
    std::shared_ptr<Backing> emplace(std::shared_ptr<Backing> backing) {
        auto i = backings.find(backing);

        if (i == backings.end()) {
            backings[backing] = 1;
            return backing;
        } else {
            (*i).second++;
            return (*i).first;
        }
    }

    /*! Remove a backing from the cache.
     * Removes a backing from the cache if it is no longer used by a View.
     */
    void erase(std::shared_ptr<Backing> backing) {
        auto i = backings.find(backing);
        if (i == backings.end()) {
            BOOST_THROW_EXCEPTION(BackingCacheError());
        } else {
            backings.erase(i);
        }
    }
};

}}}
