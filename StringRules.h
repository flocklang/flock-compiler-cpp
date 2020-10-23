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
#ifndef FLOCK_COMPILER_STRING_RULES_H
#define FLOCK_COMPILER_STRING_RULES_H

#include "Rules.h"
#include <vector>

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
namespace flock {
	namespace rule {
		using namespace std;
		using namespace types;
		enum StringRules {
			CharRange = -101,
			EqualString = -102,
			EqualChar = -103
		};

		static _sp<Rule> EQ(string value) {
			return _valueRule<string>(StringRules::EqualString, value);
		}
		static _sp<Rule> EQ(vector<string> values) {
			return _valueRule<string>(StringRules::EqualString, values);
		}
		static _sp<Rule> EQ(initializer_list<string> values) {
			return _valueRule<string>(StringRules::EqualString, values);
		}
		static _sp<Rule> EQ(int value) {
			return _valueRule<int>(StringRules::EqualChar, value);
		}
		static _sp<Rule> EQ(vector<int> values) {
			return _valueRule<int>(StringRules::EqualChar, values);
		}
		static _sp<Rule> EQ(initializer_list<int> values) {
			return _valueRule<int>(StringRules::EqualChar, values);
		}
		static _sp<Rule> RANGE(int start, int end) {
			return _valueRule<int>(StringRules::CharRange, start, end);
		}
		static _sp<Rule> NEW_LINE() {
			return  EQ({ '\n', '\r' });
		}
		static _sp<Rule> BLANK() {
			return  EQ({ ' ', '\t', '\v', '\f' });
		}
		static _sp<Rule> WHITESPACE() {
			return  OR(BLANK(), NEW_LINE());
		}
		static _sp<Rule> UPPER_ALPHA() {
			return  RANGE('A', 'Z');
		}
		static _sp<Rule> LOWER_ALPHA() {
			return  RANGE('a', 'z');
		}
		static _sp<Rule> alpha() {
			return  OR(LOWER_ALPHA(), UPPER_ALPHA());
		}
		static _sp<Rule> digit() {
			return  RANGE('0', '9');
		}
	}
}
#endif