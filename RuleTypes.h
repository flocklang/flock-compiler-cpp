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
#ifndef FLOCK_EBNF_RULE_TYPES_H
#define FLOCK_EBNF_RULE_TYPES_H

#include "Util.h"
#include "LocationSupplier.h"
#include "ConsoleFormat.h"
#include <vector>
#include <map>
#include <string>
#include <numeric>
 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
using namespace std;
namespace flock {
	namespace ebnf {
		using namespace source;
		using namespace colour;
		namespace types {
			// forwad declerations as we have cyclic dependencies on declaration.
			class Rule;
			class SyntaxNode;
			class RuleVisitor;
			class Library;
			enum class Bracket {
				NONE, SEQ, OR, AND, XOR
			};

			using Tokens = _sp<supplier::CachedSupplier <Location, _sp<Range>>>;
			static const int FAILURE = -1;


			class Rule {
			public:
				virtual ~Rule() = default;
				/// <summary>
				/// 
				/// </summary>
				/// <param name="tokens"> the caching tokenizer</param>
				/// <param name="idx">the index in the tokenizer</param>
				/// <param name="visitor"> we use a pseudo vistor pattern to allow us to do work that is part of the processing.</param>
				/// <returns>-1 is a failure, any number including 0 is considered a success. 
				/// Returns the next index to check. Only token rules explicitly increment the index</returns>
				virtual int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) = 0;

				virtual std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) = 0;
			};
			/// <summary>
			/// <symbol> : <expression>
			/// </summary>
			class Library : public  map<const string, _sp<Rule>> {
			public:
				_sp <Rule> part(const string symbol, _sp <Rule> expression) {
					emplace(symbol, expression);
					return expression;
				}
				_sp <Rule> part(const string symbol, initializer_list<_sp<Rule>> expressions);
				_sp <Rule> rule(const string symbol, _sp <Rule> expression);
				_sp <Rule> rule(const string symbol, initializer_list<_sp<Rule>> expressions);

				_sp<Rule> rule(const string symbol) {
					return at(symbol);
				}

				friend std::ostream& operator<<(std::ostream& os, const Library& library) {
					for (const auto& p : library)
					{
						os << colourize(Colour::GREEN, p.first) << " = ";
						p.second->textstream(os, true) << " ;\n";
					}
					return os;
				};
			};

			class SyntaxNode {
			public:
				SyntaxNode(string type) : type(type) {}

				void append(_sp<SyntaxNode> syntaxNode) {
					children.push_back(syntaxNode);
				}
				void fill(_sp<Range> rangeToSet) {
					range = rangeToSet;
				}
				_sp_vec<SyntaxNode> getChildren() {
					return children;
				}
				_sp<Range> getRange() {
					if (range == nullptr) {
						range = getChildrenRange();
					}
					return range;
				}

			protected:
				string type;
				_sp<Range> range = nullptr;
				_sp_vec<SyntaxNode> children;

				_sp<Range> getChildrenRange() {

					if (children.empty()) {
						return nullptr;
					}
					auto option = children.at(0);
					if (!option || option->range) {
						return nullptr;
					}
					_sp<Range> range = option->range;
					for (int i = 1; i < children.size(); i++) {
						auto option = children.at(i);

						if (!option || option->range) {
							return range;
						}
						range = std::make_shared<Range>(Range(range, option->range));
					}
					return range;
				}
			};


			// Internals.

			class RuleVisitor {
			public:
				RuleVisitor(const string type, const _sp<Library> library) : syntaxNode(make_shared<SyntaxNode>(SyntaxNode(type))), library(library) {}

				_sp<RuleVisitor> prepareCollectingVisitor(string type) {
					return make_shared<RuleVisitor>(RuleVisitor(type, library));
				}
				void accept(_sp<RuleVisitor> visitor) {
					syntaxNode->append(visitor->syntaxNode);
				}
				void accept(_sp <Range> range) {
					syntaxNode->fill(range);
				}
				_sp<SyntaxNode> getNode() {
					return syntaxNode;
				}
				_sp<Rule> rule(string ruleName) {
					return library->rule(ruleName);
				}
			protected:
				const _sp<SyntaxNode> syntaxNode;
				const _sp<Library> library;

			};

			std::pair<string, _sp<SyntaxNode>> evaluateAgainstAllRules(Tokens tokens, _sp<Library> library) {
				int idx = FAILURE;
				_sp<SyntaxNode> currentNode = nullptr;
				string successfullRule;
				for (const auto& lib_rule : *library) {
					const string name = lib_rule.first;
					_sp<RuleVisitor> evaluator = std::make_shared<RuleVisitor>(RuleVisitor(name, library));
					int newIdx = lib_rule.second->evaluate(tokens, 0, evaluator);
					if (newIdx > idx) {
						currentNode = evaluator->getNode();
						evaluator->accept(tokens->pollRange(newIdx, 0));
						successfullRule = name;
						idx = newIdx;
					}
					//evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor)
				}
				if (idx > 0) {
					tokens->popRange(idx);
				}
				return std::make_pair(successfullRule, currentNode);
			}
			// Base Rules


			class UnnaryRule : public Rule {
			public:
				UnnaryRule(_sp<Rule> child) : child(child) {}
			protected:
				_sp<Rule> child;
			};

			/// <summary>
			/// Binary is a little bit of a misnomer, as in fact we support any number greater than 1.
			/// In all cases it applies logic to more than 2 rules as though it has applied it to the first 2, 
			/// and then the result of that was applied to the third andd so forth.
			/// 
			/// For instance:
			/// and(rule1, rule2, rule3) will only evaluate as true if they all evaluate as true.
			/// or(rule1, rule2, rule3) will only evaluate to false if they all evaluate as false.
			/// xor(rule1, rule2, rule3) will only evaluate to true if only one of them evaluates to true.
			/// NONE(rule1, rule2, rule3) will execute all 3 in order left to right.
			/// </summary>
			class BinaryRule : public Rule {
			public:
				BinaryRule(_sp_vec<Rule> children) : children(children) {}
				BinaryRule(initializer_list<_sp<Rule>> children) : BinaryRule(_sp_vec<Rule>(children)) {}
				BinaryRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}
			protected:
				_sp_vec<Rule> children;
				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracketType = Bracket::NONE) override {
					if (children.size() == 1) {
						// pass through
						return children.back()->textstream(os, bracketed, bracket());
					}
					else {
						if (!(bracketed || bracket() == bracketType)) {
							os << "(";
						}
						for (auto it = children.cbegin(); it != children.cend(); ++it) {

							(*it)->textstream(os, false, bracket());
							if (it < children.cend() - 1) {
								textstream_seperator(os);
							}
						}
						if (bracketed || bracket() == bracketType) {
							return os;
						}
						return os << ")";
					}
				}
				virtual std::ostream& textstream_seperator(std::ostream& os) = 0;

				virtual Bracket bracket() = 0;

			};

			// Collecting Rules

			class SymbolRule : public UnnaryRule {
			public:
				SymbolRule(_sp<Rule> child, string collectName, const bool highlightCollect = true) : UnnaryRule(child), collectName(collectName), highlightCollect(highlightCollect) {}


				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					_sp<RuleVisitor> newVisitor = visitor->prepareCollectingVisitor(collectName);
					const int newIdx = child->evaluate(tokens, idx, newVisitor);
					if (newIdx == FAILURE) {
						return FAILURE;
					}
					const int amount = newIdx - idx;
					if (amount > 0) {
						_sp<Range> range = tokens->pollRange(amount, idx);
						if (!range) {
							return FAILURE;
						}
						newVisitor->accept(range);
						visitor->accept(newVisitor);
						return newIdx;
					}
					return idx;
				}
				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {
					if (highlightCollect) {
						os << colourize(Colour::CYAN, "? collect:" + collectName + " ? ( ");
					}
					child->textstream(os, true);

					if (highlightCollect) {
						os << colourize(Colour::CYAN, " )");
					}
					return os;
				}

			protected:
				string collectName;
				bool highlightCollect;
			};

			class GrammarRule : public Rule {
			public:
				GrammarRule(const string ruleName) : GrammarRule(ruleName, ruleName) {}
				GrammarRule(const string ruleName, const string collectName) : ruleName(ruleName), collectName(collectName) {}

				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					_sp<Rule> rule = visitor->rule(ruleName);
					if (rule == nullptr) {
						return FAILURE;
					}
					return rule->evaluate(tokens, idx, visitor);
				}

				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {
					return os << colourize(Colour::GREEN, ruleName);
				}

			protected:
				const string ruleName;
				const string collectName;
			};

			// Token Rules
			template<typename T>
			class EqualRule : public Rule {
			public:
				EqualRule(vector<T> values) : values(values) {}
				EqualRule(initializer_list<T> values) : EqualRule(vector<T>(values)) {}
				EqualRule(T value) : EqualRule({ value }) {}

				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					for (T value : values) {
						int newIdx = contains(value, tokens, idx);
						if (newIdx != FAILURE) {
							return newIdx;
						}
					}
					return FAILURE;
				}
				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {
					if (values.size() == 1) {
						return textstream_value(os, values.back());
					}
					else {
						if (!(bracketed || bracket == Bracket::OR)) {
							os << "(";
						}
						for (auto it = values.cbegin(); it != values.cend(); ++it) {
							textstream_value(os, *it);
							if (it < values.cend() - 1) {
								os << " | ";
							}
						}
						if (bracketed || bracket == Bracket::OR) {
							return os;
						}
						return os << ")";
					}
				}
			protected:
				virtual int contains(T provided, Tokens tokens, const int idx) = 0;
				virtual std::ostream& textstream_value(std::ostream& os, T value) = 0;
				vector<T> values;
			};

			class EqualStringRule : public EqualRule<string> {
			public:
				EqualStringRule(vector<string> values) : EqualRule(values) {}
				EqualStringRule(initializer_list<string> values) : EqualRule(values) {}
				EqualStringRule(string value) : EqualRule(value) {}

			protected:
				int contains(string provided, Tokens tokens, const int idx) override {
					auto range = tokens->pollRange(provided.size(), idx);
					if (range && provided == range->source) {
						return idx + provided.size();
					}
					return FAILURE;
				}
				std::ostream& textstream_value(std::ostream& os, string value) override {
					return os << "\"" << colourize(Colour::RED, value) << "\"";
				}

			};

			class EqualCharRule : public EqualRule<int> {
			public:
				EqualCharRule(vector<int> values) : EqualRule(values) {}
				EqualCharRule(initializer_list<int> values) : EqualRule(values) {}
				EqualCharRule(int value) : EqualCharRule({ value }) {}

			protected:

				int contains(int provided, Tokens tokens, const int idx) override {
					auto location = tokens->poll(idx);
					if (location && provided == location->character) {
						return idx + 1;
					}
					return FAILURE;
				}

				std::ostream& textstream_value(std::ostream& os, int value) override {
					switch (value) {
					case -1:
						return os << colourize(Colour::CYAN, "EOF");
					case '\n':
						return os << "'" << colourize(Colour::RED, "\\n") << "'";
					case '\r':
						return os << "'" << colourize(Colour::RED, "\\r") << "'";
					case '\t':
						return os << "'" << colourize(Colour::RED, "\\t") << "'";
					case '\v':
						return os << "'" << colourize(Colour::RED, "\\v") << "'";
					case '\f':
						return os << "'" << colourize(Colour::RED, "\\f") << "'";
					default:
						return os << "'" << colourize(Colour::RED, value) << "'";
					}
				}
			};

			// Logic Rules

			class OptionalRule : public UnnaryRule {
			public:
				OptionalRule(_sp<Rule> child) : UnnaryRule(child) {}

				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {
					os << "[";
					return child->textstream(os, true) << "]";
				}
			protected:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					const int newIdx = child->evaluate(tokens, idx, visitor);
					if (newIdx == FAILURE) {
						return idx;
					}
					return newIdx;
				}
			};

			class NotRule : public UnnaryRule {
			public:
				NotRule(_sp<Rule> child) : UnnaryRule(child) {}

				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {
					os << "!";
					return child->textstream(os);
				}
			protected:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					const int newIdx = child->evaluate(tokens, idx, visitor);
					if (newIdx == FAILURE) {
						return idx;
					}
					return FAILURE;
				}
			};

			class RepeatRule : public UnnaryRule {
			public:
				RepeatRule(_sp<Rule> child, const int min = 0, const int max = 0) : UnnaryRule(child), min(min), max(max) {}

				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {

					if (min == max) {
						if (min == 0) {
							os << "{";
							return  child->textstream(os, true) << "}";
						}
						else if (min == 1) {
							// Pass through
							return  child->textstream(os, bracketed, bracket);
						}
						else {
							os << to_string(min) << " * ";
							return child->textstream(os);
						}
					}
					else {
						os << "";
						if (min == 1) {
							// Pass through
							child->textstream(os, bracketed, bracket) << ", ";
						}
						else if (min > 1) {
							os << to_string(min) << " * ";
							child->textstream(os) << ", ";
						}

						if (max == 0) {
							os << " [";
							return child->textstream(os, true) << "]";
						}
						else {
							os << to_string(max - min) << " * " << "{";
							return child->textstream(os, true) << "}";
						}
					}
				}
			protected:
				const int min; // 0 represents no lower limit
				const int max; // 0 represents no upper limit

				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					int lastIdx = idx;
					for (int i = 0; i < min; i++) {
						lastIdx = child->evaluate(tokens, lastIdx, visitor);
						if (lastIdx == FAILURE) {
							return FAILURE;
						}
					}
					if (max > 0) {
						for (int i = min; i < max + 1; i++) {
							int newIdx = child->evaluate(tokens, lastIdx, visitor);
							if (newIdx == FAILURE) {
								return lastIdx;
							}
							lastIdx = newIdx;
						}
						// we have gone past the maximum
						return FAILURE;
					}
					else {
						while (true) {
							int newIdx = child->evaluate(tokens, lastIdx, visitor);
							if (newIdx == FAILURE) {
								return lastIdx;
							}
							lastIdx = newIdx;
						}
					}
				}
			};

			class AndRule : public BinaryRule {
			public:
				AndRule(_sp_vec<Rule> children) : BinaryRule(children) {}
				AndRule(initializer_list<_sp<Rule>> children) : BinaryRule(children) {}
				AndRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}

			protected:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					const int newIdx = children.at(0)->evaluate(tokens, idx, visitor);
					if (newIdx == FAILURE) {
						return FAILURE;
					}
					for (auto rule = begin(children) + 1; rule != end(children); ++rule) {
						if ((*rule)->evaluate(tokens, idx, visitor) == FAILURE) {
							return FAILURE;
						}
					}
					return newIdx;
				}
				std::ostream& textstream_seperator(std::ostream& os) override {
					return os << " & ";
				}

				Bracket bracket() override {
					return Bracket::AND;
				}
			};

			class OrRule : public BinaryRule {
			public:
				OrRule(_sp_vec<Rule> children) : BinaryRule(children) {}
				OrRule(initializer_list<_sp<Rule>> children) : BinaryRule(children) {}
				OrRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}

			protected:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					for (auto rule = begin(children); rule != end(children); ++rule) {
						const int newIdx = (*rule)->evaluate(tokens, idx, visitor);
						if (newIdx != FAILURE) {
							return newIdx;
						}
					}
					return FAILURE;
				}
				std::ostream& textstream_seperator(std::ostream& os) override {
					return os << " | ";
				}
				Bracket bracket() override {
					return Bracket::OR;
				}
			};


			class XorRule : public BinaryRule {
			public:
				XorRule(_sp_vec<Rule> children) : BinaryRule(children) {}
				XorRule(initializer_list<_sp<Rule>> children) : BinaryRule(children) {}
				XorRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}

			protected:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					int successIdx = FAILURE;
					for (auto rule = begin(children); rule != end(children); ++rule) {
						int newIdx = (*rule)->evaluate(tokens, idx, visitor);
						if (newIdx != FAILURE) {
							if (successIdx == FAILURE) {
								successIdx = newIdx;
							}
							else {
								return FAILURE;
							}
						}
					}
					return successIdx;
				}
				std::ostream& textstream_seperator(std::ostream& os) override {
					return os << " ^ ";
				}
				Bracket bracket() override {
					return Bracket::XOR;
				}
			};

			class SequentialRule : public BinaryRule {
			public:
				SequentialRule(_sp_vec<Rule> children) : BinaryRule(children) {}
				SequentialRule(initializer_list<_sp<Rule>> children) : BinaryRule(children) {}
				SequentialRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}

			protected:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					int newIdx = idx;
					for (auto rule = begin(children); rule != end(children); ++rule) {
						newIdx = (*rule)->evaluate(tokens, newIdx, visitor);
						if (newIdx == FAILURE) {
							// shortcut.
							return FAILURE;
						}
					}
					return newIdx;
				}
				std::ostream& textstream_seperator(std::ostream& os) override {
					return os << ", ";
				}
				Bracket bracket() override {
					return Bracket::SEQ;
				}
			};

			/// <summary>
			/// Automatically passes, and increments by one.
			/// </summary>
			class AnyRule : public Rule {
			public:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					auto loc = tokens->poll(idx);
					if ((!loc) || loc->character == EOF) {
						return FAILURE;
					}
					return idx + 1;
				}
				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {
					return os << colourize(Colour::CYAN, "? Any ?");
				}
			};

			/// <summary>
			/// A cross between the any rule and the not rule.
			/// 
			/// similar to the Not rule, but increments if any not matched.
			/// </summary>
			class AnyButRule : public UnnaryRule {
			public:
				AnyButRule(_sp<Rule> child) : UnnaryRule(child) {}

				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {
					os << "-";
					return child->textstream(os);
				}
			protected:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					const int newIdx = child->evaluate(tokens, idx, visitor);
					if (newIdx == FAILURE) {
						auto loc = tokens->poll(idx);
						if ((!loc) || loc->character == EOF) {
							return FAILURE;
						}
						return idx + 1;
					}
					return FAILURE;
				}
			};

			class EmptyRule : public Rule {
			public:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					return idx;
				}

				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {
					return os;
				}
			};

			class EOFRule : public Rule {
			public:
				int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
					auto loc = tokens->poll(idx);
					if ((!loc) || loc->character == EOF) {
						return idx;
					}
					return FAILURE;
				}

				std::ostream& textstream(std::ostream& os, const bool bracketed = false, const Bracket bracket = Bracket::NONE) override {
					return os << colourize(Colour::CYAN, "? EOF ?");
				}
			};

			// because declration order is a thing in c++
			_sp <Rule> Library::rule(const string symbol, _sp <Rule> expression) {
				return part(symbol, make_shared<SymbolRule>(expression, symbol, false));
			}
			_sp <Rule> Library::rule(const string symbol, initializer_list<_sp<Rule>> expressions) {
				return rule(symbol, make_shared<SequentialRule>(expressions));
			}
			_sp <Rule> Library::part(const string symbol, initializer_list<_sp<Rule>> expressions) {
				return part(symbol, make_shared<SequentialRule>(expressions));
			}

		}
	}
}
#endif