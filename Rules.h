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
#include "LocationSupplier.h"
#include "ConsoleFormat.h"
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

			template<typename T>
			using StringPtrMap = map<const string, _sp<T>>;

			// forwad declerations as we have cyclic dependencies on declaration.
			struct Rule;
			using RuleMap = StringPtrMap<Rule>;

			class TerminalRule;
			class UnaryRule;
			class CollectionRule;

			template<typename IN, typename OUT>
			class RuleStrategy;

			template<typename IN, typename OUT>
			class RuleVisitor;

			class Library;

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

			class Library {
			public:
				_sp <Rule> setPart(const string partName, _sp <Rule> expression) {
					parts.emplace(partName, expression);
					return expression;
				}
				_sp <Rule> setSymbol(const string symbolName, _sp <Rule> expression) {
					symbols.emplace(symbolName, expression);
					return expression;
				}

				_sp<Rule> getRule(const string name) {
					// hoping elvis operators work (apparently not standard c++, so doing it the longer way)
					auto symRule = getSymbol(name);
					return symRule ? symRule : getPart(name);
				}
				_sp<Rule> getPart(const string partName) {
					RuleMap::iterator it = parts.find(partName);
					if (it == parts.end()) {
						return nullptr;
					}
					return it->second;
				}
				_sp<Rule> getSymbol(const string symbolName) {
					RuleMap::iterator it = symbols.find(symbolName);
					if (it == symbols.end()) {
						return nullptr;
					}
					return it->second;
				}
			protected:
				RuleMap symbols;
				RuleMap parts;
			};

			template<typename IN, typename OUT>
			class Strategies {
			public:

				_sp<RuleStrategy<IN, OUT>> getStrategyById(const int typeId) {
					auto it = strategyMap.find(typeId);
					if (it == strategyMap.end()) {
						return nullptr;
					}
					return it->second;
				}

				_sp<RuleStrategy<IN, OUT>> getStrategy(_sp<Rule> rule) {
					return getStrategyById(rule->type);
				}

				void setStrategy(const int type, _sp<RuleStrategy<IN, OUT>> strategy) {
					strategyMap.emplace(type, strategy);
				}
			protected:
				map<int, _sp<RuleStrategy<IN, OUT>>> strategyMap;
			};

			template<typename IN, typename OUT>
			class RuleStrategy {
			public:
				virtual ~RuleStrategy() = default;
				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> rule, IN input) = 0;
			};

			/// <summary>
			/// Sometime we may just want to wrap a strategy, to implement common functionality, for instance history.
			/// </summary>
			/// <param name="rule"></param>
			/// <param name="input"></param>
			/// <returns></returns>
			template<typename IN, typename OUT>
			class WrappingRuleStrategy : RuleStrategy <IN, OUT> {
				WrappingRuleStrategy(_sp<RuleStrategy <IN, OUT>> wrapped) : wrapped(wrapped) {}
			public:
				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					IN newInput = before(visitor, baseRule, input);
					OUT wrappedOutput = wrapped->accept(visitor, baseRule, newInput);
					return after(visitor, baseRule, input, newInput, wrappedOutput);
				}
				/// <summary>
				/// if you don't want too do anything, simply implement by
				/// return input;
				/// </summary>
				/// <returns></returns>
				virtual IN before(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) = 0;
				/// <summary>
				/// if you don't want too do anything, simply implement by
				/// return wrappedOutput;
				/// </summary>
				/// <returns></returns>
				virtual OUT after(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN originalInput, IN wrappedInput, OUT wrappedOutput) = 0;

				_sp<RuleStrategy <IN, OUT>> getWrapped() {
					return wrapped;
				}
			protected:
				_sp<RuleStrategy <IN, OUT>> wrapped;
			};

			/// <summary>
			/// The sole job of the visitor is to glue the aenimic rules to the strategys, whose job it is to navigate the visitor up and doen the tree.
			/// </summary>
			/// <typeparam name="IN"></typeparam>
			/// <typeparam name="OUT"></typeparam>
			template<typename IN, typename OUT>
			class RuleVisitor : public std::enable_shared_from_this<RuleVisitor<IN, OUT>> {
			public:
				RuleVisitor(_sp<Library> library, _sp<Strategies<IN, OUT>> strategies) : library(library), strategies(strategies) {}

				OUT visit(_sp<Rule> rule, IN input) {
					auto strategy = strategies->getStrategy(rule);
					return strategy->accept(this->shared_from_this(), rule, input);
				}

				OUT visit(const string alias, IN input) {
					_sp<Rule> rule = getRule(alias);
					auto strategy = strategies->getStrategy(rule);
					return strategy->accept(this->shared_from_this(), rule, input);
				}

				// delegates for shorthandedness.
				_sp<Rule> getRule(const string name) {
					return library->getRule(name);
				}
				_sp<Rule> getPart(const string partName) {
					return library->getPart(partName);
				}
				_sp<Rule> getSymbol(const string symbolName) {
					return library->getSymbol(symbolName);
				}
			protected:
				_sp<Library> library;
				_sp<Strategies<IN, OUT>> strategies;
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