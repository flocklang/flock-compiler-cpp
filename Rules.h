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
			class LibraryStrategy;

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
				_sp <Rule> addPart(const string partName, _sp <Rule> expression) {
					parts.emplace(partName, expression);
					partNames.push_back(partName);
					return expression;
				}

				_sp <Rule> addSymbol(const string symbolName, _sp <Rule> expression) {
					symbols.emplace(symbolName, expression);
					symbolNames.push_back(symbolName);
					return expression;
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
				vector<string> getSymbolNames() {
					return symbolNames;
				}
				vector<string> getPartNames() {
					return partNames;
				}
			protected:
				vector<string> symbolNames;
				vector<string> partNames;
				RuleMap symbols;
				RuleMap parts;
			};

			template<typename IN, typename OUT>
			class Strategies {
			public:
				virtual ~Strategies() = default;
				virtual _sp<RuleStrategy<IN, OUT>> getRuleStrategyById(const int typeId) = 0;
				virtual _sp<RuleStrategy<IN, OUT>> getRuleStrategy(_sp<Rule> rule) {
					return getRuleStrategyById(rule->type);
				}
				virtual void addRuleStrategy(const int type, _sp<RuleStrategy<IN, OUT>> strategy) = 0;
				virtual _sp<LibraryStrategy<IN, OUT>> getLibraryStrategy() = 0;
				virtual void setLibraryStrategy(_sp<LibraryStrategy<IN, OUT>> strategy) = 0;
				virtual void clear() {
				}
			};

			template<typename IN, typename OUT>
			class LibraryStrategies : public Strategies<IN,OUT> {
			public:
				LibraryStrategies() {}
				virtual _sp<RuleStrategy<IN, OUT>> getRuleStrategyById(const int typeId) override {
					auto it = strategyMap.find(typeId);
					if (it == strategyMap.end()) {
						return nullptr;
					}
					return it->second;
				}

				virtual void addRuleStrategy(const int type, _sp<RuleStrategy<IN, OUT>> strategy)  override {
					strategyMap.emplace(type, strategy);
				}

				virtual _sp<LibraryStrategy<IN, OUT>> getLibraryStrategy()  override {
					return libraryStrategy;
				}

				virtual void setLibraryStrategy(_sp<LibraryStrategy<IN, OUT>> strategy) override {
					libraryStrategy = strategy;
				}
			protected:
				map<int, _sp<RuleStrategy<IN, OUT>>> strategyMap;
				_sp<LibraryStrategy<IN, OUT>> libraryStrategy;
			};

			template<typename IN, typename OUT>
			class WrappingStrategies : public Strategies<IN, OUT> {
			public:
				WrappingStrategies(_sp<Strategies<IN, OUT>> strategies) : strategies(strategies) {}

				virtual _sp<RuleStrategy<IN, OUT>> getRuleStrategyById(const int typeId) override {
					return strategies->getRuleStrategyById(typeId);
				}

				virtual _sp<RuleStrategy<IN, OUT>> getRuleStrategy(_sp<Rule> rule) {
					return strategies->getRuleStrategy(rule);
				}

				virtual void addRuleStrategy(const int type, _sp<RuleStrategy<IN, OUT>> strategy)  override {
					strategies->addRuleStrategy(type, strategy);
				}

				virtual _sp<LibraryStrategy<IN, OUT>> getLibraryStrategy()  override {
					return strategies->getLibraryStrategy();
				}

				virtual void setLibraryStrategy(_sp<LibraryStrategy<IN, OUT>> strategy) override {
					strategies->setLibraryStrategy(strategy);
				}

				virtual _sp<Strategies<IN, OUT>> getWrappedStratagies() {
					return strategies;
				}
				virtual void clear() override{
					strategies->clear();
				}
			protected:
				_sp<Strategies<IN, OUT>> strategies;
			};

			template<typename IN, typename OUT>
			class RuleStrategy {
			public:
				virtual ~RuleStrategy() = default;
				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> rule, IN input) = 0;
			};

			template<typename IN, typename OUT>
			class LibraryStrategy {
			public:
				virtual ~LibraryStrategy() = default;
				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Library> library, IN input) = 0;
				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Library> library) = 0;
			};

			/// <summary>
			/// Sometime we may just want to wrap a strategy, to implement common functionality, for instance history.
			/// </summary>
			/// <param name="wrapped"></param>
			/// <returns></returns>
			template<typename IN, typename OUT>
			class WrappingRuleStrategy : public RuleStrategy <IN, OUT> {
			public:
				WrappingRuleStrategy(_sp<RuleStrategy <IN, OUT>> wrapped) : wrapped(wrapped) {}
				
				_sp<RuleStrategy <IN, OUT>> getWrapped() {
					return wrapped;
				}
			protected:
				_sp<RuleStrategy <IN, OUT>> wrapped;
			};
			
			/// <summary>
			/// Sometime we may just want to wrap a strategy, to implement common functionality, for instance history.
			/// </summary>
			/// <param name="wrapped"></param>
			/// <returns></returns>
			template<typename IN, typename OUT>
			class WrappingLibraryStrategy : public LibraryStrategy <IN, OUT> {
			public:
				WrappingLibraryStrategy(_sp<LibraryStrategy <IN, OUT>> wrapped) : wrapped(wrapped) {}

				_sp<LibraryStrategy <IN, OUT>> getWrapped() {
					return wrapped;
				}
			protected:
				_sp<LibraryStrategy <IN, OUT>> wrapped;
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

			template<typename IN, typename OUT, typename MIX = BaseMixinsCombined<IN,OUT>>
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
			class RuleVisitor : public std::enable_shared_from_this<RuleVisitor<IN, OUT>> {
			public:
				RuleVisitor(_sp<Library> library, _sp<Strategies<IN, OUT>> strategies) : library(library), strategies(strategies) {}

				virtual OUT visit(_sp<Rule> rule, IN input) {
					auto strategy = strategies->getRuleStrategy(rule);
					return strategy->accept(this->shared_from_this(), rule, input);
				}

				virtual OUT visit(const string name, IN input) {
					auto symRule = getSymbol(name);
					if (symRule) {
						return visitSymbol(name, symRule, input);
					}
					else {
						return visitPart(name, getPart(name), input);
					}
				}

				virtual OUT visitPart(string name, _sp<Rule> rule, IN input) {
					return this->visit(rule, input);
				}
				virtual OUT visitSymbol(string name, _sp<Rule> rule, IN input) {
					return this->visit(rule, input);
				}

				virtual OUT begin(IN input) {
					auto strategy = strategies->getLibraryStrategy();
					return strategy->accept(this->shared_from_this(), library, input);
				}

				virtual OUT begin() {
					auto strategy = strategies->getLibraryStrategy();
					return strategy->accept(this->shared_from_this(), library);
				}

				virtual _sp<Rule> getPart(const string partName) {
					return library->getPart(partName);
				}
				virtual _sp<Rule> getSymbol(const string symbolName) {
					return library->getSymbol(symbolName);
				}

				virtual void clear() {
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
						if(!this->mixins->isFailure(match)) {
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