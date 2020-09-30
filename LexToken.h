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
			NewLine,
			Comment

		};

		static string toString(const LexType type) {
			switch (type) {
			case LexType::Eof:
				return "Eof";
			case LexType::Whitespace:
				return "Whitespace";
			case LexType::NewLine:
				return "NewLine";
			case LexType::Comment:
				return "Comment";
			case LexType::Unknown:
			default:
				return "Unknown";
			}
		}

		class LexToken : public Token<LexType, _sp_vec<RawToken>> {
		public:
			LexToken(LexType type, _sp_vec<RawToken> rawtokens) : Token(type, rawtokens) {}
			LexToken(LexType type, _sp<RawToken> rawToken) : Token(type, tokenAsVector(rawToken)) {}
			LexToken(LexType type) : Token(type, _sp_vec<RawToken>()) {}

			friend ostream& operator<<(ostream& os, LexToken& token) {

				switch (token.getType()) {
				case LexType::Unknown:
				case LexType::Eof:
				case LexType::Whitespace:
				case LexType::NewLine:
					return os <<  toString(token.getType()) << "[" << (token.getContents()).size() << "]";
				default:
					std::ostringstream oss;
					string parsed;
					auto vec = token.getContents();
					if (!vec.empty())
					{
						for (auto it = vec.begin(); it != vec.end(); ++it) {
							/* std::cout << *it; ... */
							oss << (**it).getContents()->source;
						}
					}
					return os << toString(token.getType()) << ": '" << oss.str() << "'";
				}

			};

		private:
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
					return LexToken(LexType::NewLine, raw_pop());
				case RawType::Unknown:
					return LexToken(LexType::Unknown, raw_pop());
				case RawType::Punctuation:
					if (pollChar(0) == '/' && pollChar(1) == '/') {
						int idx = 2;
						while (pollType(idx) != RawType::NewLine && pollType(idx) != RawType::Eof) {
							idx++;
						}
						return LexToken(LexType::Comment, vec_pop(idx));
					}
					if (pollChar(0) == '/' && pollChar(1) == '*') {
						int idx = 2;
						while (pollType(idx) != RawType::Eof && !(pollChar(idx) == '*' && pollChar(idx + 1) == '/')) {
							idx++;
						}
						return LexToken(LexType::Comment, vec_pop(idx + 2));
					}
				default:
					break;
				}
				return LexToken(LexType::Unknown, raw_pop());
			};
			int pollChar(const int idx = 0, const int pos = 0) {
				return pollString(idx).at(pos);
			};
			string pollChars(const int count = -1, const int idx = 0, const int pos = 0) {
				return pollRange(idx)->source.substr(pos, count);
			};
			string pollString(const int idx = 0) {
				return pollRange(idx)->source;
			};
			_sp<Range> pollRange(const int idx = 0) {
				return rawTokenSupplier->poll(idx)->getContents();
			};
			RawType pollType(const int idx = 0) {
				return rawTokenSupplier->poll(idx)->getType();
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
