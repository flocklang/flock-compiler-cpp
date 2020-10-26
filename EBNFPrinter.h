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
#ifndef FLOCK_COMPILER_EBNF_PRINTER_H
#define FLOCK_COMPILER_EBNF_PRINTER_H

#include "Rules.h"
#include "LogicRules.h"
#include "StringRules.h"
#include "ConsoleFormat.h"

namespace flock {
	namespace rule {
		using namespace std;
		using namespace flock::rule::types;
		using namespace flock::colour;
		namespace printer {

			struct BracketHints {
				BracketHints(const bool parentBracketed = false, const int collectionType = -1) : parentBracketed(parentBracketed), collectionType(collectionType) {}
				const bool parentBracketed;
				const int collectionType;
			};

			using Input = BracketHints;
			using Output = string;
			using PrintVisitor = RuleVisitor<Input, Output>;

			/// <summary>
			///  Terminals
			///  Any = _, #Any
			///  End = #End
			/// </summary>
			class PrintTerminal : public RuleStrategy<Input, Output> {
			public:
				PrintTerminal(const string value) : RuleStrategy(), value(value) {}

				virtual Output accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, Input bracketHints) override {
					return colourize(Colour::CYAN, value);
				}
			protected:
				const string value;
			};

			template<typename T>
			class PrintEquals : public RuleStrategy<Input, Output> {
			public:
				PrintEquals() : RuleStrategy() {}

				virtual Output accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, Input bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<ValuesRule<T>>(baseRule);
					const vector<T> values = rule->getValues();
					const bool shouldBracket = values.size() > 1 && bracketHints.collectionType != LogicRules::Or && !bracketHints.parentBracketed;
					bool first = true;
					string collected;
					if (shouldBracket) {
						collected += "(";
					}
					for (auto value : values) {
						if (first) {
							first = false;
						}
						else {
							collected += " | ";
						}
						collected += getValue(value);
					}

					if (shouldBracket) {
						collected += ")";
					}
					return collected;
				}
				virtual string getValue(T value) {
					string collected = "\""; // split otherwise scope goes a bit weird.
					collected += colourize(Colour::RED, value);
					collected += "\"";
					return collected;
				}
			};

			class PrintEqualsChar : public PrintEquals<int> {

				virtual string getValue(int value) override {
					if (isgraph(value) || value == ' ') {
						return PrintEquals<int>::getValue(value);
					}
					else {
						return colourize(Colour::CYAN, "? ASCII character " + to_string(value) + " ?");
					}
				}
			};

			class PrintEqualsString : public PrintEquals<string> {

				virtual string getValue(string value) override {
					string collect;
					bool startOfGraph = true;
					for (char const& ch : value) {
						if (startOfGraph) {
							if (isgraph(ch) || ch == ' ') {
								startOfGraph = false;
								collect += "\"" + colourStart(Colour::RED);
								collect += ch;
							}
							else {
								collect += colourize(Colour::CYAN, "? ASCII character " + to_string(ch) + " ?") + ", ";
							}
						}
						else {
							if (isgraph(ch) || ch == ' ') {
								collect += ch;
							}
							else {
								collect += colourEnd() + "\", " + colourize(Colour::CYAN, "? ASCII character " + to_string(ch) + " ?") + ", ";
								startOfGraph = true;
							}
						}
					}
					if (startOfGraph) {
						collect = collect.substr(0, collect.size() - 2);
					}
					else {
						collect += colourEnd() + "\"";
					}
					return collect;
				}
			};
			/// <summary>
			/// EBNF does not have the concept of range, but its the same as alot of ORs
			/// </summary>
			class PrintRange : public RuleStrategy<Input, Output> {
			public:
				PrintRange() : RuleStrategy() {}

				virtual Output accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, Input bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<ValuesRule<int>>(baseRule);
					const vector<int> values = rule->getValues();
					const int min = values.at(0);
					const int max = values.at(1);
					const bool shouldBracket = (max > min + 1) && bracketHints.collectionType != LogicRules::Or && !bracketHints.parentBracketed;
					bool first = true;
					string collected;
					if (shouldBracket) {
						collected += "(";
					}
					for (int i = min; i <= max; i++) {
						if (first) {
							first = false;
						}
						else {
							collected += " | ";
						}
						collected += getValue(i);
					}

					if (shouldBracket) {
						collected += ")";
					}
					return collected;
				}

				string getValue(int value) {
					if (isgraph(value) || value == ' ') {
						string collected = "\""; // split otherwise scope goes a bit weird.
						collected += colourize(Colour::RED, value);
						collected += "\"";
						return collected;
					}
					else {
						return colourize(Colour::CYAN, "? ASCII character " + to_string(value) + " ?");
					}
				}
			};

			/// <summary>
			///  Repeat = *A , 2*A, +A, 3+A, A{ *,5 }, A{ 2,6 }
			/// </summary>
			class PrintRepeat : public RuleStrategy<Input, Output> {
			public:
				PrintRepeat() : RuleStrategy<Input, Output>() {}

				virtual Output accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, Input bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<RepeatRule>(baseRule);
					const int min = std::max(rule->getMin(), 0); // ensure negatives are 0;
					const int max = std::max(rule->getMax(), 0);
					string prepend;
					string postpend;
					bool bracketed;

					if (min == max) {
						if (min == 0) {
							prepend = "{";
							postpend = "}";
							bracketed = true;
						}
						else if (min == 1) {
							prepend = "";
							postpend = "";
							bracketed = bracketHints.parentBracketed;
						}
						else {
							prepend = to_string(max) + "*";
							postpend = "";
							bracketed = false;
						}

					}
					else if (max == 0) {
						prepend = colourize(Colour::DARK_CYAN, "? FLOCK repeat " + to_string(min) + ",* ");
						postpend = colourize(Colour::DARK_CYAN, " ?"); // Non EBNF
						bracketed = false;
					}
					else if (min == 0) {
						if (max == 1) {
							prepend = "[";
							postpend = "]";
							bracketed = true;
						}
						else {
							prepend = to_string(max) + "*[";
							postpend = "]";
							bracketed = true;
						}
					}
					else {
						prepend = colourize(Colour::DARK_CYAN, "? FLOCK repeat " + to_string(min) + "," + to_string(max) + " ");
						postpend = colourize(Colour::DARK_CYAN, " ?"); // Non EBNF
						bracketed = false;
					}

					const string collected = visitor->visit(rule->getChild(), BracketHints(bracketed, -1));
					return prepend + collected + postpend;

				}
			};
			/// <summary>
			/// 	AnyBut = A<>
			/// </summary>
			class PrintAnyBut : public RuleStrategy<Input, Output> {
			public:
				PrintAnyBut() : RuleStrategy<Input, Output>() {}

				virtual Output accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, Input bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					const string collected = visitor->visit(rule->getChild(), BracketHints(false, -1));
					return colourize(Colour::DARK_CYAN, "? FLOCK anybut ") + collected + colourize(Colour::DARK_CYAN, " ?");
				}
			};

			/// <summary>
			///		Not = A!
			/// </summary>
			class PrintNot : public RuleStrategy<Input, Output> {
			public:
				PrintNot() : RuleStrategy<Input, Output>() {}

				virtual Output accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, Input bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					const string collected = visitor->visit(rule->getChild(), BracketHints(false, -1));
					return colourize(Colour::DARK_CYAN, "? FLOCK not ") + collected + colourize(Colour::DARK_CYAN, " ?");
				}
			};

			/// <summary>
			/// 	Optional = [A]
			/// </summary>
			class PrintOptional : public RuleStrategy<Input, Output> {
			public:
				PrintOptional() : RuleStrategy<Input, Output>() {}

				virtual Output accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, Input bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
					const string collected = visitor->visit(rule->getChild(), BracketHints(true, -1));
					return "[" + collected + "]";
				}
			};

			/// <summary>
			/// Alias = A
			/// </summary>
			class PrintAlias : public RuleStrategy<Input, Output> {
			public:
				PrintAlias() : RuleStrategy<Input, Output>() {}

				virtual Output accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, Input bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<AliasRule>(baseRule);
					return colourize(Colour::GREEN, rule->getAlias());
				}
			};

			/// <summary>
			/// Collections
			///		Sequence = A B C
			///		Or = A | B | C
			///		And = A & B & C
			///		XOr = A ^ B ^ C
			/// </summary>
			class PrintCollection : public RuleStrategy<Input, Output> {
			public:
				PrintCollection(const string seperator) : RuleStrategy<Input, Output>(), seperator(seperator) {}
				virtual Output accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, Input bracketHints) override {
					const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
					const int type = (LogicRules)rule->type;
					const bool aCollection = rule->getChildren().size() > 1;
					// if we have one or less children, we aren't grouping anything
					// if the parent is bracketed, or the collection type is the same, its clearer not use to brackets
					const bool shouldBracket = aCollection && !(bracketHints.parentBracketed || type == bracketHints.collectionType);

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

			class PrintLibraryStrategy : public LibraryStrategy<Input, Output> {
			public:
				virtual Output accept(const _sp<RuleVisitor<Input, Output>> visitor, _sp<Library> library, Input input) override {
					vector<string> symbolNames = library->getSymbolNames();
					vector<string> partNames = library->getPartNames();
					string name = "";
					Output out = colourize(Colour::YELLOW, "\n==== PARTS ====\n");
					for (auto rule = partNames.begin(); rule != partNames.end(); ++rule) {
						out += colourize(Colour::DARK_MAGENTA, *rule + " = ") + visitor->visit(*rule, input) + ";\n";
					}
					out += colourize(Colour::YELLOW, "==== SYMBOLS ====\n");
					for (auto rule = symbolNames.begin(); rule != symbolNames.end(); ++rule) {
						out += colourize(Colour::DARK_MAGENTA, *rule + " = ") + visitor->visit(*rule, input) + ";\n";
					}
					return out; // return the first as a success
				};
				virtual Output accept(const _sp<RuleVisitor<Input, Output>> visitor, _sp<Library> library) override {
					return accept(visitor, library, BracketHints(false, -1));
				}
			};


			static _sp<LibraryStrategies<Input, Output>> printStrategies() {
				_sp<LibraryStrategies<Input, Output>> strategies = make_shared<LibraryStrategies<Input, Output>>();

				strategies->setLibraryStrategy(make_shared<PrintLibraryStrategy>());
				strategies->addRuleStrategy(StringRules::EqualChar, make_shared<PrintEqualsChar>());
				strategies->addRuleStrategy(StringRules::EqualString, make_shared<PrintEqualsString>());
				strategies->addRuleStrategy(StringRules::CharRange, make_shared<PrintRange>());
				strategies->addRuleStrategy(LogicRules::Not, make_shared<PrintNot>());
				strategies->addRuleStrategy(LogicRules::AnyBut, make_shared<PrintAnyBut>());
				strategies->addRuleStrategy(LogicRules::Repeat, make_shared<PrintRepeat>());
				strategies->addRuleStrategy(LogicRules::Optional, make_shared<PrintOptional>());
				strategies->addRuleStrategy(LogicRules::Alias, make_shared<PrintAlias>());
				strategies->addRuleStrategy(LogicRules::End, make_shared<PrintTerminal>("? End ?"));
				strategies->addRuleStrategy(LogicRules::Any, make_shared<PrintTerminal>("? Any ?"));
				strategies->addRuleStrategy(LogicRules::Sequence, make_shared<PrintCollection>(", "));
				strategies->addRuleStrategy(LogicRules::Or, make_shared<PrintCollection>(" | "));
				strategies->addRuleStrategy(LogicRules::And, make_shared<PrintCollection>(" & "));
				strategies->addRuleStrategy(LogicRules::XOr, make_shared<PrintCollection>(" ^ "));
				return strategies;
			}
		}
	}
}
#endif