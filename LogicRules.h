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
#ifndef FLOCK_COMPILER_LOGIC_RULES_H
#define FLOCK_COMPILER_LOGIC_RULES_H

#include "Rules.h"
#include <vector>

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
namespace flock {
	namespace rule {
		using namespace std;
		using namespace types;
		namespace logic {
			enum LogicRules {
				// Terminals
				Any = -1,
				// Terminal Values
				Equal = -2,
				// Unary
				AnyBut = -3,
				Optional = -4,
				Not = -5,
				// Specialist Unary
				Repeat = -6,
				// Collections
				Sequence = -7,
				Or = -8,
				And = -9,
				XOr = -10
			};

			/// <summary>
			/// Repeats need some extra data so we create a specialised rule in this case.
			/// </summary>
			class RepeatRule : public UnaryRule {
			public:
				RepeatRule(int min, int max, _sp<Rule> child) : UnaryRule(LogicRules::Repeat, child), min(min), max(max) {}
				RepeatRule(int amount, _sp<Rule> child) : RepeatRule(amount, amount, child) {}
				RepeatRule(_sp<Rule> child) : RepeatRule(0, 0, child) {}

				int getMin() {
					return min;
				}
				int getMax() {
					return max;
				}
			protected:
				const int min;
				const int max;
			};


			// Terminal Rules

			static _sp<Rule> Any() {
				return _terminalRule(LogicRules::Any);
			}

			// Terminal Value Rules
			template<typename T>
			static _sp<Rule> Equal(T value) {
				return _valueRule(LogicRules::Equal, value);
			}

			template<typename T>
			static _sp<Rule> Equal(vector<T> values) {
				return _valueRule(LogicRules::Equal, values);
			}

			template<typename T>
			static _sp<Rule> Equal(initializer_list<T> values) {
				return _valueRule(LogicRules::Equal, values);
			}

			// Collection Rules

			static _sp<Rule> Seq(_sp_vec<Rule> rules) {
				return _collectionRule(LogicRules::Sequence, rules);
			}
			static _sp<Rule> Seq(initializer_list<_sp<Rule>> rules) {
				return _collectionRule(LogicRules::Sequence, rules);
			}
			static _sp<Rule> And(_sp_vec<Rule> rules) {
				return _collectionRule(LogicRules::And, rules);
			}
			static _sp<Rule> And(initializer_list<_sp<Rule>> rules) {
				return _collectionRule(LogicRules::And, rules);
			}
			static _sp<Rule> Or(_sp_vec<Rule> rules) {
				return _collectionRule(LogicRules::Or, rules);
			}
			static _sp<Rule> Or(initializer_list<_sp<Rule>> rules) {
				return _collectionRule(LogicRules::Or, rules);
			}
			static _sp<Rule> XOr(_sp_vec<Rule> rules) {
				return _collectionRule(LogicRules::XOr, rules);
			}
			static _sp<Rule> XOr(initializer_list<_sp<Rule>> rules) {
				return _collectionRule(LogicRules::XOr, rules);
			}

			// Unary Rules
			static _sp<Rule> Optional(_sp<Rule> rule) {
				return _unaryRule(LogicRules::Optional, rule);
			}
			static _sp<Rule> Not(_sp<Rule> rule) {
				return _unaryRule(LogicRules::Not, rule);
			}

			/// <summary>
			/// Min = 0, Max = 0
			/// </summary>
			/// <param name="rule"></param>
			/// <returns></returns>
			static _sp<Rule> Repeat(_sp<Rule> toRepeat) {
				return make_shared<RepeatRule>(toRepeat);
			}
			static _sp<Rule> Repeat(int amount, _sp<Rule> toRepeat) {
				return make_shared<RepeatRule>( amount, toRepeat);
			}

			static _sp<Rule> Repeat(int min, int max, _sp<Rule> toRepeat) {
				return make_shared<RepeatRule>(min, max, toRepeat);
			}

			static _sp<Rule> AnyBut(_sp<Rule> rule) {
				return _unaryRule(LogicRules::AnyBut, rule);
			}

			static _sp<Rule> Until( _sp<Rule> rule) {
				return Repeat(AnyBut(rule));
			}
			static _sp<Rule> Until(const bool inclusive, _sp<Rule> rule) {
				if (inclusive) {
					return Seq({ Until(rule), rule });
				}
				return Until(rule);
			}

		}

	}
}
#endif