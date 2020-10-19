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

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
namespace flock {
	namespace rule {
		using namespace std;


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
		class RulesEvaluator;
		
		template<typename IN, typename OUT>
		struct RuleVisitor;

		class Library;


		struct Rule {
			virtual ~Rule() = default;
			Rule(const int type) : type(type) {};
			const int type;
			const int id = ids.next();
		};

		class Library {
		public:
			_sp <Rule> part(const string partName, _sp <Rule> expression) {
				parts.emplace(partName, expression);
				return expression;
			}
			_sp <Rule> symbol(const string symbolName, _sp <Rule> expression) {
				symbols.emplace(symbolName, expression);
				return expression;
			}

			_sp<Rule> rule(const string name) {
				// hoping elvis operators work (apparently not standard c++, so doing it the longer way)
				auto symRule = symbol(name);
				return symRule ? symRule : part(name);
			}
			_sp<Rule> part(const string partName) {
				RuleMap::iterator it = parts.find(partName);
				if (it == parts.end()) {
						return nullptr;
				}
				return it->second;
			}
			_sp<Rule> symbol(const string symbolName) {
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
		class Evaluators {
		public:
			
			_sp<RulesEvaluator<IN, OUT>> evaluatorById(const int typeId) {
				auto it = evaluators.find(typeId);
				if (it == evaluators.end()) {
					return nullptr;
				}
				return it->second;
			}

			_sp<RulesEvaluator<IN, OUT>> evaluator(_sp<Rule> rule) {
				return evaluatorById(rule->type);
			}

			void evaluator(const int type, _sp<RulesEvaluator<IN, OUT>> evaluator) {
				evaluators.emplace(type, evaluator);
			}
		protected:
			map<int, _sp<RulesEvaluator<IN, OUT>>> evaluators;
		};

		template<typename IN, typename OUT>
		class RulesEvaluator {
		public:
			virtual ~RulesEvaluator() = default;
			virtual OUT accept(_sp<RuleVisitor<IN,OUT>> visitor, _sp<Rule> rule, IN input) = 0;
		};

		/// <summary>
		/// The sole job of the visitor is to glue the aenimic rules to the evaluators, whose job it is to navigate the visitor up and doen the tree.
		/// </summary>
		/// <typeparam name="IN"></typeparam>
		/// <typeparam name="OUT"></typeparam>
		template<typename IN, typename OUT>
		class RuleVisitor : public std::enable_shared_from_this<RuleVisitor<IN, OUT>> {
		public:
			RuleVisitor(_sp<Library> library, _sp<Evaluators<IN,OUT>> evaluators) : library(library), evaluators(evaluators) {}

			OUT visit(_sp<Rule> rule, IN input) {
				auto evaluator = evaluators->evaluator(rule);
				return evaluator->accept(shared_from_this(), rule, input);
			}

			// delegates for shorthandedness.
			_sp<Rule> rule(const string name) {
				return library->rule(name);
			}
			_sp<Rule> part(const string partName) {
				return library->part(partName);
			}
			_sp<Rule> symbol(const string symbolName) {
				return library->symbol(symbolName);
			}
		protected:
			_sp<Library> library;
			_sp<Evaluators<IN,OUT>> evaluators;
		};

		class TerminalRule : public Rule {
		public:
			TerminalRule(const int type) : Rule(type) {}
		};

		class UnaryRule : public Rule {
		public:
			UnaryRule(const int type, _sp<Rule> child) : child(child), Rule(type) {};
			_sp<Rule> getChild() {
				return child;
			}
		protected:
			_sp<Rule> child;
		};

		class CollectionRule : public Rule {
		public:
			CollectionRule(const int type, _sp_vec<Rule> children) : children(children), Rule(type) {}
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
			ValuesRule(int type, vector<T> values) : TerminalRule(type), values(values) {}

			_sp_vec<T> getValues() {
				return values;
			}
		protected:
			vector<T> values;
		};


		namespace trial {
			enum RuleTypes {
				// Terminals
				Eof,
				Any,
				// Unary
				AnyBut,
				Repeat,
				Optional,
				// Collections
				Sequence,
				Or,
				And,
				XOr
			};

			struct BracketHints {
				BracketHints(const bool bracketed = false, const int collection = -1) : bracketed(bracketed), collection(collection) {}
				BracketHints bracket(const bool bracket) {
					return BracketHints(bracket, collection);
				}
				BracketHints coll(const int coll) {
					return BracketHints(bracketed, coll);
				}
				const bool bracketed;
				const int collection;
			};
			using PrintVisitor = RuleVisitor<BracketHints, string>;

			class PrintTerminalEval : public RulesEvaluator<BracketHints, string> {
			public:
				PrintTerminalEval(const string value) : value(value), RulesEvaluator(){}

				virtual string accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, BracketHints bracketHints) override {
					return value;
				}
			protected:
				const string value;
			};

			class PrintUnaryEval : public RulesEvaluator<BracketHints, string> {
			public:
				PrintUnaryEval(const string start, const string end = "", bool hasBrackets = false) : start(start), end(end), hasBrackets(hasBrackets), RulesEvaluator() {}

				virtual string accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, BracketHints bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					string collected = visitor->visit(rule->getChild(), BracketHints(hasBrackets, -1));
					return start + collected + end;
				}
			protected:
				const string start;
				const string end;
				const bool hasBrackets;
			};

			class PrintCollectionEval : public RulesEvaluator<BracketHints, string> {
			public:
				PrintCollectionEval(const string seperator) : seperator(seperator), RulesEvaluator() {}
				virtual string accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, BracketHints bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const int type = (RuleTypes)rule->type;
					const bool aCollection = rule->getChildren().size() > 1;
					// if we have one or less children, we aren't grouping anything
					// if the parent is bracketed, or the collection type is the same, its clearer not use to brackets
					const bool shouldBracket = aCollection && !(bracketHints.bracketed || type == bracketHints.collection);

					bool first = true;
					string collected;
					if (shouldBracket) {
						collected += "(";
					}
					for (auto rule : rule->getChildren()) {
						if (first) {
							first = false;
						}
						else {
							collected += seperator;
						}
						collected += visitor->visit(rule, BracketHints(!aCollection, type));
					}

					if (shouldBracket) {
						collected += ")";
					}
					return collected;
				}
			protected:
				const string seperator;
			};
			/*
			
				AnyBut,
				Repeat,
				Optional,
				*/
			static Evaluators<BracketHints, string> printEvaluators() {
				Evaluators<BracketHints, string> evaluators;
				evaluators.evaluator(And, make_shared<PrintCollectionEval>(" & "));
				evaluators.evaluator(Or, make_shared<PrintCollectionEval>(" | "));
				evaluators.evaluator(XOr, make_shared<PrintCollectionEval>(" ^ "));
				evaluators.evaluator(Sequence, make_shared<PrintCollectionEval>(", "));
				evaluators.evaluator(Eof, make_shared<PrintTerminalEval>("? EOF ?"));
				evaluators.evaluator(Any, make_shared<PrintTerminalEval>("? Any ?"));
				evaluators.evaluator(AnyBut, make_shared<PrintUnaryEval>("-"));
				evaluators.evaluator(Repeat, make_shared<PrintUnaryEval>("{", "}",true));
				evaluators.evaluator(Optional, make_shared<PrintUnaryEval>("[", "]", false));
				return evaluators;
			}

		}
	}
}
#endif