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
#ifndef FLOCK_EBNF_RULE_FUNCTIONS_H
#define FLOCK_EBNF_RULE_FUNCTIONS_H

#include "RuleTypes.h"
#include "LocationSupplier.h"
#include "ConsoleFormat.h"
#include <vector>
#include <map>
#include <string>
#include <numeric>
#include <cstdarg>
 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
namespace flock {
	namespace ebnf {
		namespace rule {
			using namespace flock::ebnf::types;

			static _sp<SequentialRule> SEQ(initializer_list<_sp<Rule>> rules);
			static _sp<AnyRule> ANY();
			static _sp<AnyButRule> ANYBUT(_sp<Rule> rule);
			static _sp<AnyButRule> ANYBUT(initializer_list<_sp<Rule>> rules);
			static _sp<SymbolRule> SYM(string type, _sp<Rule> rule, bool highlightCollect = true);
			static _sp<EqualStringRule> EQ(initializer_list<string > values);
			static _sp<EqualStringRule> EQ(string value);
			static _sp<EqualCharRule> EQ(initializer_list<int> values);
			static _sp<EqualCharRule> EQ(vector<int> values);
			static _sp<EqualCharRule> EQ(int value);
			static _sp<EqualCharRule> equalRange(int start, int end);
			static _sp<EOFRule> eof();
			static _sp<RepeatRule> REP(_sp<Rule> rule, const int min = 0, const int max = 0);
			static _sp<NotRule> NOT(_sp<Rule> rule);
			static _sp<NotRule> NOT(initializer_list<_sp<Rule>> rules);
			static _sp<OrRule> OR(initializer_list<_sp<Rule>> rules);
			static _sp<OrRule> OR(_sp<Rule> left, _sp<Rule> right);
			static _sp<OrRule> OR(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right);
			static _sp<AndRule> AND(initializer_list<_sp<Rule>> rules);
			static _sp<AndRule> AND(_sp<Rule> left, _sp<Rule> right);
			static _sp<AndRule> AND(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right);
			static _sp<XorRule> XOR(initializer_list<_sp<Rule>> rules);
			static _sp<XorRule> XOR(_sp<Rule> left, _sp<Rule> right);
			static _sp<SequentialRule> SEQ(_sp<Rule> left, _sp<Rule> right);
			static _sp<SequentialRule> SEQ(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right);
			static _sp<OptionalRule> OPT(_sp<Rule> rule);
			static _sp<OptionalRule> OPT(initializer_list<_sp<Rule>> rules);
			static _sp<RepeatRule> UNTIL(_sp<Rule> rule);
			static _sp<SequentialRule> UNTIL_INC(_sp<Rule> rule);
			static _sp<SymbolRule> keyword(string keyword);
			static _sp<SymbolRule> keyword(initializer_list<string> keywords);
			static _sp<Rule> RULE(string grammerName);
			static _sp<EqualCharRule> new_line();
			static _sp<EqualCharRule> blank();
			static _sp<OrRule> whitespace();
			static _sp<EqualCharRule> uppercase_alpha();
			static _sp<EqualCharRule> lowercase_alpha();
			static _sp<OrRule> alpha();
			static _sp<EqualCharRule> digit();


			static _sp<SequentialRule> SEQ(initializer_list<_sp<Rule>> rules) {
				return make_shared<SequentialRule>(rules);
			}
			static _sp<AnyRule> ANY() {
				return make_shared<AnyRule>();
			}
			static _sp<AnyButRule> ANYBUT(_sp<Rule> rule) {
				return make_shared<AnyButRule>(rule);
			}
			static _sp<AnyButRule> ANYBUT(initializer_list<_sp<Rule>> rules) {
				return ANYBUT(SEQ(rules));
			}
			static _sp<SymbolRule> SYM(string type, _sp<Rule> rule, bool highlightCollect) {
				return make_shared<SymbolRule>(rule, type, highlightCollect);
			}

			static _sp<EqualStringRule> EQ(initializer_list<string > values) {
				return make_shared<EqualStringRule>(values);
			}
			static _sp<EqualStringRule> EQ(string value) {
				return EQ({ value });
			}
			static _sp<EqualCharRule> EQ(vector<int> values) {
				return make_shared<EqualCharRule>(values);
			}
			static _sp<EqualCharRule> EQ(initializer_list<int> values) {
				return make_shared<EqualCharRule>(values);
			}
			static _sp<EqualCharRule> EQ(int value) {
				return EQ({ value });
			}
			static _sp<EqualCharRule> equalRange(int start, int end) {
				vector<int> list((end - start) + 1);
				iota(std::begin(list), std::end(list), start);
				return EQ(list);
			}
			static _sp<EOFRule> eof() {
				return make_shared<EOFRule>();
			}


			static _sp<NotRule> NOT(_sp<Rule> rule) {
				return make_shared<NotRule>(rule);
			}
			static _sp<NotRule> NOT(initializer_list<_sp<Rule>> rules) {
				return make_shared<NotRule>(SEQ(rules));
			}
			static _sp<RepeatRule> REP(_sp<Rule> rule, int min, int max) {
				return make_shared<RepeatRule>(rule, min, max);
			}
			static _sp<OrRule> OR(initializer_list<_sp<Rule>> rules) {
				return make_shared<OrRule>(rules);
			}
			static _sp<OrRule> OR(_sp<Rule> left, _sp<Rule> right) {
				return OR({ left, right });
			}
			static _sp<OrRule> OR(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right) {
				return OR({ left, middle, right });
			}
			static _sp<AndRule> AND(initializer_list<_sp<Rule>> rules) {
				return make_shared<AndRule>(rules);
			}
			static _sp<AndRule> AND(_sp<Rule> left, _sp<Rule> right) {
				return AND({ left, right });
			}
			static _sp<AndRule> AND(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right) {
				return AND({ left, middle, right });
			}
			static _sp<XorRule> XOR(initializer_list<_sp<Rule>> rules) {
				return make_shared<XorRule>(rules);
			}
			static _sp<XorRule> XOR(_sp<Rule> left, _sp<Rule> right) {
				return XOR({ left, right });
			}
			static _sp<SequentialRule> SEQ(_sp<Rule> left, _sp<Rule> right) {
				return SEQ({ left, right });
			}
			static _sp<SequentialRule> SEQ(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right) {
				return SEQ({ left, middle, right });
			}

			static _sp<OptionalRule> OPT(_sp<Rule> rule) {
				return make_shared<OptionalRule>(rule);
			}
			static _sp<OptionalRule> OPT(initializer_list<_sp<Rule>> rules) {
				return OPT(SEQ(rules));
			}
			static _sp<RepeatRule> UNTIL(_sp<Rule> rule) {
				return REP(ANYBUT(rule));
			}
			static _sp<SequentialRule> UNTIL_INC(_sp<Rule> rule) {
				return SEQ(REP(ANYBUT(rule)), rule);
			}
			static _sp<SymbolRule> keyword(string keyword) {
				return SYM("keyword", EQ(keyword));
			}
			static _sp<SymbolRule> keyword(initializer_list<string> keywords) {
				return SYM("keyword", EQ(keywords));
			}

			/// <summary>
			/// </summary>
			/// <param name="grammer"></param>
			/// <returns></returns>
			static _sp<Rule> RULE(string grammerName) {
				switch (grammerName.back()) {
				case '*': {
					string name = grammerName.substr(0, grammerName.length() - 1);
					_sp<GrammarRule> grammarRule = make_shared<GrammarRule>(name);
					return REP(grammarRule);
				}
				case '+': {
					string name = grammerName.substr(0, grammerName.length() - 1);
					_sp<GrammarRule> grammarRule = make_shared<GrammarRule>(name);
					return SEQ(grammarRule, REP(grammarRule));
				}
				case '?': {
					string name = grammerName.substr(0, grammerName.length() - 1);
					_sp<GrammarRule> grammarRule = make_shared<GrammarRule>(name);
					return OPT(grammarRule);

				}
				case '-': {
					string name = grammerName.substr(0, grammerName.length() - 1);
					_sp<GrammarRule> grammarRule = make_shared<GrammarRule>(name);
					return ANYBUT(grammarRule);
				}

				default: {
					return make_shared<GrammarRule>(grammerName);
				}

				}
			}
			static _sp<EqualCharRule> new_line() {
				return  EQ({ '\n', '\r' });
			}
			static _sp<EqualCharRule> blank() {
				return  EQ({ ' ', '\t', '\v', '\f' });
			}
			static _sp<OrRule> whitespace() {
				return  OR(blank(), new_line());
			}
			static _sp<EqualCharRule> uppercase_alpha() {
				return  equalRange('A', 'Z');
			}
			static _sp<EqualCharRule> lowercase_alpha() {
				return  equalRange('a', 'z');
			}
			static _sp<OrRule> alpha() {
				return  OR(uppercase_alpha(), lowercase_alpha());
			}
			static _sp<EqualCharRule> digit() {
				return  equalRange('0', '9');
			}


			
		}
	}
}
#endif