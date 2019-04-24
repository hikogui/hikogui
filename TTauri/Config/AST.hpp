
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <any>

namespace TTauri::Config {

struct ASTNode {
    size_t pos;
};

struct ASTIdentifier : ASTNode {
    std::string value;
};

struct ASTKey : ASTNode {
    std::vector<std::shared_ptr<ASTIdentifier>> ids;

};

struct ASTExpression: ASTNode {
};

struct ASTFunction: ASTExpression {
    std::shared_ptr<ASTExpression> left;
    std::vector<std::shared_ptr<ASTExpression>> arguments;
};

struct ASTBinaryExpression: ASTExpression {
    std::shared_ptr<ASTExpression> left;
    std::shared_ptr<ASTExpression> right;
    std::string op;
};

struct ASTIndicingExpression: ASTExpression {
    std::shared_ptr<ASTExpression> left;
    std::shared_ptr<ASTExpression> index;
};

struct ASTLiteral: ASTExpression {
    std::any value;
};

struct ASTName: ASTExpression {
    std::string name;
};

struct ASTObject: ASTExpression {
    std::vector<std::shared_ptr<ASTStatement>> statements;
};

struct ASTList: ASTExpression {
    std::vector<std::shared_ptr<ASTExpression>> expressions;
};


struct ASTStatement: ASTNode {

};

struct ASTKeyStatement: ASTStatement {
    std::shared_ptr<ASTKey> key;
};

struct ASTAssignmentStatement: ASTStatement {
    std::shared_ptr<ASTKey> key;
    std::shared_ptr<ASTExpression> expression;
};

struct ASTExpressionStatement: ASTStatement {
    std::shared_ptr<ASTExpression> expression;
};

}