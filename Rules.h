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
#ifndef FLOCK_EBNF_RULES_H
#define FLOCK_EBNF_RULES_H

#include "Util.h"
#include "LocationSupplier.h"
#include "ConsoleFormat.h"
#include <vector>
#include <map>
#include <string>
#include <numeric>
#include <mutex>

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
using namespace std;
namespace flock {
	namespace rule {
		
		// forwad declerations as we have cyclic dependencies on declaration.
		struct Rule;

		class TerminalRule;

		class UnaryRule;
		class CollectionRule;

		struct RuleVisitor;


		struct Rule {
			virtual ~Rule() = default;
			Rule(const int type) : type(type) {}
			virtual void accept(_sp<RuleVisitor> visitor) = 0;
			const int type;
		};


		struct RuleVisitor {
			virtual void visit(int type) = 0;
			virtual void visit(int type, _sp<Rule> child) = 0;
			virtual void visit(int type, _sp_vec<Rule> children) = 0;
		};

		class UnaryRule : public Rule {
		public:
			UnaryRule(int type, _sp<Rule> child) : child(child), Rule(type) {}
			virtual void accept(_sp<RuleVisitor> visitor) override {
				visitor->visit(type, child);
			}
		protected:
			_sp<Rule> child;
		};

		class CollectionRule : public Rule {
		public:
			CollectionRule(int type, _sp_vec<Rule> children) : children(children), Rule(type) {}
			CollectionRule(int type, initializer_list<_sp<Rule>> children) : CollectionRule(type, _sp_vec<Rule>(children)) {}

			virtual void accept(_sp<RuleVisitor> visitor) override {
				visitor->visit(type, children);
			}
		protected:
			_sp_vec<Rule> children;
		};

		class TerminalRule : public Rule {
		public:
			TerminalRule(int type) : Rule(type) {}
			virtual void accept(_sp<RuleVisitor> visitor) override {
				visitor->visit(type);
			}
		};



		namespace trial {
			enum Terminals {
				Eof,
				Any
			};
			enum Collections {
				Sequence,
				Or,
				And,
				Xor
			};
			struct TrialVisitor : RuleVisitor {

				virtual void visit(int type, int value) = 0;
				virtual void visit(int type, int one, int two) = 0;
				virtual void visit(int type, vector<int> values) = 0;
				virtual void visit(int type, string value) = 0;
				virtual void visit(int type, vector<string> values) = 0;
			};


			class CharRule : public TerminalRule {
			public:
				CharRule(int type, int value) : TerminalRule(type), value(value) {}
				virtual void accept(_sp<RuleVisitor> visitor) override {
					std::dynamic_pointer_cast<TrialVisitor> (visitor)->visit(type, value);
				}
				int value;
			};

			class TwoCharRule : public TerminalRule {
			public:
				TwoCharRule(int type, int one, int two) : TerminalRule(type), one(one), two(two) {}
				virtual void accept(_sp<RuleVisitor> visitor) override {
					std::dynamic_pointer_cast<TrialVisitor> (visitor)->visit(type, one, two);
				}
				int one;
				int two;
			};

			class CharsRule : public TerminalRule {
			public:
				CharsRule(int type, vector<int> values) : TerminalRule(type), values(values) {}
				virtual void accept(_sp<RuleVisitor> visitor) override {
					std::dynamic_pointer_cast<TrialVisitor> (visitor)->visit(type, values);
				}
				vector<int> values;
			};

			class StringRule : public TerminalRule {
			public:
				StringRule(int type, string values) : TerminalRule(type), value(value) {}
				virtual void accept(_sp<RuleVisitor> visitor) override {
					std::dynamic_pointer_cast<TrialVisitor> (visitor)->visit(type, value);
				}
				string value;
			};

			class StringsRule : public TerminalRule {
			public:
				StringsRule(int type, vector<string> values) : TerminalRule(type), values(values) {}
				virtual void accept(_sp<RuleVisitor> visitor) override {
					std::dynamic_pointer_cast<TrialVisitor> (visitor)->visit(type, values);
				}
				vector<string> values;
			};

			class PrintVisitor : public TrialVisitor {
			public:
				PrintVisitor(const bool bracketed = false, const int collection = -1) : bracketed(bracketed), parentCollectionType(parentCollectionType) {}
				const bool bracketed;
				const int parentCollectionType;
				string collected;
			protected:
				virtual void visit(int type)override {
					switch ((Terminals)type) {
					case Eof:
						collected += "? EOF ?";
						break;
					case Any:
						collected += "? ANY ?";
						break;
					default:
					}
				}
				virtual void visit(int type, int value) override {

				}
				virtual void visit(int type, int one, int two) override {

				}
				virtual void visit(int type, string value) override {

				}
				virtual void visit(int type, vector<int> values) override {

				}
				virtual void visit(int type, vector<string> values) override {

				}
				virtual void visit(int type, _sp<Rule> child) override {

				}
				virtual void visit(int type, _sp_vec<Rule> children) override {
					string seperator;
					switch ((Collections)type) {
					case Sequence:
						seperator = ", ";
						break;
					case Or:
						seperator = " | ";
						break;
					case And:
						seperator = " & ";
						break;
					case Xor:
						seperator = " ^ ";
						break;
					default:
					}
					bool first = true;
					bool shouldBracket = type != parentCollectionType || !bracketed;
					for (auto rule : children) {
						auto printVisitor = make_shared<PrintVisitor>(shouldBracket, type);
						rule->accept(printVisitor);
						if (first) {
							first = false;
						}
						else {
							collected += seperator;
						}
						collected += printVisitor->collected;
					}
				}
			};


		}
	}
}
#endif