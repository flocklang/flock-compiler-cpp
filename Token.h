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
#ifndef FLOCK_COMPILER_TOKEN_H
#define FLOCK_COMPILER_TOKEN_H
#include <utility>
#include "LocationSupplier.h"

namespace flock {
    using namespace supplier;
    using namespace source;
    namespace token {
        enum class Type {
            Unknown,
            Eof,
            Whitespace,
            NewLine,
            Comment,
            String,
            Number,
            Symbol

        };
        static std::string toString(const Type type) {
            switch (type) {
            case Type::Eof:
                return "Eof";
            case Type::Whitespace:
                return "Whitespace";
            case Type::NewLine:
                return "NewLine";
            case Type::Comment:
                return "Comment";
            case Type::String:
                return "String";
            case Type::Number:
                return "Number";
            case Type::Symbol:
                return "Symbol";
            case Type::Unknown:
            default:
                return "Unknown";
            }
        }
       
        class Token {
        public:
            Token(const source::Range source,    const Type type) : source(source), type(type) {}
            Token(const source::Location source, const Type type) : source(source::Range(source)), type(type) {}
            Type getType()
            {
               return type;
            }
            source::Range getSource()
            {
                return source;
            }
            friend std::ostream& operator<<(std::ostream& os, const Token& token) {
                return os << "type: " << toString(token.type) << ", source: " << token.source;
            };
           
        private:
            const source::Range source;
            const Type type;
        };


        class Tokenizer : public Supplier<std::shared_ptr<Token>>{
        public:
            Tokenizer(Supplier<int> * charSupplier) : locationSupplier(LocationSupplier(charSupplier)) {}

            std::shared_ptr<Token> supply() override
            {
                std::shared_ptr<Location> location = locationSupplier.supply();
                return std::make_shared<Token>(eof(location));
            };
            Token eof(std::shared_ptr<Location> location) {
                return Token(*location, Type::Eof);
            };
        private:
            LocationSupplier locationSupplier;
        };

        /*void soSomething() {
            Tokenizer<supplier::ConsoleLocationSupplier> tokenizer(supplier::ConsoleLocationSupplier);
        }*/
    }
}
#endif
