//
//  QueueCapabilities.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-07.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#pragma clang diagnostic ignored "-Wcomma"
#include <boost/format.hpp>
#pragma clang diagnostic pop

namespace TTauri {
namespace Toolkit {
namespace GUI {

struct QueueCapabilities {
    bool handlesGraphics;
    bool handlesCompute;
    bool handlesPresent;

    QueueCapabilities() :
        handlesGraphics(false), handlesCompute(false), handlesPresent(false) {}

    bool handlesEverything() const {
        return handlesGraphics and handlesCompute and handlesPresent;
    }

    bool handlesGraphicsAndPresent() const {
        return handlesGraphics and handlesPresent;
    }

    bool handlesGraphicsAndCompute() const {
        return handlesGraphics and handlesCompute;
    }

    std::string str(void) const {
        return (boost::format("%c%c%c") %
            (handlesGraphics ? "G" : ".") %
            (handlesCompute ? "C" : ".") %
            (handlesPresent ? "P" : ".")
        ).str();
    }

    bool handlesAllOff(const QueueCapabilities &other) const {
        return not (
            (other.handlesGraphics and not handlesGraphics) or
            (other.handlesCompute and not handlesCompute) or
            (other.handlesPresent and not handlesPresent)
        );
    }

    unsigned int score(void) const {
        unsigned int score = 0;
        score += handlesEverything() ? 10 : 0;
        score += handlesGraphicsAndPresent() ? 5 : 0;
        score += handlesGraphics ? 1 : 0;
        score += handlesPresent ? 1 : 0;
        score += handlesCompute ? 1 : 0;
        return score;
    }

    QueueCapabilities &operator|=(const QueueCapabilities &other) {
        handlesGraphics |= other.handlesGraphics;
        handlesCompute |= other.handlesCompute;
        handlesPresent |= other.handlesPresent;
        return *this;
    }

    QueueCapabilities operator-(const QueueCapabilities &other) const {
        QueueCapabilities result;
        result.handlesGraphics = handlesGraphics and not other.handlesGraphics;
        result.handlesCompute = handlesCompute and not other.handlesCompute;
        result.handlesPresent = handlesPresent and not other.handlesPresent;
        return result;
    }

};

}}}
