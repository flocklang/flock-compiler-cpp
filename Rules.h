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
#ifndef FLOCK_COMPILER_RULES_H
#define FLOCK_COMPILER_RULES_H

#include "Util.h"
#include "Visitor.h"
#include "IDCounter.h"
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <numeric>
#include <assert.h> 

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
namespace flock {
	namespace rule {
		using namespace std;
		namespace types {

			static IDCounter ids;
			// forwad declerations as we have cyclic dependencies on declaration.
			struct Rule;

			class TerminalRule;
			class UnaryRule;
			class CollectionRule;

			class RuleLibrary;

			template<typename IN, typename OUT>
			class RuleVisitor;

			template<typename IN, typename OUT>
			using RuleStrategy = visitor::Strategy<IN, OUT, Rule, RuleVisitor<IN, OUT>>;

			template<typename IN, typename OUT>
			using LibraryStrategy = visitor::LibraryStrategy<IN, OUT, Rule, RuleVisitor<IN, OUT>, RuleLibrary>;
			template<typename IN, typename OUT>
			using BaseStrategies = visitor::BaseStrategies<IN, OUT, Rule, RuleStrategy<IN, OUT>, LibraryStrategy<IN, OUT>>;
			template<typename IN, typename OUT>
			using WrappingStrategies = visitor::WrappingStrategies<IN, OUT, Rule, RuleStrategy<IN, OUT>, LibraryStrategy<IN, OUT>>;

			template<typename IN, typename OUT>
			using WrappingRuleStrategy = visitor::WrappingStrategy<IN, OUT, Rule, RuleVisitor<IN, OUT>>;

			template<typename IN, typename OUT>
			using WrappingLibraryStrategy = visitor::WrappingLibraryStrategy<IN, OUT, Rule, RuleVisitor<IN, OUT>, LibraryStrategy<IN, OUT>>;

			template<typename IN, typename OUT>
			using Strategies = visitor::Strategies<IN, OUT, Rule, RuleStrategy<IN, OUT>, LibraryStrategy<IN, OUT>>;

			struct LibraryAddStrategy {
				virtual _sp<Rule> addNode(_sp<visitor::Library<Rule>> library, const string symbolName, _sp <Rule> expression) {
					return library->addNode(symbolName, expression);
				}
			};

			/// <summary>
			/// All rules have a unique id on instantiation.
			/// All rules have a type to be used by the evaluator. 
			/// 
			/// Negative Type values are reserved by the API, typically they are in the form of enums.
			/// </summary>
			struct Rule {
				virtual ~Rule() = default;
				Rule(const int type) : type(type) {};

				const int type;
				const int id = ids.next();
			};

			class RuleLibrary : public visitor::Library<Rule>, public std::enable_shared_from_this<RuleLibrary> {
			public:
				RuleLibrary(_sp<LibraryAddStrategy> addStrategy) : addStrategy(addStrategy) {}
				RuleLibrary() : RuleLibrary(make_shared<LibraryAddStrategy>()) {}

				_sp <Rule> addSymbol(const string symbolName, _sp<Rule> expression) {
					return addSymbol(symbolName, addStrategy, expression);
				}
				_sp <Rule> addPart(const string partName, _sp<Rule> expression) {
					return addPart(partName, addStrategy, expression);
				}
				_sp <Rule> addSymbol(const string symbolName, _sp< LibraryAddStrategy> addStrategy, _sp<Rule> expression) {
					return addStrategy->addNode(this->shared_from_this(), symbolName, expression);
				}
				_sp <Rule> addPart(const string partName, _sp< LibraryAddStrategy> addStrategy, _sp<Rule> expression) {
					return addStrategy->addNode(parts, partName, expression);
				}

				_sp<Rule> getSymbol(const string symbolName) {
					return getNode(symbolName);
				}
				_sp<Rule> getPart(const string partName) {
					return  parts->getNode(partName);
				}
				vector<string> getSymbolNames() {
					return getNames();
				}
				vector<string> getPartNames() {
					return parts->getNames();
				}
			protected:
				// parts are usefull rules, but we are not interested in collecting information on them.
				_sp<visitor::Library<Rule>> parts = make_shared<visitor::Library<Rule>>();
				_sp<LibraryAddStrategy> addStrategy;
			};

			/// <summary>
			/// Helper class to save on the typing.
			/// </summary>
			/// <typeparam name="IN"></typeparam>
			/// <typeparam name="OUT"></typeparam>
			template<typename IN, typename OUT>
			class BaseMixinsCombined {
			public:
				virtual ~BaseMixinsCombined() = default;
				virtual bool isFailure(OUT out) = 0;
				virtual OUT makeFailure() = 0;
				virtual OUT makeSuccess(IN input) = 0;
				virtual OUT makeEmptySuccess(IN input) = 0;
				virtual bool isEnd(IN input) = 0;
			};

			template<typename IN, typename OUT, typename MIX = BaseMixinsCombined<IN, OUT>>
			class MixinsRuleStrategy : public RuleStrategy<IN, OUT> {
			public:
				MixinsRuleStrategy(const _sp<MIX> mixins) : mixins(mixins) {}
			protected:
				const _sp<MIX> mixins; //ish
			};

			/// <summary>
			/// The sole job of the visitor is to glue the aenimic rules to the strategys, whose job it is to navigate the visitor up and doen the tree.
			/// </summary>
			/// <typeparam name="IN"></typeparam>
			/// <typeparam name="OUT"></typeparam>
			template<typename IN, typename OUT>
			class RuleVisitor : public visitor::Visitor<IN, OUT, Rule, RuleLibrary, Strategies<IN, OUT>, RuleVisitor<IN, OUT>> {
			public:
				RuleVisitor(_sp<RuleLibrary> library, _sp<Strategies<IN, OUT>> strategies) : visitor::Visitor<IN, OUT, Rule, RuleLibrary, Strategies<IN, OUT>, RuleVisitor<IN, OUT>>(library, strategies) {}


				virtual OUT visitByName(const string name, const IN input) override {
					auto symRule = getSymbol(name);
					if (symRule) {
						return visitSymbol(name, symRule, input);
					}
					else {
						return visitPart(name, getPart(name), input);
					}
				}

				virtual OUT visitPart(const string name, const _sp<Rule> rule, const IN input) {
					if (rule) {
						return this->visit(rule, input);
					}
					throw string("Rule Part " + name + " does not exist");
				}
				virtual OUT visitSymbol(const string name, const _sp<Rule> rule, const IN input) {
					if (rule) {
						return this->visit(rule, input);
					}
					throw string("Rule Symbol " + name + " does not exist");
				}

				virtual _sp<Rule> getPart(const string partName) {
					return library->getPart(partName);
				}

				virtual _sp<Rule> getSymbol(const string symbolName) {
					return library->getSymbol(symbolName);
				}
			protected:
			};

			class TerminalRule : public Rule {
			public:
				TerminalRule(const int type) : Rule(type) {}
			};

			class UnaryRule : public Rule {
			public:
				UnaryRule(const int type, _sp<Rule> child) : Rule(type), child(child) {
					assert(child); // no nullptrs
				};
				_sp<Rule> getChild() {
					return child;
				}
			protected:
				_sp<Rule> child;
			};

			/// <summary>
			/// The strategies based on these rules have an explicit contract. Meaning collections > 0 and no nullptrs.
			/// 
			/// If you want to define a seperate rule that supports empty collections and nullptrs be my guest. 
			/// </summary>
			class CollectionRule : public Rule {
			public:
				CollectionRule(const int type, _sp_vec<Rule> children) : Rule(type), children(children) {
#ifndef NDEBUG
					// If NDEBUG defined, it turns of assertion anyway, but lets remove the loop as well.
					assert(!children.empty()); // empty collection is meaningless.
					for (auto child : children) {
						assert(child); // no nullptrs
					}
#endif
				}
				CollectionRule(const int type, initializer_list<_sp<Rule>> children) : CollectionRule(type, _sp_vec<Rule>(children)) {}

				_sp_vec<Rule> getChildren() {
					return children;
				}
			protected:
				_sp_vec<Rule> children;
			};

			template<typename T>
			class ValuesRule : public TerminalRule {
			public:
				ValuesRule(const int type, vector<T> values) : TerminalRule(type), values(values) {}
				ValuesRule(const int type, initializer_list<T> values) : ValuesRule(type, vector<T>(values)) {}

				vector<T> getValues() {
					return values;
				}
			protected:
				vector<T> values;
			};

			template<typename T, typename IN, typename OUT>
			class HasValueRuleStrategy : public MixinsRuleStrategy<IN, OUT> {
			public:
				HasValueRuleStrategy(_sp<BaseMixinsCombined<IN, OUT>> mixins) : MixinsRuleStrategy<IN, OUT>(mixins) {}

				virtual OUT accept(const _sp<RuleVisitor<IN, OUT>> visitor, const _sp<Rule> baseRule, const IN input) override {
					if (this->mixins->isEnd(input)) {
						return this->mixins->makeFailure();
					}
					const auto rule = std::dynamic_pointer_cast<ValuesRule<T>>(baseRule);
					vector<T> values = rule->getValues();

					for (T value : values) {
						OUT match = matches(value, input);
						if (!this->mixins->isFailure(match)) {
							return match;
						}
					}
					return this->mixins->makeFailure(); // return failure.
				}

				virtual OUT matches(T value, IN input) = 0;
			};

			static _sp<CollectionRule> _collectionRule(const int type, _sp_vec<Rule> rules) {
				return make_shared<CollectionRule>(type, rules);
			}
			static _sp<CollectionRule> _collectionRule(const int type, initializer_list<_sp<Rule>> rules) {
				return make_shared<CollectionRule>(type, rules);
			}
			static _sp<UnaryRule> _unaryRule(const int type, _sp<Rule> rule) {
				return make_shared<UnaryRule>(type, rule);
			}
			static _sp<TerminalRule> _terminalRule(const int type) {
				return make_shared<TerminalRule>(type);
			}

			template<typename T>
			static _sp<ValuesRule<T>> _valueRule(int type, vector<T> values) {
				return make_shared<ValuesRule<T>>(type, values);
			}

			template<typename T>
			static _sp<ValuesRule<T>> _valueRule(int type, initializer_list<T> values) {
				return make_shared<ValuesRule<T>>(type, values);
			}

			template<typename T>
			static _sp<ValuesRule<T>> _valueRule(int type, T value) {
				return _valueRule(type, { value });
			}

			template<typename T>
			static _sp<ValuesRule<T>> _valueRule(int type, T value1, T value2) {
				return _valueRule(type, { value1, value2 });
			}

		}

	}
}
#endif