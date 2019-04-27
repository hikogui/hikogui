#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

    struct ASTBinaryOperator : ASTExpression {
        enum class Operator {
            MUL,
            DIV,
            MOD,
            ADD,
            SUB,
            SHL,
            SHR,
            LT,
            GT,
            LE,
            GE,
            EQ,
            NE,
            AND,
            XOR,
            OR,
            LOGICAL_AND,
            LOGICAL_XOR,
            LOGICAL_OR
        };

        Operator op;
        ASTExpression *left;
        ASTExpression *right;

        ASTBinaryOperator(ASTLocation location, Operator op, ASTExpression *left, ASTExpression *right) : ASTExpression(location), op(op), left(left), right(right) {}

        ~ASTBinaryOperator() {
            delete left;
            delete right;
        }

        std::string str() override {
            std::string s = left->str();

            switch (op) {
            case Operator::MUL: s+= "*"; break;
            case Operator::DIV: s+= "/"; break;
            case Operator::MOD: s+= "%"; break;
            case Operator::ADD: s+= "+"; break;
            case Operator::SUB: s+= "-"; break;
            case Operator::SHL: s+= "<<"; break;
            case Operator::SHR: s+= ">>"; break;
            case Operator::LT: s+= "<"; break;
            case Operator::GT: s+= ">"; break;
            case Operator::LE: s+= "<="; break;
            case Operator::GE: s+= ">="; break;
            case Operator::EQ: s+= "=="; break;
            case Operator::NE: s+= "!="; break;
            case Operator::AND: s+= "&"; break;
            case Operator::XOR: s+= "^"; break;
            case Operator::OR: s+= "|"; break;
            case Operator::LOGICAL_AND: s+= " and "; break;
            case Operator::LOGICAL_XOR: s+= " xor "; break;
            case Operator::LOGICAL_OR: s+= " or "; break;
            }

            return s + right->str();
        }

        virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) override { 
            switch (op) {
            case Operator::MUL: return (*left->execute(context)) * (*right->execute(context));
            case Operator::DIV: return (*left->execute(context)) / (*right->execute(context));
            case Operator::MOD: return (*left->execute(context)) % (*right->execute(context));
            case Operator::ADD: return (*left->execute(context)) + (*right->execute(context));
            case Operator::SUB: return (*left->execute(context)) - (*right->execute(context));
            case Operator::SHL: return (*left->execute(context)) << (*right->execute(context));
            case Operator::SHR: return (*left->execute(context)) >> (*right->execute(context));
            case Operator::LT: return (*left->execute(context)) < (*right->execute(context));
            case Operator::GT: return (*left->execute(context)) > (*right->execute(context));
            case Operator::LE: return (*left->execute(context)) <= (*right->execute(context));
            case Operator::GE: return (*left->execute(context)) >= (*right->execute(context));
            case Operator::EQ: return (*left->execute(context)) == (*right->execute(context));
            case Operator::NE: return (*left->execute(context)) != (*right->execute(context));
            case Operator::AND: return (*left->execute(context)) & (*right->execute(context));
            case Operator::XOR: return (*left->execute(context)) ^ (*right->execute(context));
            case Operator::OR: return (*left->execute(context)) | (*right->execute(context));
            case Operator::LOGICAL_AND: return (*left->execute(context)) && (*right->execute(context));
            case Operator::LOGICAL_XOR: return (*left->execute(context)).operator_xor(*right->execute(context));
            case Operator::LOGICAL_OR: return (*left->execute(context)) || (*right->execute(context));
            }
        } 
    };

}