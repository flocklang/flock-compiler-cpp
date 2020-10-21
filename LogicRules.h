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
				// Unary
				Not = -2,
				AnyBut = -3,
				Optional = -4,
				// Specialist Unary
				Repeat = -5,
				Alias = -6,
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
			class AliasPassThroughRuleStrategy : RuleStrategy <IN, OUT> {
			public:
				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto aliasRule = std::dynamic_pointer_cast<AliasRule>(baseRule);
					return visitor->visit(aliasRule->getAlias(), input);
				}
			};

			/// <summary>
			/// Implement as a mixin, and then use multiple inheritence to save time.
			/// </summary>
			/// <typeparam name="OUT"></typeparam>
			template<typename OUT>
			class HasFailureStrategyHelper {
			public:
				virtual bool isFailure(OUT out) = 0;
				virtual OUT makeFailure() = 0;
			};

			/// <summary>
			/// Implement as a mixin, and then use multiple inheritence to save time.
			/// </summary>
			/// <typeparam name="IN"></typeparam>
			/// <typeparam name="OUT"></typeparam>
			template<typename IN, typename OUT>
			class SequentialInputStrategyHelper {
			public:
				virtual IN nextInFromPrevious(IN previousInput, OUT previousOutput) = 0;
			};

			/// <summary>
			/// Implement as a mixin, and then use multiple inheritence to save time.
			/// </summary>
			/// <typeparam name="IN"></typeparam>
			/// <typeparam name="OUT"></typeparam>
			template<typename IN, typename OUT>
			class EmptySuccessStrategyHelper {
			public:
				virtual OUT makeSuccess(IN input) = 0;
			};

			/// <summary>
			/// Implement as a mixin, and then use multiple inheritence to save time.
			/// </summary>
			/// <typeparam name="IN"></typeparam>
			template<typename IN>
			class EndCheckStrategyHelper {
			public:
				virtual bool isEnd(IN input) = 0;
			};

			template<typename IN, typename OUT>
			class AndLogicRuleStrategy : public HasFailureStrategyHelper <OUT>, public RuleStrategy <IN, OUT> {
			public:
				virtual bool isFailure(OUT out) override = 0;
				virtual OUT makeFailure() override = 0;

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const auto children = rule->getChildren();

					const OUT firstOut = visitor->visit(children.at(0), input);
					if (this->isFailure(firstOut)) {
						return this->makeFailure(); // return as a failure
					}
					for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
						const OUT nextOut = visitor->visit((*rule), input);
						if (this->isFailure(nextOut)) {
							return this->makeFailure(); // return as a failure
						}
					}
					return firstOut; // return the first as a success
				}
			};

			template<typename IN, typename OUT>
			class OrLogicRuleStrategy : public HasFailureStrategyHelper <OUT>, public RuleStrategy <IN, OUT> {
			public:
				virtual bool isFailure(OUT out) override = 0;
				virtual OUT makeFailure() override = 0;

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const auto children = rule->getChildren();

					const OUT firstOut = visitor->visit(children.at(0), input);
					if (!this->isFailure(firstOut)) {
						return firstOut; // return as a success
					}
					for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
						const OUT nextOut = visitor->visit((*rule), input);
						if (!this->isFailure(nextOut)) {
							return nextOut; // return as a success
						}
					}
					return this->makeFailure(); // return the first as a failure
				}
			};

			template<typename IN, typename OUT>
			class XOrLogicRuleStrategy : public HasFailureStrategyHelper <OUT>, public RuleStrategy <IN, OUT> {
			public:
				virtual bool isFailure(OUT out) override = 0;
				virtual OUT makeFailure() override = 0;

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const auto children = rule->getChildren();

					OUT successOut = visitor->visit(children.at(0), input);

					bool isFailure = this->isFailure(successOut));
					for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
						const OUT nextOut = visitor->visit((*rule), input);
						if (!this->isFailure(nextOut)) {
							if (isFailure) {
								successOut = nextOut;
								isSuccess = true;
							}
							else {
								return this->makeFailure(); // only one success allowed.
							}
							return nextOut; // return as a success
						}
					}
					return successOut;
				}
			};


			template<typename IN, typename OUT>
			class SeqLogicRuleStrategy : public HasFailureStrategyHelper <OUT>, public SequentialInputStrategyHelper<IN, OUT>, public RuleStrategy <IN, OUT> {
			public:
				virtual bool isFailure(OUT out) override = 0;
				virtual OUT makeFailure() override = 0;
				virtual IN nextInFromPrevious(IN previousInput, OUT previousOutput) override = 0;

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const auto children = rule->getChildren();

					IN currentInput = input;
					OUT currentOut = visitor->visit(children.at(0), currentInput);
					if (this->isFailure(currentOut)) {
						return this->makeFailure(); // is a failure
					}
					for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
						currentInput = this->nextInFromPrevious(currentInput, currentOut);
						currentOut = visitor->visit((*rule), currentInput);
						if (this->isFailure(currentOut)) {
							return this->makeFailure(); // is a failure
						}
					}
					return currentOut; // return the combined as a success.
				}
			};

			template<typename IN, typename OUT>
			class OptionalLogicRuleStrategy : public HasFailureStrategyHelper <OUT>, public EmptySuccessStrategyHelper<IN, OUT>, public RuleStrategy <IN, OUT> {
			public:
				virtual bool isFailure(OUT out) override = 0;
				virtual OUT makeSuccess(IN input) override = 0;

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					const auto child = rule->getChild();

					const OUT currentOut = visitor->visit(child, input);
					if (this->isFailure(currentOut)) {
						return this->makeSuccess(input); // is a success
					}

					return currentOut; // return the evaluated success.
				}
			};

			template<typename IN, typename OUT>
			class NotLogicRuleStrategy : public HasFailureStrategyHelper <OUT>, public EmptySuccessStrategyHelper<IN, OUT>, public RuleStrategy <IN, OUT> {
			public:
				virtual bool isFailure(OUT out) override = 0;
				virtual OUT makeFailure() override = 0;
				virtual OUT makeSuccess(IN input) override = 0;

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					const auto child = rule->getChild();

					const OUT currentOut = visitor->visit(child, input);
					if (this->isFailure(currentOut)) {
						return this->makeSuccess(input); // is a success
					}

					return this->makeFailure(); // return failure.
				}
			};

			template<typename IN, typename OUT>
			class RepeatLogicRuleStrategy :
				public SequentialInputStrategyHelper<IN, OUT>,
				public HasFailureStrategyHelper <OUT>,
				public RuleStrategy<IN, OUT> {
			public:
				virtual bool isFailure(OUT out) override = 0;
				virtual OUT makeFailure() override = 0;
				virtual IN nextInFromPrevious(IN previousInput, OUT previousOutput) override = 0;

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<RepeatRule>(baseRule);
					const auto child = rule->getChild();
					const int min = rule->getMin();
					const int max = rule->getMax();

					IN currentIn = input;
					OUT currentOut = visitor->visit(child, currentIn);
					if (this->isFailure(currentOut) && min > 0) {
						return this->makeFailure();
					}

					for (int i = 1; i < min; i++) {
						currentIn = this->nextInFromPrevious(currentIn, currentOut);
						currentOut = visitor->visit(child, currentIn);
						if (this->isFailure(currentOut)) {
							return this->makeFailure();
						}
					}
					if (max > 0) {
						for (int i = min; i < max + 1; i++) {
							currentIn = this->nextInFromPrevious(currentIn, currentOut);
							const OUT nextOut = visitor->visit(child, currentIn);

							if (this->isFailure(nextOut)) {
								return currentOut;
							}
							currentOut = nextOut;
						}
						// we have gone past the maximum
						return this->makeFailure();
					}
					else {
						while (true) {
							currentIn = this->nextInFromPrevious(currentIn, currentOut);
							const OUT nextOut = visitor->visit(child, currentIn);
							if (this->isFailure(nextOut)) {
								return currentOut;
							}
							currentOut = nextOut;
						}
					}
				}
			};


			template<typename IN, typename OUT>
			class AnyLogicRuleStrategy :
				public EmptySuccessStrategyHelper<IN, OUT>,
				public HasFailureStrategyHelper<OUT>,
				public EndCheckStrategyHelper<IN>,
				public RuleStrategy<IN, OUT> {
			public:
				virtual OUT makeFailure() override = 0;
				virtual OUT makeSuccess(IN input) override = 0;
				virtual bool isEnd(IN input) override = 0;

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> rule, IN input) override {
					if (this->isEnd(input)) {
						return this->makeFailure();
					}
					return this->makeSuccess(input);
				}
			};


			template<typename IN, typename OUT>
			class AnyButLogicRuleStrategy :
				public HasFailureStrategyHelper <OUT>,
				public EmptySuccessStrategyHelper<IN, OUT>,
				public EndCheckStrategyHelper<IN>,
				public RuleStrategy <IN, OUT> {
			public:
				virtual bool isFailure(OUT out) override = 0;
				virtual OUT makeFailure() override = 0;
				virtual OUT makeSuccess(IN input) override = 0;
				virtual bool isEnd(IN input) override = 0;

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					const auto child = rule->getChild();

					const OUT currentOut = visitor->visit(child, input);
					if (this->isFailure(currentOut)) {
						if (this->isEnd(input)) {
							return this->makeFailure();
						}
						return this->makeSuccess(input); // is a success
					}

					return this->makeFailure(); // return failure.
				}
			};


			// Terminal Rule

			static _sp<Rule> Any() {
				return _terminalRule(LogicRules::Any);
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


			static _sp<Rule> Repeat(_sp<Rule> toRepeat) {
				return make_shared<RepeatRule>(toRepeat);
			}
			static _sp<Rule> Repeat(const int amount, _sp<Rule> toRepeat) {
				return make_shared<RepeatRule>(amount, toRepeat);
			}
			static _sp<Rule> Repeat(const int min, const int max, _sp<Rule> toRepeat) {
				return make_shared<RepeatRule>(min, max, toRepeat);
			}
			static _sp<Rule> Alias(const string alias, _sp<Rule> rule) {
				return make_shared<AliasRule>(alias, rule);
			}

			static _sp<Rule> AnyBut(_sp<Rule> rule) {
				return _unaryRule(LogicRules::AnyBut, rule);
			}

			static _sp<Rule> Until(_sp<Rule> rule) {
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