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

			static _sp<SequentialRule> seq(initializer_list<_sp<Rule>> rules);
			static _sp<AnyRule> any();
			static _sp<AnyButRule> anybut(_sp<Rule> rule);
			static _sp<AnyButRule> anybut(initializer_list<_sp<Rule>> rules);
			static _sp<SymbolRule> symbol(string type, _sp<Rule> rule, bool highlightCollect = true);
			static _sp<EqualStringRule> equal(initializer_list<string > values);
			static _sp<EqualStringRule> equal(string value);
			static _sp<EqualCharRule> equal(initializer_list<int> values);
			static _sp<EqualCharRule> equal(vector<int> values);
			static _sp<EqualCharRule> equal(int value);
			static _sp<EqualCharRule> equalRange(int start, int end);
			static _sp<EOFRule> eof();
			static _sp<RepeatRule> repeat(_sp<Rule> rule, const int min = 0, const int max = 0);
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
			static _sp<SequentialRule> seq(_sp<Rule> left, _sp<Rule> right);
			static _sp<SequentialRule> seq(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right);
			static _sp<OptionalRule> option(_sp<Rule> rule);
			static _sp<OptionalRule> option(initializer_list<_sp<Rule>> rules);
			static _sp<RepeatRule> until(_sp<Rule> rule);
			static _sp<SequentialRule> until_incl(_sp<Rule> rule);
			static _sp<SymbolRule> keyword(string keyword);
			static _sp<SymbolRule> keyword(initializer_list<string> keywords);
			static _sp<Rule> grammar(string grammerName);
			static _sp<EqualCharRule> new_line();
			static _sp<EqualCharRule> blank();
			static _sp<OrRule> whitespace();
			static _sp<EqualCharRule> uppercase_alpha();
			static _sp<EqualCharRule> lowercase_alpha();
			static _sp<OrRule> alpha();
			static _sp<EqualCharRule> digit();


			static _sp<SequentialRule> seq(initializer_list<_sp<Rule>> rules) {
				return make_shared<SequentialRule>(rules);
			}
			static _sp<AnyRule> any() {
				return make_shared<AnyRule>();
			}
			static _sp<AnyButRule> anybut(_sp<Rule> rule) {
				return make_shared<AnyButRule>(rule);
			}
			static _sp<AnyButRule> anybut(initializer_list<_sp<Rule>> rules) {
				return anybut(seq(rules));
			}
			static _sp<SymbolRule> symbol(string type, _sp<Rule> rule, bool highlightCollect) {
				return make_shared<SymbolRule>(rule, type, highlightCollect);
			}

			static _sp<EqualStringRule> equal(initializer_list<string > values) {
				return make_shared<EqualStringRule>(values);
			}
			static _sp<EqualStringRule> equal(string value) {
				return equal({ value });
			}
			static _sp<EqualCharRule> equal(vector<int> values) {
				return make_shared<EqualCharRule>(values);
			}
			static _sp<EqualCharRule> equal(initializer_list<int> values) {
				return make_shared<EqualCharRule>(values);
			}
			static _sp<EqualCharRule> equal(int value) {
				return equal({ value });
			}
			static _sp<EqualCharRule> equalRange(int start, int end) {
				vector<int> list((end - start) + 1);
				iota(std::begin(list), std::end(list), start);
				return equal(list);
			}
			static _sp<EOFRule> eof() {
				return make_shared<EOFRule>();
			}


			static _sp<NotRule> NOT(_sp<Rule> rule) {
				return make_shared<NotRule>(rule);
			}
			static _sp<NotRule> NOT(initializer_list<_sp<Rule>> rules) {
				return make_shared<NotRule>(seq(rules));
			}
			static _sp<RepeatRule> repeat(_sp<Rule> rule, int min, int max) {
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
			static _sp<SequentialRule> seq(_sp<Rule> left, _sp<Rule> right) {
				return seq({ left, right });
			}
			static _sp<SequentialRule> seq(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right) {
				return seq({ left, middle, right });
			}

			static _sp<OptionalRule> option(_sp<Rule> rule) {
				return make_shared<OptionalRule>(rule);
			}
			static _sp<OptionalRule> option(initializer_list<_sp<Rule>> rules) {
				return option(seq(rules));
			}
			static _sp<RepeatRule> until(_sp<Rule> rule) {
				return repeat(anybut(rule));
			}
			static _sp<SequentialRule> until_incl(_sp<Rule> rule) {
				return seq(repeat(anybut(rule)), rule);
			}
			static _sp<SymbolRule> keyword(string keyword) {
				return symbol("keyword", equal(keyword));
			}
			static _sp<SymbolRule> keyword(initializer_list<string> keywords) {
				return symbol("keyword", equal(keywords));
			}

			/// <summary>
			/// </summary>
			/// <param name="grammer"></param>
			/// <returns></returns>
			static _sp<Rule> grammar(string grammerName) {
				switch (grammerName.back()) {
				case '*': {
					string name = grammerName.substr(0, grammerName.length() - 1);
					_sp<GrammarRule> grammarRule = make_shared<GrammarRule>(name);
					return repeat(grammarRule);
				}
				case '+': {
					string name = grammerName.substr(0, grammerName.length() - 1);
					_sp<GrammarRule> grammarRule = make_shared<GrammarRule>(name);
					return seq(grammarRule, repeat(grammarRule));
				}
				case '?': {
					string name = grammerName.substr(0, grammerName.length() - 1);
					_sp<GrammarRule> grammarRule = make_shared<GrammarRule>(name);
					return option(grammarRule);

				}
				case '-': {
					string name = grammerName.substr(0, grammerName.length() - 1);
					_sp<GrammarRule> grammarRule = make_shared<GrammarRule>(name);
					return anybut(grammarRule);
				}

				default: {
					return make_shared<GrammarRule>(grammerName);
				}

				}
			}
			static _sp<EqualCharRule> new_line() {
				return  equal({ '\n', '\r' });
			}
			static _sp<EqualCharRule> blank() {
				return  equal({ ' ', '\t', '\v', '\f' });
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