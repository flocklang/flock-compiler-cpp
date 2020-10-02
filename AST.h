/*
 * Copyright 2020 John Orlando Keleshian Moxley, All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef FLOCK_COMPILER_AST_H
#define FLOCK_COMPILER_AST_H
#include <string>
#include <memory>
#include <vector>

 //===----------------------------------------------------------------------===//
 // Abstract Syntax Tree (aka Parse Tree) Using LLVM Kalediscope Tutorial as Base
 //===----------------------------------------------------------------------===//
using namespace std;
namespace flock {
	namespace ast {
        /// ExprAST - Base class for all expression nodes.
        class ExprAST {
        public:
            virtual ~ExprAST() = default;
        };

        /// NumberExprAST - Expression class for numeric literals like "1.0".
        class NumberExprAST : public ExprAST {
            double Val;

        public:
            NumberExprAST(double Val) : Val(Val) {}
        };

        /// VariableExprAST - Expression class for referencing a variable, like "a".
        class VariableExprAST : public ExprAST {
            std::string Name;

        public:
            VariableExprAST(const std::string& Name) : Name(Name) {}
        };

        /// BinaryExprAST - Expression class for a binary operator.
        class BinaryExprAST : public ExprAST {
            char Op;
            std::unique_ptr<ExprAST> LHS, RHS;

        public:
            BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
                : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
        };

        /// CallExprAST - Expression class for function calls.
        class CallExprAST : public ExprAST {
            std::string Callee;
            std::vector<std::unique_ptr<ExprAST>> Args;

        public:
            CallExprAST(const std::string& Callee,
                std::vector<std::unique_ptr<ExprAST>> Args)
                : Callee(Callee), Args(std::move(Args)) {}
        };

        /// PrototypeAST - This class represents the "prototype" for a function,
        /// which captures its name, and its argument names (thus implicitly the number
        /// of arguments the function takes).
        class PrototypeAST {
            std::string Name;
            std::vector<std::string> Args;

        public:
            PrototypeAST(const std::string& Name, std::vector<std::string> Args)
                : Name(Name), Args(std::move(Args)) {}

            const std::string& getName() const { return Name; }
        };

        /// FunctionAST - This class represents a function definition itself.
        class FunctionAST {
            std::unique_ptr<PrototypeAST> Proto;
            std::unique_ptr<ExprAST> Body;

        public:
            FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                std::unique_ptr<ExprAST> Body)
                : Proto(std::move(Proto)), Body(std::move(Body)) {}
        };

    }
}
#endif

