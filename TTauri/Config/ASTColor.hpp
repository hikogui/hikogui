
#pragma once

#include "ASTExpression.hpp"
#include <boost/format.hpp>
#include <glm/glm.hpp>

namespace TTauri::Config {

struct ASTColor : ASTExpression {
    glm::vec4 value;

    ASTColor(ASTLocation location, uint32_t value) : ASTExpression(location) {
        this->value = {
            gammaToLinear(static_cast<float>((value & 0xff) >> 24) / 255.0),
            gammaToLinear(static_cast<float>((value & 0xff) >> 16) / 255.0),
            gammaToLinear(static_cast<float>((value & 0xff) >> 8) / 255.0),
            static_cast<float>(value & 0xff) / 255.0
        };
    }

    std::string str() override {
        uint32_t tmp =
            (static_cast<uint32_t>(linearToGamma(value.r) * 255.0) << 24) |
            (static_cast<uint32_t>(linearToGamma(value.g) * 255.0) << 16) |
            (static_cast<uint32_t>(linearToGamma(value.b) * 255.0) << 8) |
            (static_cast<uint32_t>(value.a * 255.0);

        return (boost::format("#%08x") % tmp).str();
    }

    Value execute(ExecutionContext *context) override { 
        return value;
    } 

};

}
