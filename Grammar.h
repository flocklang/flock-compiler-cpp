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
#ifndef FLOCK_COMPILER_GRAMMAR_H
#define FLOCK_COMPILER_GRAMMAR_H
#include "Util.h"
#include "RawToken.h"
#include <vector>
#include <map>
#include <string>

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
using namespace std;
namespace flock {
	using namespace source;
	namespace grammar {
		// forwad declerations as we have cyclic dependencies on declaration.
		class Rule;
		class SyntaxNode;
		class RuleVisitor;
		class Library;

		const static string COLOR_END = "\033[0m";
		const static string COLOR_RED = "\x1B[31m";
		const static string COLOR_GREEN = "\x1B[92m";
		const static string COLOR_YELLOW = "\x1B[93m";
		const static string COLOR_DARK_YELLOW = "\x1B[93m";
		const static string COLOR_DARK_COLOR_MAGENTA = "\x1B[95m";
		const static string COLOR_MAGENTA = "\x1B[95m";
		const static string COLOR_DARK_CYAN = "\x1B[36m";
		const static string COLOR_CYAN = "\x1B[96m";
		const static string COLOR_BLUE = "\x1B[94m";
		// blue is just too dark
		const static string COLORS[] = { COLOR_RED , COLOR_GREEN, COLOR_YELLOW,COLOR_DARK_YELLOW, COLOR_MAGENTA, COLOR_DARK_COLOR_MAGENTA, COLOR_CYAN,COLOR_DARK_CYAN };
		const static int NUM_OF_COLORS = sizeof(COLORS) / sizeof(COLORS[0]);

		static string RANDOM_COLOR() {
			return COLORS[rand() % NUM_OF_COLORS];
		}

		using Tokens = supplier::CachedSupplier<token::RawToken>*;
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

			virtual std::ostream& textstream(std::ostream& os, const bool bracketed = false) = 0;
		};
		/// <summary>
		/// <symbol> : <expression>
		/// </summary>
		class Library : public  map<const string, _sp<Rule>> {
		public:
			_sp <Rule> rule(const string symbol, _sp <Rule> expression) {
				emplace(symbol, expression);
				return expression;
			}
			_sp <Rule> rule(const string symbol, initializer_list<_sp<Rule>> expressions);

			_sp<Rule> rule(const string symbol) {
				return at(symbol);
			}

			friend std::ostream& operator<<(std::ostream& os, const Library& library) {
				for (const auto& p : library)
				{
					os << p.first << " = ";
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
			void fill(_sp_vec<token::RawToken> tokensToSet) {
				tokens = tokensToSet;
				if (tokens.size() > 0) {
					_sp<Range> newRange = tokens.at(0)->getContents();
					for (int i = 1; i < tokens.size(); i++) {
						newRange = make_shared<Range>(Range(newRange, tokens.at(i)->getContents()));
					}
					range = newRange;
				}
			}
		protected:
			_sp_vec<SyntaxNode> children;
			_sp_vec<token::RawToken> tokens;
			_sp<Range> range = nullptr;
			string type;
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
			void accept(_sp_vec<token::RawToken> tokensToSet) {
				syntaxNode->fill(tokensToSet);
			}
			_sp<Rule> rule(string ruleName) {
				library->rule(ruleName);
			}
		protected:
			_sp<SyntaxNode> syntaxNode;
			const _sp<Library> library;

		};

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
		/// seq(rule1, rule2, rule3) will execute all 3 in order left to right.
		/// </summary>
		class BinaryRule : public Rule {
		public:
			BinaryRule(initializer_list<_sp<Rule>> children) : children(_sp_vec<Rule>(children)) {}
			BinaryRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}
		protected:
			_sp_vec<Rule> children;
		};

		// Collecting Rules

		class CollectingRule : public UnnaryRule {
		public:
			CollectingRule(_sp<Rule> child, string collectName) : UnnaryRule(child), collectName(collectName) {}
			CollectingRule(_sp<Rule> child) : UnnaryRule(child) {}
			int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
				_sp<RuleVisitor> newVisitor = visitor->prepareCollectingVisitor(collectName);
				const int newIdx = child->evaluate(tokens, idx, newVisitor);
				if (newIdx == FAILURE) {
					return FAILURE;
				}
				const int amount = newIdx - idx;
				if (amount > 0) {
					_sp_vec<token::RawToken> range = tokens->pollRange(amount, idx);
					newVisitor->accept(range);
					visitor->accept(newVisitor);
					return newIdx;
				}
				return idx;
			}
			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				return os << collectName;
			}

		protected:
			string collectName;
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
				return CollectingRule(rule, collectName).evaluate(tokens, idx, visitor);
			}

			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				return os << "<" << ruleName << ">";
			}

		protected:
			const string ruleName;
			const string collectName;
		};

		// Token Rules
		template<typename T, typename E = T>
		class EqualRule : public Rule {
		public:
			EqualRule(initializer_list<T> values) : values(vector<T>(values)) {}
			EqualRule(T value) : EqualRule({ value }) {}

			int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
				E toCompare = this->getFromRawToken(tokens->poll(idx));
				for (T value : values) {
					if (isEqual(value, toCompare)) {
						return idx + 1;
					}
				}
				return FAILURE;
			}
			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				return os;
			}
		protected:
			virtual bool isEqual(T provided, E evaluated) = 0;
			virtual E getFromRawToken(_sp<token::RawToken> token) = 0;
			vector<T> values;
		};

		class EqualTypeRule : public EqualRule<token::RawType> {
		public:
			EqualTypeRule(initializer_list<token::RawType> values) : EqualRule(values) {}
			EqualTypeRule(token::RawType value) : EqualRule(value) {}


			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				if (values.size() == 1) {
					return os << toString(values.back());
				}
				else {
					const string color = RANDOM_COLOR();
					if (!bracketed) {
						os << color << "(" << COLOR_END;
					}
					for (auto it = values.cbegin(); it != values.cend(); ++it) {

						os << toString(*it);
						if (it < values.cend() - 1) {
							os << " | ";
						}
					}
					if (bracketed) {
						return os;
					}
					return os << color << ")" << COLOR_END;
				};
			}
		protected:
			token::RawType getFromRawToken(_sp<token::RawToken> token) override {
				return token->getType();
			}

			bool isEqual(token::RawType provided, token::RawType evaluated) override {
				return provided == evaluated;
			}
		};

		class EqualStringRule : public EqualRule<string> {
		public:
			EqualStringRule(initializer_list<string> values) : EqualRule(values) {}
			EqualStringRule(string value) : EqualRule(value) {}

			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				if (values.size() == 1) {
					return os << "\"" << values.back() << "\"";
				}
				else {
					const string color = RANDOM_COLOR();
					if (!bracketed) {
						os << color << "(" << COLOR_END;
					}
					for (auto it = values.cbegin(); it != values.cend(); ++it) {

						os << "\"" << *it << "\"";
						if (it < values.cend() - 1) {
							os << " | ";
						}
					}
					if (!bracketed) {
						return os;
					}
					return os << color << ")" << COLOR_END;
				}
			}
		protected:
			string getFromRawToken(_sp<token::RawToken> token) override {
				return token->getContents()->source;
			}

			bool isEqual(string provided, string evaluated) override {
				return provided == evaluated;
			}

		};

		class HasCharRule : public EqualRule<int, string> {
		public:
			HasCharRule(initializer_list<int> values, const int pos = 0) : EqualRule(values), pos(pos) {}
			HasCharRule(int value, const int pos = 0) : HasCharRule({ value }, pos) {}


			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				if (values.size() == 1) {
					return os << "'" << (char)(values.back()) << "'";
				}
				else {
					const string color = RANDOM_COLOR();
					if (!bracketed) {
						os << color << "(" << COLOR_END;
					}
					for (auto it = values.cbegin(); it != values.cend(); ++it) {

						os << "'" << *it << "'";
						if (it < values.cend() - 1) {
							os << " | ";
						}
					}
					if (bracketed) {
						return os;
					}
					return os << color << ")" << COLOR_END;
				}
			}
		protected:
			string getFromRawToken(_sp<token::RawToken> token) override {
				return token->getContents()->source;
			}
			bool isEqual(int provided, string evaluated) override {
				return evaluated.length() > pos && provided == evaluated.at(pos);
			}
			const int pos;
		};

		// Logic Rules

		class OptionalRule : public UnnaryRule {
		public:
			OptionalRule(_sp<Rule> child) : UnnaryRule(child) {}

			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				const string color = RANDOM_COLOR();
				os << color << "[" << COLOR_END;
				return child->textstream(os, true) << color << "]" << COLOR_END;;
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

			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
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

			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				const string color = RANDOM_COLOR();

				if (min == max) {
					if (min == 0) {
						os << color << "{" << COLOR_END;
						return  child->textstream(os, true) << color << "}" << COLOR_END;
					}
					else if (min == 1) {
						// Pass through
						return  child->textstream(os, bracketed);
					}
					else {
						os << to_string(min) << " * ";
						return child->textstream(os);
					}
				}
				else {
					os << "";
					if (min > 0) {
						os << to_string(min) << " * ";
						child->textstream(os) << ", ";
					}
					os << to_string(max) << " * " << color << "[" << COLOR_END;;
					return child->textstream(os, true) << color << "]" << COLOR_END;;
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
			AndRule(initializer_list<_sp<Rule>> children) : BinaryRule(children) {}
			AndRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}

			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				if (children.size() == 1) {
					// pass through
					return children.back()->textstream(os, bracketed);
				}
				else {
					string color = RANDOM_COLOR();
					if (!bracketed) {
						os << color << "(" << COLOR_END;
					}
					for (auto it = children.cbegin(); it != children.cend(); ++it) {

						(*it)->textstream(os);
						if (it < children.cend() - 1) {
							os << " & ";
						}
					}
					if (bracketed) {
						return os;
					}
					return os << color << ")" << COLOR_END;
				}
			}
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
		};

		class OrRule : public BinaryRule {
		public:
			OrRule(initializer_list<_sp<Rule>> children) : BinaryRule(children) {}
			OrRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}
			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				if (children.size() == 1) {
					// pass through
					return children.back()->textstream(os, bracketed);
				}
				else {
					string color = RANDOM_COLOR();
					if (!bracketed) {
						os << color << "(" << COLOR_END;
					}
					for (auto it = children.cbegin(); it != children.cend(); ++it) {

						(*it)->textstream(os);
						if (it < children.cend() - 1) {
							os << " | ";
						}
					}
					if (bracketed) {
						return os;
					}
					return os << color << ")" << COLOR_END;
				}
			}
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
		};


		class XorRule : public BinaryRule {
		public:
			XorRule(initializer_list<_sp<Rule>> children) : BinaryRule(children) {}
			XorRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}
	
			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				if (children.size() == 1) {
					// pass through
					return children.back()->textstream(os, bracketed);
				}
				else {
					string color = RANDOM_COLOR();
					if (!bracketed) {
						os << color << "(" << COLOR_END;
					}
					for (auto it = children.cbegin(); it != children.cend(); ++it) {

						(*it)->textstream(os);
						if (it < children.cend() - 1) {
							os << " ^ ";
						}
					}
					if (bracketed) {
						return os;
					}
					return os << color << ")" << COLOR_END;
				}
			}
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
		};

		class SequentialRule : public BinaryRule {
		public:
			SequentialRule(initializer_list<_sp<Rule>> children) : BinaryRule(children) {}
			SequentialRule(_sp<Rule> left, _sp<Rule> right) : BinaryRule({ left, right }) {}
			
			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				if (children.size() == 1) {
					// pass through
					return children.back()->textstream(os, bracketed);
				}
				else {
					string color = RANDOM_COLOR();
					if (!bracketed) {
						os << color << "(" << COLOR_END;
					}
					for (auto it = children.cbegin(); it != children.cend(); ++it) {

						(*it)->textstream(os);
						if (it < children.cend() - 1) {
							os << ", ";
						}
					}
					if (bracketed) {
						return os;
					}
					return os << color << ")" << COLOR_END;
				}
			}
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
		};

		/// <summary>
		/// Automatically passes, and increments by one.
		/// </summary>
		class AnyRule : public Rule {
		public:
			int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
				return idx+1;
			}
			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				return os << *this;
			}
			friend std::ostream& operator<<(std::ostream& os, const AnyRule& rule) {
				return os << "Any";
			};
		};

		/// <summary>
		/// A cross between the any rule and the not rule.
		/// 
		/// similar to the Not rule, but increments if any not matched.
		/// </summary>
		class AnyButRule : public UnnaryRule {
		public:
			AnyButRule(_sp<Rule> child) : UnnaryRule(child) {}

			std::ostream& textstream(std::ostream& os, const bool bracketed = false) override {
				os << "-";
				return child->textstream(os);
			}
		protected:
			int evaluate(Tokens tokens, const int idx, _sp<RuleVisitor> visitor) override {
				const int newIdx = child->evaluate(tokens, idx, visitor);
				if (newIdx == FAILURE) {
					return idx+1;
				}
				return FAILURE;
			}
		};


		// Shorthand 
		static _sp<SequentialRule> seq(initializer_list<_sp<Rule>> rules) {
			return make_shared<SequentialRule>(SequentialRule(rules));
		}
		static _sp<AnyRule> any() {
			return make_shared<AnyRule>(AnyRule());
		}
		static _sp<AnyButRule> anybut(_sp<Rule> rule) {
			return make_shared<AnyButRule>(AnyButRule(rule));
		}
		static _sp<AnyButRule> anybut(initializer_list<_sp<Rule>> rules) {
			return anybut(seq(rules));
		}
		static _sp<CollectingRule> collect(string type, _sp<Rule> rule) {
			return make_shared<CollectingRule>(CollectingRule(rule, type));
		}
		static _sp<EqualTypeRule> equal(initializer_list<token::RawType> values) {
			return make_shared<EqualTypeRule>(EqualTypeRule(values));
		}
		static _sp<EqualTypeRule> equal(token::RawType value) {
			return equal({ value });
		}
		static _sp<EqualStringRule> equal(initializer_list<string > values) {
			return make_shared<EqualStringRule>(EqualStringRule(values));
		}
		static _sp<EqualStringRule> equal(string value) {
			return equal({ value });
		}
		static _sp<HasCharRule> has_char(initializer_list<int> values, const int pos = 0) {
			return make_shared<HasCharRule>(HasCharRule(values, pos));
		}

		static _sp<HasCharRule> has_char(int value, const int pos = 0) {
			return has_char({ value }, pos);
		}

		static _sp<NotRule> r_not(_sp<Rule> rule) {
			return make_shared<NotRule>(NotRule(rule));
		}
		static _sp<NotRule> r_not(initializer_list<_sp<Rule>> rules) {
			return make_shared<NotRule>(NotRule(seq(rules)));
		}
		static _sp<RepeatRule> repeat(_sp<Rule> rule, const int min = 0, const int max = 0) {
			return make_shared<RepeatRule>(RepeatRule(rule, min, max));
		}
		static _sp<OrRule> r_or(initializer_list<_sp<Rule>> rules) {
			return make_shared<OrRule>(OrRule(rules));
		}
		static _sp<OrRule> r_or(_sp<Rule> left, _sp<Rule> right) {
			return r_or({ left, right });
		}
		static _sp<OrRule> r_or(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right) {
			return r_or({ left, middle, right });
		}
		static _sp<AndRule> r_and(initializer_list<_sp<Rule>> rules) {
			return make_shared<AndRule>(AndRule(rules));
		}
		static _sp<AndRule> r_and(_sp<Rule> left, _sp<Rule> right) {
			return r_and({ left, right });
		}
		static _sp<AndRule> r_and(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right) {
			return r_and({ left, middle, right });
		}
		static _sp<XorRule> r_xor(initializer_list<_sp<Rule>> rules) {
			return make_shared<XorRule>(XorRule(rules));
		}
		static _sp<XorRule> r_xor(_sp<Rule> left, _sp<Rule> right) {
			return r_xor({ left, right });
		}
		static _sp<SequentialRule> seq(_sp<Rule> left, _sp<Rule> right) {
			return seq({ left, right });
		}
		static _sp<SequentialRule> seq(_sp<Rule> left, _sp<Rule> middle, _sp<Rule> right) {
			return seq({ left, middle, right });
		}

		static _sp<OptionalRule> option(_sp<Rule> rule) {
			return make_shared<OptionalRule>(OptionalRule(rule));
		}
		static _sp<OptionalRule> option(initializer_list<_sp<Rule>> rules) {
			return option(seq(rules));
		}
		static _sp<GrammarRule> grammar(string grammer, string name) {
			return make_shared<GrammarRule>(GrammarRule(grammer, name));
		}
		static _sp<GrammarRule> grammar(string grammer) {
			return make_shared<GrammarRule>(GrammarRule(grammer));
		}
		static _sp<HasCharRule> symbol(initializer_list<int> values) {
			//return r_and(equal(token::RawType::Punctuation), has_char(values));
			return has_char(values);
		}
		static _sp<HasCharRule> symbol(int value) {
			return symbol({ value });
		}
		static _sp<EqualStringRule> keyword(string value) {
			//return r_and(equal(token::RawType::Alpha), equal({ value }));
			return equal({ value });
		}
		static _sp<EqualStringRule> keyword(initializer_list<string> values) {
			// return r_and(equal(token::RawType::Alpha), equal(values));
			return equal({ values });
		}
		static _sp<EqualTypeRule> alpha() {
			return  equal(token::RawType::Alpha);
		}
		static _sp<EqualTypeRule> number() {
			return  equal(token::RawType::Integer);
		}
		static _sp<RepeatRule> alphanum(const int min = 0, const int max = 0) {
			return  repeat(r_or(alpha(), number()), min, max);
		}
		static _sp<EqualTypeRule> eof() {
			return  equal(token::RawType::Eof);
		}
		static _sp<RepeatRule> whitespace(const int min = 0, const int max = 0) {
			return repeat(equal(token::RawType::Whitespace), min, max);
		}
		static _sp<RepeatRule> newline(const int min = 0, const int max = 0) {
			return repeat(equal(token::RawType::NewLine), min, max);
		}
		static _sp<RepeatRule> whitespace_all(const int min = 0, const int max = 0) {
			return repeat(r_or(equal(token::RawType::Whitespace), equal(token::RawType::NewLine), eof()), min, max);
		}

		// Operator overloads

		template<class L = Rule*, class R = Rule*>
		static _sp<OrRule> operator||(const L left, const  R right) {
			return r_or({ left, right });
		}
		template<class L = Rule*, class R = Rule*>
		static _sp<AndRule> operator&&(const L left, const  R right) {
			return r_and({ left, right });
		}
		template<class L = Rule*, class R = Rule*>
		static _sp<AndRule> operator^(const L left, const  R right) {
			return r_xor({ left, right });
		}
		template<class C = Rule*>
		static _sp<NotRule> operator!(const C child) {
			return r_not(child);
		}

		template<class C = Rule*>
		static _sp<OptionalRule> operator~(const C child) {
			return option(child);
		}

		template<class L = Rule*, class R = Rule*>
		static _sp<SequentialRule> operator>>(const L left, const  R right) {
			return seq({ left, right });
		}

		template<class L = Rule*>
		static _sp<SequentialRule> operator>>(const L left, const string right) {
			return seq({ left, make_shared<GrammarRule>(GrammarRule(right)) });
		}
		template<class R = Rule*>
		static _sp<SequentialRule> operator>>(const string left, const R right) {
			return seq({ make_shared<GrammarRule>(GrammarRule(left)), right });
		}


		// because declration order is a thing in c++

		_sp <Rule> Library::rule(const string symbol, initializer_list<_sp<Rule>> expressions) {
			return rule(symbol, seq(expressions));
		}

	}
}
#endif