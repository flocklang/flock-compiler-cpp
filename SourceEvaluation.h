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
#ifndef FLOCK_COMPILER_SOURCE_EVALUATION_H
#define FLOCK_COMPILER_SOURCE_EVALUATION_H

#include "Rules.h"
#include "LogicRules.h"
#include "StringRules.h"
#include "RuleHistory.h"
#include "LocationSupplier.h"

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
namespace flock {
	namespace rule {
		using namespace std;
		using namespace flock::rule::types;
		using namespace flock::rule::history;
		namespace evaluator {
			class SyntaxNode;
			using Tokens = _sp<supplier::CachedSupplier<Location, _sp<Range>>>;
			using Input = pair<int, Tokens>;
			using Output = pair<int, _sp<SyntaxNode>>;
			using Store = Output;
			using EvaluationVisitor = RuleVisitor<Input, Output>;
			using Key = int;
			const static Output FAILURE = make_pair(-1, nullptr);

			class EvaluationMixins : public BaseMixinsCombined<Input, Output>, public LogicMixinsCombined<Input, Output>, public HistoryMixinsCombined<Input, Output, Key, Store> {
			public:
				virtual bool isFailure(Output out) override {
					return out == FAILURE;
				}
				virtual Output makeFailure() override {
					return FAILURE;
				}
				virtual Output makeSuccess(Input input) override {
					return make_pair(input.first + 1, nullptr);
				}
				virtual Output makeEmptySuccess(Input input) override {
					return make_pair(input.first, nullptr);
				}
				virtual bool isEnd(Input input) override {
					Tokens tokens = input.second;
					return tokens->isEnd(input.first);
				}
				virtual Input nextInFromPrevious(Input previousInput, Output previousOutput) override {
					return make_pair(previousOutput.first, previousInput.second);
				}
				virtual Key getKeyForInput(Input input) override {
					const int idx = input.first;
					const Tokens tokens = input.second;
					auto location = tokens->poll(idx);
					if (location) {
						return location->position;
					}
					return -1;
				}

				virtual Output getOutFromStorage(Store store) override {
					return store;
				}
				virtual Store getStorageFromOut(_sp<RuleVisitor<Input, Output>> baseVisitor, Input input, Output output) override {
					return output;
				}
			};


			class HasCharRuleStrategy : public HasValueRuleStrategy<int, Input, Output> {
			public:
				HasCharRuleStrategy(_sp<BaseMixinsCombined<Input, Output>> mixins) : HasValueRuleStrategy<int, Input, Output>(mixins) {}

				virtual Output matches(int value, Input input) override {
					const int idx = input.first;
					const Tokens tokens = input.second;
					auto location = tokens->poll(idx);
					if (location && value == location->character) {
						return make_pair(idx + 1, nullptr);
					}
					return FAILURE;
				}
			};


			class HasStringRuleStrategy : public HasValueRuleStrategy<string, Input, Output> {
			public:
				HasStringRuleStrategy(_sp<BaseMixinsCombined<Input, Output>> mixins) : HasValueRuleStrategy<string, Input, Output>(mixins) {}

				virtual Output matches(string value, Input input) override {
					const int idx = input.first;
					const Tokens tokens = input.second;
					auto range = tokens->pollRange(value.size(), idx);
					if (range && value == range->source) {
						return make_pair(idx + value.size(), nullptr);
					}
					return FAILURE;
				}
			};

			class CharRangeRuleStrategy : public MixinsRuleStrategy<Input, Output> {
			public:
				CharRangeRuleStrategy(_sp<BaseMixinsCombined<Input, Output>> mixins) : MixinsRuleStrategy< Input, Output>(mixins) {}

				virtual Output accept(const _sp<EvaluationVisitor> visitor, const _sp<Rule> baseRule, const Input input) override {
					if (mixins->isEnd(input)) {
						return FAILURE;
					}
					const auto rule = std::dynamic_pointer_cast<ValuesRule<int>>(baseRule);
					const vector<int> values = rule->getValues();

					const int start = values.at(0);
					const int end = values.at(1);
					const int idx = input.first;
					const Tokens tokens = input.second;
					auto location = tokens->poll(idx);

					if (location && start <= location->character && end >= location->character) {
						return make_pair(idx + 1, nullptr);
					}
					return FAILURE; // return failure.
				}

			};

			class EvaluationLibraryStrategy : public LibraryStrategy<Input, Output> {
			public:
				virtual Output accept( _sp<EvaluationVisitor> visitor, _sp<Library> library, Input input) override {
					vector<string> symbolNames = library->getSymbolNames();
					Output out = FAILURE;
					string name = "";

					for (auto rule = symbolNames.begin(); rule != symbolNames.end(); ++rule) {
						cout << "\nEVALUATING : " << *rule << "\n";
						Output newOut = visitor->visit(*rule, input);
						cout << "\nEVALUATED : " << *rule << " : " << to_string(newOut.first) << "\n";
						if (newOut.first > out.first) {
							out = newOut;
							name = *rule;
						}
					}
					if (out.first >= 0) {
						auto evaluated = input.second->popRange(out.first);
						cout << "\nChose : " << name << " : \"" << evaluated->source << "\"" << " : " << *evaluated <<"\n";
					}
					return out; // return the first as a success
				};
				virtual Output accept(const _sp<EvaluationVisitor> visitor, _sp<Library> library) override {
					return FAILURE; // maybe setup console or something later.
				}
			};

			const static _sp<EvaluationMixins> evaluationMixins = make_shared<EvaluationMixins>();

			class SyntaxNode {
			public:
				SyntaxNode(string type) : type(type) {}
				SyntaxNode(_sp<SyntaxNode> parent, string type) : type(type), parent(parent) {}
				SyntaxNode(_sp<SyntaxNode> parent, _sp<Range> range) : type(type), range(range), parent(parent) {}

				void append(_sp<SyntaxNode> syntaxNode) {
					children.push_back(syntaxNode);
				}

				_sp_vec<SyntaxNode> getChildren() {
					return children;
				}
				void setRange(_sp<Range> newRange) {
					range = newRange;
				}
				_sp<Range> getRange() {
					if (range == nullptr) {
						range = getChildrenRange();
					}
					return range;
				}
				_sp<SyntaxNode> getParent() {
					return parent;
				}

				friend std::ostream& operator<<(std::ostream& os, const SyntaxNode& node) {
					string printRange = node.range ? node.range->source : "";
					 os << "{ " << node.type << " : " << printRange << " : [";
					 for (_sp<SyntaxNode> child : node.children) {
						 os << *child;
					 }
					 return os << "]}";
				};

			protected:
				string type;
				_sp<Range> range = nullptr;
				_sp_vec<SyntaxNode> children;
				_sp<SyntaxNode> parent = nullptr;

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


			class CollectingRuleVisitor : public RuleVisitor<Input, Output> {
			public:
				CollectingRuleVisitor(_sp<Library> library, _sp<Strategies<Input, Output>> strategies) : RuleVisitor<Input, Output>(library, strategies) {}

				_sp<SyntaxNode> getSyntaxNode() {
					return syntaxNode;
				}

				_sp<SyntaxNode> attachNode(_sp<SyntaxNode> childNode) {
					if (this->syntaxNode) {
						this->syntaxNode->append(childNode);
						return childNode;
					}
					setNode(childNode);
					return childNode;
				}
				_sp< SyntaxNode> attachNode(_sp<Range> range) {
					if (range) {
						return attachNode(make_shared<SyntaxNode>(this->syntaxNode, range));
					}
					return nullptr;
				}
				//SyntaxNode(string type) : type(type) {}
				void setNode(_sp<SyntaxNode> childNode) {
					this->syntaxNode = childNode;
				}

				virtual Output visitSymbol(string name, _sp<Rule> rule, Input input) override {
					_sp<SyntaxNode> current = this->syntaxNode;
					_sp<SyntaxNode> next = make_shared<SyntaxNode>(current, name);
					setNode(next);
					Output out = this->visit(rule, input);
					if (out.first >= 0) {
						if (next->getChildren().empty()) {
							_sp<Range> range = input.second->pollRange(out.first - input.first, input.first);
							next->setRange(range);
						}
						if (current) {
							setNode(current);
							attachNode(next);
						}
						else {
							setNode(next);
						}

						return make_pair(out.first, next);
					}
					else {
						setNode(current);
					}
					return out;
				}
				virtual void clear() override {
					syntaxNode = nullptr;
				}
			protected:
				_sp<SyntaxNode> syntaxNode;
			};

		

			static _sp<Strategies<Input, Output>> evaluationStrategies() {
				auto strategies = cache<Input, Output, Key, Store>(evaluationMixins);

				addLogicStrategies<Input, Output>(evaluationMixins, strategies);
				strategies->setLibraryStrategy(make_shared<EvaluationLibraryStrategy>());
				strategies->addRuleStrategy(StringRules::EqualChar, make_shared<HasCharRuleStrategy>(evaluationMixins));
				strategies->addRuleStrategy(StringRules::EqualString, make_shared<HasStringRuleStrategy>(evaluationMixins));
				strategies->addRuleStrategy(StringRules::CharRange, make_shared<CharRangeRuleStrategy>(evaluationMixins));
				return strategies;
			}
		}

	}
}
#endif