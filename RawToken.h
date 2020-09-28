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
#ifndef FLOCK_COMPILER_RAW_TOKEN_H
#define FLOCK_COMPILER_RAW_TOKEN_H
#include "Token.h"
#include "LocationSupplier.h"

/* Raw Token has one main job
 * To decipher whether we are looking at whitepace, alphabatical, numerics, or symbols. This is meant to be very simple. 
 * From raw tokens we can get symbolic tokens.
 */
namespace flock {
    using namespace supplier;
    using namespace source;
    namespace token {
        enum class RawType {
            Unknown,
            Eof,
            Whitespace,
            NewLine,
           // Comment,
            Alpha,
            Integer,
            Punctuation

        };
        static std::string toString(const RawType type) {
            switch (type) {
            case RawType::Eof:
                return "Eof";
            case RawType::Whitespace:
                return "Whitespace";
            case RawType::NewLine:
                return "NewLine";
  /*          case RawType::Comment:
                return "Comment";*/
            case RawType::Alpha:
                return "Alpha";
            case RawType::Integer:
                return "Integer";
            case RawType::Punctuation:
                return "Punctuation";
            case RawType::Unknown:
            default:
                return "Unknown";
            }
        }

        class RawToken : public Token<RawType, std::shared_ptr <Range>>{
        public:
            RawToken(RawType type, const Range range) : Token(type, toRange(range)) {}
            RawToken(RawType type, const Location location) : Token(type, toRange(std::make_shared<Location>(location))) {}
            RawToken(RawType type, const std::shared_ptr<Range> range) : Token(type, range) {}
            RawToken(RawType type, const std::shared_ptr<Location> location) : Token(type, toRange(location)) {}
            static std::shared_ptr <Range> toRange(Range range) {
                return std::make_shared<Range>(range);
            }
            static std::shared_ptr <Range> toRange(std::shared_ptr<Location> location) {
               return std::make_shared<Range>(Range(location));
            }

            friend std::ostream& operator<<(std::ostream& os, RawToken& token) {
                switch (token.getType()) {
                case RawType::Unknown:
                case RawType::Eof:
                case RawType::Whitespace:
                case RawType::NewLine:
                    return os << "type: " << toString(token.getType()) << ", source: " << (*token.getContents()).toStringNoText();
                default:
                    return os << "type: " << toString(token.getType()) << ", source: " << *token.getContents();
                }
            };
        };

        class RawTokenizer : public Supplier<std::shared_ptr<RawToken>> {
        public:
            RawTokenizer(Supplier<int>* charSupplier) : locationSupplier(LocationSupplier(charSupplier)) {}

            std::shared_ptr<RawToken> supply() override {
                return std::make_shared<RawToken>(decipherToken());
            };

        private:
            RawToken decipherToken() {
                if (pollChar() == EOF) {
                    return RawToken(RawType::Eof, loc_pop());
                }
                if (isnewline(pollChar())) {
                    int idx = 1;
                    while (isnewline(pollChar(idx))) {
                        idx++;
                    }
                    return RawToken(RawType::NewLine, range_pop(idx));
                }
                if (isblank(pollChar())) {
                    int idx = 1;
                    while (isblank(pollChar(idx))) {
                        idx++;
                    }
                    return RawToken(RawType::Whitespace, range_pop(idx));
                }
                if (isalpha(pollChar())) {
                    int idx = 1;
                    while (isalpha(pollChar(idx))) {
                        idx++;
                    }
                    return RawToken(RawType::Alpha, range_pop(idx));
                }
                // We only care about integers at this point, a decimal would be represented in a more refined token. This allows us to get closer to BNF style grammars, (no shortcuts)
                if (isdigit(pollChar())) {
                    int idx = 1;
                    while (isdigit(pollChar(idx))) {
                        idx++;
                    }
                    return RawToken(RawType::Integer, range_pop(idx));
                }
                // Punctuation we want to view one by one, as they have meaning.
                if (ispunct(pollChar())) {
                    return RawToken(RawType::Punctuation, loc_pop());
                }
                return RawToken(RawType::Unknown, loc_pop());
            };

            int pollChar(const int idx = 0) {
                return locationSupplier.poll(idx)->character;
            };
            std::shared_ptr<Location> loc_poll(const int idx = 0) {
                return locationSupplier.poll(idx);
            }
            std::shared_ptr<Location> loc_pop() {
                return locationSupplier.pop();
            }
            std::shared_ptr<Range> range_pop(const int amount = 1) {
                return locationSupplier.popRange(amount);
            }

            LocationSupplier locationSupplier;
        };
    }
}
#endif
