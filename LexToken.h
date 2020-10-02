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
#ifndef FLOCK_COMPILER_LEX_TOKEN_H
#define FLOCK_COMPILER_LEX_TOKEN_H
#include "CachedSupplier.h"
#include "RawToken.h"
#include "Util.h"
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>

 /*
  * Lex Token has one main job
  * To work out the more symbolic tokens which reflect the language.
  */
using namespace std;
namespace flock {
	using namespace source;
	using namespace supplier;
	namespace token {
		enum class LexType {
			Unknown,
			Eof,
			Whitespace,
			Comment,
			Integer,
			Decimal,
			String,
			Identifier,
			Symbol

		};

		static string toString(const LexType type) {
			switch (type) {
			case LexType::Eof:
				return "Eof";
			case LexType::Whitespace:
				return "Whitespace";
			case LexType::Comment:
				return "Comment";
			case LexType::Integer:
				return "Integer";
			case LexType::Decimal:
				return "Decimal";
			case LexType::String:
				return "String";
			case LexType::Identifier:
				return "Identifier";
			case LexType::Symbol:
				return "Symbol";
			case LexType::Unknown:
			default:
				return "Unknown";
			}
		}

		class LexToken : public Token<LexType, _sp_vec<RawToken>> {
		public:
			LexToken(LexType type, _sp_vec<RawToken> rawtokens) : Token(type, rawtokens), joinedRange(joinRange(rawtokens)) {}
			LexToken(LexType type, _sp<RawToken> rawToken) : LexToken(type, tokenAsVector(rawToken)) {}
			LexToken(LexType type) : LexToken(type, _sp_vec<RawToken>()) {}

			_sp<Range> joinedRange;

			int getChar() {
				if (joinedRange) {
					return joinedRange->source.at(0);
				}
				return EOF;
			}

			string getString() {
				if (joinedRange) {
					return joinedRange->source;
				}
				return string();
			}

			friend ostream& operator<<(ostream& os, LexToken& token) {

				switch (token.getType()) {
				case LexType::Unknown:
				case LexType::Eof:
				case LexType::Whitespace:
					return os << toString(token.getType()) << "[" << token.getString().size() << "]";
				default:
					return os << toString(token.getType()) << ": '" << token.getString() << "'";
				}

			};

		protected:
			static _sp<Range> joinRange(_sp_vec<RawToken> rawTokens) {
				if (rawTokens.size() == 1) {
					return rawTokens.at(0)->getContents();
				}
				Range* range = rawTokens.at(0)->getContents().get();
				for (auto it = rawTokens.begin() + 1; it != rawTokens.end(); ++it) {
					range = new Range(*range, *(*it)->getContents().get());
				}
				return make_shared<Range>(*range);
			}
			static _sp_vec<RawToken> tokenAsVector(_sp<RawToken> rawToken) {
				_sp_vec<RawToken> vec;
				vec.push_back(rawToken);
				return vec;
			}
		};

		class LexTokenizer : public CachedVectorSupplier<LexToken> {
		public:
			LexTokenizer(CachedVectorSupplier<RawToken>* rawTokenSupplier) : rawTokenSupplier(rawTokenSupplier) {}

			shared_ptr<LexToken> supply() override {
				return make_shared<LexToken>(decipherToken());
			};

		private:
			LexToken decipherToken() {
				switch (pollType()) {
				case RawType::Eof:
					return LexToken(LexType::Eof, raw_pop());
				case RawType::NewLine:
				case RawType::Whitespace: {
					int idx = 1;
					while (any_of<RawType>(pollType(idx++), {RawType::Whitespace, RawType::NewLine})) {}
					return LexToken(LexType::Whitespace, vec_pop(idx-1));
				}
				case RawType::Punctuation: {
					const int nextChar = pollChar();
					if (any_of(nextChar, "\"'")) {
						int idx = 1;
						while (pollType(idx) != RawType::Eof && (nextChar != pollChar(idx) || pollChar(idx - 1) == '\\')) {
							idx++;
						}
						return LexToken(LexType::String, vec_pop(idx + 1));
					}
					
					const string nextChars = pollString(2);
					if (nextChars == "//") {
						int idx = 2;
						while (none_of<RawType>(pollType(idx++), { RawType::NewLine, RawType::Eof })) {}
						return LexToken(LexType::Comment, vec_pop(idx-1));
					}
					if (nextChars == "/*") {
						int idx = 2;
						while (pollType(idx) != RawType::Eof && pollString(2, idx) != "*/") {
							idx++;
						}
						return LexToken(LexType::Comment, vec_pop(idx + 2));
					} 
					if (any_of(nextChar, "_$")) {
						int idx = endIndexOfIdentifier(1);
						if (idx > 1) {
							return LexToken(LexType::Identifier, vec_pop(idx + 1));
						}
						return LexToken(LexType::Symbol, raw_pop());
					}
					if (any_of(nextChar, "[](){}<>.,;:/\\#-+*%|&~@?!^=_$")) { //_$ already captured, but if I change my mind at least they are there
						return LexToken(LexType::Symbol, raw_pop());
					}
				}
				case RawType::Integer: {
					const RawType nextType = pollType(1);
					if (any_of<RawType>(nextType, { RawType::Punctuation, RawType::Whitespace, RawType::NewLine, RawType::Eof })) {
						if (pollChar(1) != '.' || pollType(2) != RawType::Integer) {
							return LexToken(LexType::Integer, raw_pop());
						}
						if (pollChar(3) != '.' || pollType(4) != RawType::Integer) {
							return LexToken(LexType::Decimal, vec_pop(3));
						}
					}
				}
				case RawType::Alpha: {
					int idx = endIndexOfIdentifier(1);
					return LexToken(LexType::Identifier, vec_pop(idx));
				}
				default:
					break;
				}
				return LexToken(LexType::Unknown, raw_pop());
			};
			int endIndexOfIdentifier(const int index)
			{
				int idx = index;
				RawType type = pollType(idx);
				while (any_of<RawType>(type, { RawType::Alpha, RawType::Integer }) || (type == RawType::Punctuation && any_of(pollChar(idx), "_$"))) {
					type = pollType(++idx);
				}
				return idx;
			};
			int pollChar(const int idx = 0, const int pos = 0) {
				const string value = pollTokenString(idx);
				if (pos >= value.size()) {
					return -1;
				}
				return value.at(pos);
			};
			string pollString(const int count = -1, const int idx = 0, const int pos = 0) {
				string str;
				int index = idx;
				do {
					if (pollType(index) == RawType::Eof) {
						if (str.size() > pos) {
							return str.substr(pos);
						}
						return string();
					}
					// won't ever be a null pointer because pollType does that check already.
					str += pollTokenRange(index++)->source;
				} while (count == -1 || (str.size() < count + pos));
				return str.substr(pos, count);
			};
			string pollTokenString(const int idx = 0) {
				auto value = pollTokenRange(idx);
				if (value == nullptr) {
					return string();
				}
				return value->source;
			};
			_sp<Range> pollTokenRange(const int idx = 0) {
				auto value = rawTokenSupplier->poll(idx);
				if (value == nullptr) {
					return nullptr;
				}
				return value->getContents();
			};
			RawType pollType(const int idx = 0) {
				auto value = rawTokenSupplier->poll(idx);
				if (value == nullptr) {
					return RawType::Eof;
				}
				return value->getType();
			};
			_sp<RawToken> raw_poll(const int idx = 0) {
				return rawTokenSupplier->poll(idx);
			}
			_sp<RawToken> raw_pop() {
				return rawTokenSupplier->pop();
			}
			_sp_vec<RawToken> vec_pop(const int amount = 1) {
				return rawTokenSupplier->popRange(amount);
			}

			CachedVectorSupplier<RawToken>* rawTokenSupplier;
		};
	}
}
#endif
