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
#include <string>

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
				End = -2,
				// Unary
				Not = -3,
				AnyBut = -4,
				Optional = -5,
				// Specialist Unary
				Repeat = -6,
				Alias = -7,
				// Collections
				Sequence = -8,
				Or = -9,
				And = -10,
				XOr = -11
			};

			/// <summary>
			/// Repeats need some extra data so we create a specialised rule in this case.
			/// </summary>
			class RepeatRule : public UnaryRule {
			public:
				RepeatRule(int min, int max, _sp<Rule> child) : UnaryRule(LogicRules::Repeat, child), min(min), max(max) {
					assert((min >= 0 && max >= 0) || (min > 0 && (max == 0 || min <= max)));
				}
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

			/// <summary>
			/// Provides a way of aliasing rules from the rules library.
			/// </summary>
			class AliasRule : public UnaryRule {
			public:
				AliasRule(const string alias, _sp<Rule> child) : UnaryRule(LogicRules::Alias, child), alias(alias) {}

				string getAlias() {
					return alias;
				}
			protected:
				const string alias;
			};

			/// <summary>
			/// Asks the visitor too locate the alias rule, and then visit that.
			/// Normally this would be used for actual evaluation, rather than printing the rule tree.
			/// </summary>
			/// <typeparam name="IN"></typeparam>
			/// <typeparam name="OUT"></typeparam>
			template<typename IN, typename OUT>
			class AliasRuleStrategy : public RuleStrategy <IN, OUT> {
			public:
				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto aliasRule = std::dynamic_pointer_cast<AliasRule>(baseRule);
					return visitor->visit(aliasRule->getAlias(), input);
				}
			};

		
			/// <summary>
			/// Helper class to save on the typing.
			/// </summary>
			/// <typeparam name="IN"></typeparam>
			/// <typeparam name="OUT"></typeparam>
			template<typename IN, typename OUT>
			class LogicMixinsCombined : public BaseMixinsCombined<IN,OUT>{
			public:
				virtual IN nextInFromPrevious(IN previousInput, OUT previousOutput) = 0;
			};

			template<typename IN, typename OUT>
			class LogicRuleStrategy : public MixinsRuleStrategy<IN, OUT, LogicMixinsCombined<IN,OUT>> {
			public:
				LogicRuleStrategy(const _sp<LogicMixinsCombined<IN, OUT>> mixins) : MixinsRuleStrategy<IN, OUT, LogicMixinsCombined<IN, OUT>>(mixins) {}
			};

			template<typename IN, typename OUT>
			class AndRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				AndRuleStrategy(const _sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const auto children = rule->getChildren();

					const OUT firstOut = visitor->visit(children.at(0), input);
					if (mixins->isFailure(firstOut)) {
						return mixins->makeFailure(); // return as a failure
					}
					for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
						const OUT nextOut = visitor->visit((*rule), input);
						if (mixins->isFailure(nextOut)) {
							return mixins->makeFailure(); // return as a failure
						}
					}
					return firstOut; // return the first as a success
				}
			};

			template<typename IN, typename OUT>
			class OrRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				OrRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>( mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const auto children = rule->getChildren();

					const OUT firstOut = visitor->visit(children.at(0), input);
					if (!mixins->isFailure(firstOut)) {
						return firstOut; // return as a success
					}
					for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
						const OUT nextOut = visitor->visit((*rule), input);
						if (!mixins->isFailure(nextOut)) {
							return nextOut; // return as a success
						}
					}
					return mixins->makeFailure(); // return the first as a failure
				}
			};

			template<typename IN, typename OUT>
			class XOrRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				XOrRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const auto children = rule->getChildren();

					OUT successOut = visitor->visit(children.at(0), input);

					bool isFailure = mixins->isFailure(successOut);
					for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
						const OUT nextOut = visitor->visit((*rule), input);
						if (!mixins->isFailure(nextOut)) {
							if (isFailure) {
								successOut = nextOut;
								isFailure = false;
							}
							else {
								return mixins->makeFailure(); // only one success allowed.
							}
							return nextOut; // return as a success
						}
					}
					return successOut;
				}
			};


			template<typename IN, typename OUT>
			class SeqRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				SeqRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const auto children = rule->getChildren();

					IN currentInput = input;
					OUT currentOut = visitor->visit(children.at(0), currentInput);
					if (mixins->isFailure(currentOut)) {
						return mixins->makeFailure(); // is a failure
					}
					for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
						currentInput = mixins->nextInFromPrevious(currentInput, currentOut);
						currentOut = visitor->visit((*rule), currentInput);
						if (mixins->isFailure(currentOut)) {
							return mixins->makeFailure(); // is a failure
						}
					}
					return currentOut; // return the combined as a success.
				}
			};

			template<typename IN, typename OUT>
			class OptionalRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				OptionalRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					const auto child = rule->getChild();

					const OUT currentOut = visitor->visit(child, input);
					if (mixins->isFailure(currentOut)) {
						return mixins->makeSuccess(input); // is a success
					}

					return currentOut; // return the evaluated success.
				}
			};

			template<typename IN, typename OUT>
			class NotRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				NotRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					const auto child = rule->getChild();

					const OUT currentOut = visitor->visit(child, input);
					if (mixins->isFailure(currentOut)) {
						return mixins->makeEmptySuccess(input); // is a success, but don't move forward.
					}

					return mixins->makeFailure(); // return failure.
				}
			};

			template<typename IN, typename OUT>
			class RepeatRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				RepeatRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<RepeatRule>(baseRule);
					const auto child = rule->getChild();
					const int min = rule->getMin();
					const int max = rule->getMax();

					IN currentIn = input;
					OUT currentOut = visitor->visit(child, currentIn);
					if (mixins->isFailure(currentOut) && min > 0) {
						return mixins->makeFailure();
					}

					for (int i = 1; i < min; i++) {
						currentIn = mixins->nextInFromPrevious(currentIn, currentOut);
						currentOut = visitor->visit(child, currentIn);
						if (mixins->isFailure(currentOut)) {
							return mixins->makeFailure();
						}
					}
					if (max > 0) {
						for (int i = min; i < max + 1; i++) {
							currentIn = mixins->nextInFromPrevious(currentIn, currentOut);
							const OUT nextOut = visitor->visit(child, currentIn);

							if (mixins->isFailure(nextOut)) {
								return currentOut;
							}
							currentOut = nextOut;
						}
						// we have gone past the maximum
						return mixins->makeFailure();
					}
					else {
						while (true) {
							currentIn = mixins->nextInFromPrevious(currentIn, currentOut);
							const OUT nextOut = visitor->visit(child, currentIn);
							if (mixins->isFailure(nextOut)) {
								return currentOut;
							}
							currentOut = nextOut;
						}
					}
				}
			};


			template<typename IN, typename OUT>
			class AnyRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				AnyRuleStrategy(const _sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> rule, IN input) override {
					if (mixins->isEnd(input)) {
						return mixins->makeFailure();
					}
					return mixins->makeSuccess(input);
				}
			};



			template<typename IN, typename OUT>
			class EndRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				EndRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> rule, IN input) override {
					if (mixins->isEnd(input)) {
						return mixins->makeEmptySuccess(input); // success but no point moving forwards.
					}
					return mixins->makeFailure();
				}
			};


			template<typename IN, typename OUT>
			class AnyButRuleStrategy : public LogicRuleStrategy<IN, OUT> {
			public:
				AnyButRuleStrategy(_sp<LogicMixinsCombined<IN,OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					const auto child = rule->getChild();

					const OUT currentOut = visitor->visit(child, input);
					if (mixins->isFailure(currentOut)) {
						if (mixins->isEnd(input)) {
							return mixins->makeFailure();
						}
						return mixins->makeSuccess(input); // is a success
					}

					return mixins->makeFailure(); // return failure.
				}
			};
			
			template<typename IN, typename OUT>
			static void addLogicStrategies(_sp<LogicMixinsCombined<IN, OUT>> mixins, _sp<Strategies<IN,OUT>> strategies) {

				strategies->setStrategy(LogicRules::Any, make_shared<AnyRuleStrategy<IN, OUT>>(mixins));
				strategies->setStrategy(LogicRules::End, make_shared<EndRuleStrategy<IN, OUT>>(mixins));

				strategies->setStrategy(LogicRules::Not, make_shared<NotRuleStrategy<IN, OUT>>(mixins));
				strategies->setStrategy(LogicRules::AnyBut, make_shared<AnyButRuleStrategy<IN, OUT>>(mixins));
				strategies->setStrategy(LogicRules::Optional, make_shared<OptionalRuleStrategy<IN, OUT>>(mixins));

				strategies->setStrategy(LogicRules::Repeat, make_shared<RepeatRuleStrategy<IN, OUT>>(mixins));
				strategies->setStrategy(LogicRules::Alias, make_shared<AliasRuleStrategy<IN, OUT>>());

				strategies->setStrategy(LogicRules::Sequence, make_shared<SeqRuleStrategy<IN, OUT>>(mixins));
				strategies->setStrategy(LogicRules::Or, make_shared<OrRuleStrategy<IN, OUT>>(mixins));
				strategies->setStrategy(LogicRules::And, make_shared<AndRuleStrategy<IN, OUT>>(mixins));
				strategies->setStrategy(LogicRules::XOr, make_shared<XOrRuleStrategy<IN, OUT>>(mixins));
			}

			// Terminal Rule

			static _sp<Rule> r_Any() {
				return _terminalRule(LogicRules::Any);
			}

			static _sp<Rule> r_End() {
				return _terminalRule(LogicRules::End);
			}

			// Collection Rules

			static _sp<Rule> r_Seq(_sp_vec<Rule> rules) {
				return _collectionRule(LogicRules::Sequence, rules);
			}
			static _sp<Rule> r_Seq(initializer_list<_sp<Rule>> rules) {
				return _collectionRule(LogicRules::Sequence, rules);
			}
			static _sp<Rule> r_And(_sp_vec<Rule> rules) {
				return _collectionRule(LogicRules::And, rules);
			}
			static _sp<Rule> r_And(initializer_list<_sp<Rule>> rules) {
				return _collectionRule(LogicRules::And, rules);
			}
			static _sp<Rule> r_Or(_sp_vec<Rule> rules) {
				return _collectionRule(LogicRules::Or, rules);
			}
			static _sp<Rule> r_Or(initializer_list<_sp<Rule>> rules) {
				return _collectionRule(LogicRules::Or, rules);
			}
			static _sp<Rule> r_XOr(_sp_vec<Rule> rules) {
				return _collectionRule(LogicRules::XOr, rules);
			}
			static _sp<Rule> r_XOr(initializer_list<_sp<Rule>> rules) {
				return _collectionRule(LogicRules::XOr, rules);
			}

			// Unary Rules
			static _sp<Rule> r_Optional(_sp<Rule> rule) {
				return _unaryRule(LogicRules::Optional, rule);
			}
			static _sp<Rule> r_Not(_sp<Rule> rule) {
				return _unaryRule(LogicRules::Not, rule);
			}


			static _sp<Rule> r_Repeat(_sp<Rule> toRepeat) {
				return make_shared<RepeatRule>(toRepeat);
			}
			static _sp<Rule> r_Repeat(const int amount, _sp<Rule> toRepeat) {
				return make_shared<RepeatRule>(amount, toRepeat);
			}
			static _sp<Rule> r_Repeat(const int min, const int max, _sp<Rule> toRepeat) {
				return make_shared<RepeatRule>(min, max, toRepeat);
			}
			static _sp<Rule> r_Alias(const string alias, _sp<Rule> rule) {
				return make_shared<AliasRule>(alias, rule);
			}

			static _sp<Rule> r_AnyBut(_sp<Rule> rule) {
				return _unaryRule(LogicRules::AnyBut, rule);
			}

			static _sp<Rule> r_Until(_sp<Rule> rule) {
				return r_Repeat(r_AnyBut(rule));
			}
			static _sp<Rule> r_Until(const bool inclusive, _sp<Rule> rule) {
				if (inclusive) {
					return r_Seq({ r_Until(rule), rule });
				}
				return r_Until(rule);
			}

		}

	}
}
#endif