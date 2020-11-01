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

#include <iostream>
#include "Rules.h"
#include "LogicRules.h"
#include "StringRules.h"
#include "RuleHistory.h"
#include "LocationSupplier.h"
#include "Syntax.h"

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
namespace flock {
	namespace rule {
		using namespace std;
		using namespace flock::syntax;
		using namespace flock::rule::types;
		using namespace flock::rule::history;
		namespace evaluator {
			using Tokens = _sp<supplier::CachedSupplier<Location, _sp<Range>>>;
			struct Input;
			struct Output;
			class SyntaxStrategies;
			using EvaluationVisitor = RuleVisitor<Input, Output>;
			using Key = int;

			struct Input {
				Input(const Tokens tokens, const int idx) : idx(idx), tokens(tokens) {}
				Input(const Tokens tokens) : idx(0), tokens(tokens) {}
				Input(const Input& other) : idx{ other.idx }, tokens(other.tokens)
				{ }

				Input& operator=(const Input& other)
				{
					if (&other != this) {
						idx = other.idx;
						tokens = other.tokens;
					}
					return *this;
				}
				Input next(const int idx) {
					return Input(tokens, idx);
				}
				int idx;
				Tokens tokens;
			};

			struct Output {
				Output(int idx, _sp_vec<SyntaxNode>	syntaxNodes) : idx(idx), syntaxNodes(syntaxNodes) {}
				Output(int idx, _sp<SyntaxNode>	syntaxNode) : idx(idx), syntaxNodes({ syntaxNode }) {}
				Output(int idx) : idx(idx) {}
				bool isFailure() {
					return idx < 0;
				}
				bool isSuccess() {
					return idx >= 0;
				}
				bool hasNodes() {
					return !syntaxNodes.empty();
				}
				int idx;
				_sp_vec<SyntaxNode>	syntaxNodes;
			};

			const static Output FAILURE = Output(-1);

			class EvaluationMixins : public BaseMixinsCombined<Input, Output>, public LogicMixinsCombined<Input, Output>, public HistoryMixinsCombined<Input, Output, Key> {
			public:
				virtual bool isFailure(Output out) override {
					return out.isFailure();
				}
				virtual Output makeFailure() override {
					return FAILURE;
				}
				virtual Output makeSuccess(Input input) override {
					return Output(input.idx + 1);
				}
				virtual Output makeEmptySuccess(Input input) override {
					return Output(input.idx);
				}
				virtual bool isEnd(Input input) override {
					Tokens tokens = input.tokens;
					return tokens->isEnd(input.idx);
				}
				virtual Input nextInFromPrevious(Input previousInput, Output previousOutput) override {
					return previousInput.next(previousOutput.idx);
				}
				virtual Output joinOutputs(Output first, Output second) override {
					if (first.hasNodes()) {
						if (second.hasNodes()) {
							_sp_vec<SyntaxNode> nodes;
							nodes.reserve(first.syntaxNodes.size() + second.syntaxNodes.size()); // preallocate memory
							nodes.insert(nodes.end(), first.syntaxNodes.begin(), first.syntaxNodes.end());
							nodes.insert(nodes.end(), second.syntaxNodes.begin(), second.syntaxNodes.end());
							return Output(second.idx, nodes);
						}
						else {
							return Output(second.idx, first.syntaxNodes);
						}
					}
					else {
						return second;
					}
				}
				virtual Key getKeyForInput(Input input) override {
					const int idx = input.idx;
					const Tokens tokens = input.tokens;
					auto location = tokens->poll(idx);
					if (location) {
						return location->position;
					}
					return -1;
				}
			};

			class HasCharRuleStrategy : public HasValueRuleStrategy<int, Input, Output> {
			public:
				HasCharRuleStrategy(_sp<BaseMixinsCombined<Input, Output>> mixins) : HasValueRuleStrategy<int, Input, Output>(mixins) {}

				virtual Output matches(int value, Input input) override {
					const int idx = input.idx;
					const Tokens tokens = input.tokens;
					auto location = tokens->poll(idx);
					if (location && value == location->character) {
						return Output(idx + 1);
					}
					return FAILURE;
				}
			};

			class HasStringRuleStrategy : public HasValueRuleStrategy<string, Input, Output> {
			public:
				HasStringRuleStrategy(_sp<BaseMixinsCombined<Input, Output>> mixins) : HasValueRuleStrategy<string, Input, Output>(mixins) {}

				virtual Output matches(string value, Input input) override {
					const int idx = input.idx;
					const Tokens tokens = input.tokens;
					auto range = tokens->pollRange(value.size(), idx);
					if (range && value == range->source) {
						return Output(idx + value.size());
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
					const int idx = input.idx;
					const Tokens tokens = input.tokens;
					auto location = tokens->poll(idx);

					if (location && start <= location->character && end >= location->character) {
						return Output(idx + 1);
					}
					return FAILURE; // return failure.
				}

			};

			class EvaluationLibraryStrategy : public LibraryStrategy<Input, Output> {
			public:
				virtual Output accept(_sp<EvaluationVisitor> visitor, _sp<RuleLibrary> library, Input input) override {
					vector<string> symbolNames = library->getSymbolNames();
					Output out = FAILURE;
					string name = "";

					for (auto rule = symbolNames.begin(); rule != symbolNames.end(); ++rule) {
						try {
							Output newOut = visitor->visitByName(*rule, input);
							if (newOut.idx > out.idx) {
								name = *rule;
								out = newOut;
							}
						}
						catch (string exc) {
							cout << "\nexception was thrown: " << exc << "\n";
						}
					}
					if (out.isSuccess()) {
						auto evaluated = input.tokens->popRange(out.idx - input.idx);

						_sp<SyntaxNode> syntaxNode = make_shared<SyntaxNode>(name, input.tokens->pollRangeBetween(input.idx, out.idx));
						if (out.hasNodes()) {
							for (_sp<SyntaxNode> child : out.syntaxNodes) {
								if (child) {
									syntaxNode->append(child->clone());
								}
							}
						}
						return Output(out.idx, syntaxNode);
					}
					return out; // return the first as a success
				};
			};


			class SyntaxAliasRuleStrategy : public  WrappingRuleStrategy<Input, Output> {
			public:
				SyntaxAliasRuleStrategy(_sp<RuleStrategy <Input, Output>> wrapped) : WrappingRuleStrategy<Input, Output>(wrapped) {}

				virtual Output accept(_sp<RuleVisitor<Input, Output>> visitor, _sp<Rule> baseRule, Input input) override {
					const auto rule = std::dynamic_pointer_cast<AliasRule>(baseRule);
					Output output = wrapped->accept(visitor, rule, input);
					if (output.isSuccess() && visitor->getSymbol(rule->getAlias())) {
						_sp<SyntaxNode> syntaxNode = make_shared<SyntaxNode>(rule->getAlias(), input.tokens->pollRangeBetween(input.idx, output.idx));
						if (output.hasNodes()) {
							for (_sp<SyntaxNode> child : output.syntaxNodes) {
								if (child) {
									syntaxNode->append(child->clone());
								}
							}
						}
						return Output(output.idx, syntaxNode);
					}
					return output;
				}
			};

			class SyntaxStrategies : public types::WrappingStrategies<Input, Output> {
			public:
				SyntaxStrategies(_sp<types::Strategies<Input, Output>> strategies) :
					types::WrappingStrategies<Input, Output>(strategies) {}

				virtual void addStrategy(const int type, _sp<RuleStrategy<Input, Output>> strategy) override {
					switch (type) {
					case LogicRules::Alias:
						getWrappedStratagies()->addStrategy(type, make_shared<SyntaxAliasRuleStrategy>(strategy));
						return;
					default:
						getWrappedStratagies()->addStrategy(type, strategy);
					}
				}
			};

			const static _sp<EvaluationMixins> evaluationMixins = make_shared<EvaluationMixins>();

			static _sp<Strategies<Input, Output>> evaluationStrategies() {
				auto baseStrategies = make_shared<BaseStrategies<Input, Output>>();
				auto strategies = cache<Input, Output, Key>(make_shared< SyntaxStrategies>(baseStrategies), evaluationMixins);
				//auto strategies = make_shared<SyntaxStrategies>(baseStrategies);
				addLogicStrategies<Input, Output>(evaluationMixins, strategies);
				strategies->setLibraryStrategy(make_shared<EvaluationLibraryStrategy>());
				strategies->addStrategy(StringRules::EqualChar, make_shared<HasCharRuleStrategy>(evaluationMixins));
				strategies->addStrategy(StringRules::EqualString, make_shared<HasStringRuleStrategy>(evaluationMixins));
				strategies->addStrategy(StringRules::CharRange, make_shared<CharRangeRuleStrategy>(evaluationMixins));
				return strategies;
			}
		}

	}
}
#endif