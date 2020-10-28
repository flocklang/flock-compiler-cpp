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
		class AliasRule : public TerminalRule {
		public:
			AliasRule(const string alias) : TerminalRule(LogicRules::Alias), alias(alias) {}

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
				return visitor->visitByName(aliasRule->getAlias(), input);
			}
		};


		/// <summary>
		/// Helper class to save on the typing.
		/// </summary>
		/// <typeparam name="IN"></typeparam>
		/// <typeparam name="OUT"></typeparam>
		template<typename IN, typename OUT>
		class LogicMixinsCombined : public BaseMixinsCombined<IN, OUT> {
		public:
			virtual IN nextInFromPrevious(IN previousInput, OUT previousOutput) = 0;
		};

		template<typename IN, typename OUT>
		class LogicRuleStrategy : public MixinsRuleStrategy<IN, OUT, LogicMixinsCombined<IN, OUT>> {
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
				if (this->mixins->isFailure(firstOut)) {
					return this->mixins->makeFailure(); // return as a failure
				}
				for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
					const OUT nextOut = visitor->visit((*rule), input);
					if (this->mixins->isFailure(nextOut)) {
						return this->mixins->makeFailure(); // return as a failure
					}
				}
				return firstOut; // return the first as a success
			}
		};

		template<typename IN, typename OUT>
		class OrRuleStrategy : public LogicRuleStrategy<IN, OUT> {
		public:
			OrRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

			virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
				const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
				const auto children = rule->getChildren();

				const OUT firstOut = visitor->visit(children.at(0), input);
				if (!this->mixins->isFailure(firstOut)) {
					return firstOut; // return as a success
				}
				for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
					const OUT nextOut = visitor->visit((*rule), input);
					if (!this->mixins->isFailure(nextOut)) {
						return nextOut; // return as a success
					}
				}
				return this->mixins->makeFailure(); // return the first as a failure
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

				bool isFailure = this->mixins->isFailure(successOut);
				for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
					const OUT nextOut = visitor->visit((*rule), input);
					if (!this->mixins->isFailure(nextOut)) {
						if (isFailure) {
							successOut = nextOut;
							isFailure = false;
						}
						else {
							return this->mixins->makeFailure(); // only one success allowed.
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
				if (this->mixins->isFailure(currentOut)) {
					return this->mixins->makeFailure(); // is a failure
				}
				for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
					currentInput = this->mixins->nextInFromPrevious(currentInput, currentOut);
					currentOut = visitor->visit((*rule), currentInput);
					if (this->mixins->isFailure(currentOut)) {
						return this->mixins->makeFailure(); // is a failure
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
				if (this->mixins->isFailure(currentOut)) {
					return this->mixins->makeEmptySuccess(input); // is a success
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
				if (this->mixins->isFailure(currentOut)) {
					return this->mixins->makeEmptySuccess(input); // is a success, but don't move forward.
				}

				return this->mixins->makeFailure(); // return failure.
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
				if (this->mixins->isFailure(currentOut)) {
					if (min > 0) {
						return this->mixins->makeFailure();
					}
					else {
						return this->mixins->makeEmptySuccess(input);
					}
				}

				for (int i = 1; i < min; i++) {
					currentIn = this->mixins->nextInFromPrevious(currentIn, currentOut);
					currentOut = visitor->visit(child, currentIn);
					if (this->mixins->isFailure(currentOut)) {
						return this->mixins->makeFailure();
					}
				}
				if (max > 0) {
					for (int i = min; i < max + 1; i++) {
						currentIn = this->mixins->nextInFromPrevious(currentIn, currentOut);
						const OUT nextOut = visitor->visit(child, currentIn);

						if (this->mixins->isFailure(nextOut)) {
							return currentOut;
						}
						currentOut = nextOut;
					}
					// we have gone past the maximum
					return this->mixins->makeFailure();
				}
				else {
					while (true) {
						currentIn = this->mixins->nextInFromPrevious(currentIn, currentOut);
						const OUT nextOut = visitor->visit(child, currentIn);
						if (this->mixins->isFailure(nextOut)) {
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
				if (this->mixins->isEnd(input)) {
					return this->mixins->makeFailure();
				}
				return this->mixins->makeSuccess(input);
			}
		};



		template<typename IN, typename OUT>
		class EndRuleStrategy : public LogicRuleStrategy<IN, OUT> {
		public:
			EndRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

			virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> rule, IN input) override {
				if (this->mixins->isEnd(input)) {
					return this->mixins->makeEmptySuccess(input); // success but no point moving forwards.
				}
				return this->mixins->makeFailure();
			}
		};


		template<typename IN, typename OUT>
		class AnyButRuleStrategy : public LogicRuleStrategy<IN, OUT> {
		public:
			AnyButRuleStrategy(_sp<LogicMixinsCombined<IN, OUT>> mixins) : LogicRuleStrategy<IN, OUT>(mixins) {}

			virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
				const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
				const auto child = rule->getChild();

				const OUT currentOut = visitor->visit(child, input);
				if (this->mixins->isFailure(currentOut)) {
					if (this->mixins->isEnd(input)) {
						return this->mixins->makeFailure();
					}
					return this->mixins->makeSuccess(input); // is a success
				}

				return this->mixins->makeFailure(); // return failure.
			}
		};

		template<typename IN, typename OUT>
		static void addLogicStrategies(_sp<LogicMixinsCombined<IN, OUT>> mixins, _sp<Strategies<IN, OUT>> strategies) {

			strategies->addStrategy(LogicRules::Any, make_shared<AnyRuleStrategy<IN, OUT>>(mixins));
			strategies->addStrategy(LogicRules::End, make_shared<EndRuleStrategy<IN, OUT>>(mixins));

			strategies->addStrategy(LogicRules::Not, make_shared<NotRuleStrategy<IN, OUT>>(mixins));
			strategies->addStrategy(LogicRules::AnyBut, make_shared<AnyButRuleStrategy<IN, OUT>>(mixins));
			strategies->addStrategy(LogicRules::Optional, make_shared<OptionalRuleStrategy<IN, OUT>>(mixins));

			strategies->addStrategy(LogicRules::Repeat, make_shared<RepeatRuleStrategy<IN, OUT>>(mixins));
			strategies->addStrategy(LogicRules::Alias, make_shared<AliasRuleStrategy<IN, OUT>>());

			strategies->addStrategy(LogicRules::Sequence, make_shared<SeqRuleStrategy<IN, OUT>>(mixins));
			strategies->addStrategy(LogicRules::Or, make_shared<OrRuleStrategy<IN, OUT>>(mixins));
			strategies->addStrategy(LogicRules::And, make_shared<AndRuleStrategy<IN, OUT>>(mixins));
			strategies->addStrategy(LogicRules::XOr, make_shared<XOrRuleStrategy<IN, OUT>>(mixins));
		}

		// Terminal Rule

		static _sp<Rule> ANY() {
			return _terminalRule(LogicRules::Any);
		}

		static _sp<Rule> END() {
			return _terminalRule(LogicRules::End);
		}

		// Collection Rules

		static _sp<Rule> SEQ(_sp_vec<Rule> rules) {
			return _collectionRule(LogicRules::Sequence, rules);
		}
		static _sp<Rule> SEQ(initializer_list<_sp<Rule>> rules) {
			return _collectionRule(LogicRules::Sequence, rules);
		}
		static _sp<Rule> SEQ(_sp<Rule> ruleOne, _sp<Rule> ruleTwo) {
			return SEQ({ ruleOne,ruleTwo });
		}
		static _sp<Rule> AND(_sp_vec<Rule> rules) {
			return _collectionRule(LogicRules::And, rules);
		}
		static _sp<Rule> AND(initializer_list<_sp<Rule>> rules) {
			return _collectionRule(LogicRules::And, rules);
		}
		static _sp<Rule> AND(_sp<Rule> ruleOne, _sp<Rule> ruleTwo) {
			return AND({ ruleOne,ruleTwo });
		}
		static _sp<Rule> OR(_sp_vec<Rule> rules) {
			return _collectionRule(LogicRules::Or, rules);
		}
		static _sp<Rule> OR(initializer_list<_sp<Rule>> rules) {
			return _collectionRule(LogicRules::Or, rules);
		}
		static _sp<Rule> OR(_sp<Rule> ruleOne, _sp<Rule> ruleTwo) {
			return OR({ ruleOne,ruleTwo });
		}
		static _sp<Rule> XOR(_sp_vec<Rule> rules) {
			return _collectionRule(LogicRules::XOr, rules);
		}
		static _sp<Rule> XOR(initializer_list<_sp<Rule>> rules) {
			return _collectionRule(LogicRules::XOr, rules);
		}
		static _sp<Rule> XOR(_sp<Rule> ruleOne, _sp<Rule> ruleTwo) {
			return XOR({ ruleOne, ruleTwo });
		}

		// Unary Rules
		static _sp<Rule> OPT(_sp<Rule> rule) {
			return _unaryRule(LogicRules::Optional, rule);
		}
		static _sp<Rule> OPT(initializer_list<_sp<Rule>> rules) {
			return OPT(SEQ(rules));
		}
		static _sp<Rule> NOT(_sp<Rule> rule) {
			return _unaryRule(LogicRules::Not, rule);
		}
		static _sp<Rule> NOT(initializer_list<_sp<Rule>> rules) {
			return NOT(SEQ(rules));
		}


		static _sp<Rule> REP(_sp<Rule> toRepeat) {
			return make_shared<RepeatRule>(toRepeat);
		}
		static _sp<Rule> REP(initializer_list<_sp<Rule>> rules) {
			return REP(SEQ(rules));
		}
		static _sp<Rule> REP(const int amount, _sp<Rule> toRepeat) {
			return make_shared<RepeatRule>(amount, toRepeat);
		}
		static _sp<Rule> REP(const int amount, initializer_list<_sp<Rule>> rules) {
			return REP(amount, SEQ(rules));
		}
		static _sp<Rule> REP(const int min, const int max, _sp<Rule> toRepeat) {
			return make_shared<RepeatRule>(min, max, toRepeat);
		}
		static _sp<Rule> REP(const int min, const int max, initializer_list<_sp<Rule>> rules) {
			return REP(min, max, SEQ(rules));
		}

		static _sp<Rule> BUT(_sp<Rule> rule) {
			return _unaryRule(LogicRules::AnyBut, rule);
		}
		static _sp<Rule> BUT(initializer_list<_sp<Rule>> rules) {
			return BUT(SEQ(rules));
		}
		static _sp<Rule> RULE(const string alias) {
			switch (alias.back()) {
			case '*': {
				string name = alias.substr(0, alias.length() - 1);
				_sp<Rule> grammarRule = make_shared<AliasRule>(name);
				return REP(grammarRule);
			}
			case '+': {
				string name = alias.substr(0, alias.length() - 1);
				_sp<Rule> grammarRule = make_shared<AliasRule>(name);
				return SEQ(grammarRule, REP(grammarRule));
			}
			case '?': {
				string name = alias.substr(0, alias.length() - 1);
				_sp<Rule> grammarRule = make_shared<AliasRule>(name);
				return OPT(grammarRule);

			}
			case '-': {
				string name = alias.substr(0, alias.length() - 1);
				_sp<Rule> grammarRule = make_shared<AliasRule>(name);
				return BUT(grammarRule);
			}

			default: {
				return make_shared<AliasRule>(alias);
			}

			}
		}

		static _sp<Rule> UNTIL(_sp<Rule> rule) {
			return REP(BUT(rule));
		}
		static _sp<Rule> UNTIL(initializer_list<_sp<Rule>> rules) {
			return UNTIL(SEQ(rules));
		}
		static _sp<Rule> UNTIL(const bool inclusive, _sp<Rule> rule) {
			if (inclusive) {
				return SEQ({ UNTIL(rule), rule });
			}
			return UNTIL(rule);
		}
		static _sp<Rule> UNTIL(const bool inclusive, initializer_list<_sp<Rule>> rules) {
			return UNTIL(inclusive, SEQ(rules));
		}

	}
}
#endif