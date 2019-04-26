
#pragma once

namespace TTauri::Config {

struct ASTNode {
    ASTLocation location;
    ASTNode(ASTLocation location) : location(location) {}

    virtual std::string str() { return "<node>"; }
};

}
