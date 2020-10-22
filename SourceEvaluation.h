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
		using namespace flock::rule::logic;
		using namespace flock::rule::history;
		namespace evaluation {
			using Tokens = _sp<supplier::CachedSupplier<Location, _sp<Range>>>;
			using Input = pair<int, Tokens>;
			using Output = int;
			using Key = int;
			const static int FAILURE = -1;

			class EvaluationMixins : public LogicMixinsCombined<Input, Output>, public HistoryMixinsCombined<Input, Output, Key>, public BaseMixinsCombined<Input, Output> {
			public:
				virtual bool isFailure(Output out) override {
					return out == FAILURE;
				}
				virtual Output makeFailure() override {
					return FAILURE;
				}
				virtual Output makeSuccess(Input input) override {
					return input.first + 1;
				}
				virtual Output makeEmptySuccess(Input input) override {
					return input.first;
				}
				virtual bool isEnd(Input input) override {
					Tokens tokens = input.second;
					return tokens->isEnd(input.first);
				}
				virtual Input nextInFromPrevious(Input previousInput, Output previousOutput) override {
					return make_pair(previousOutput, previousInput.second);
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
			};


			class HasCharRuleStrategy : public HasValueRuleStrategy<int, Input, Output> {
			public:
				HasCharRuleStrategy(_sp<BaseMixinsCombined<Input, Output>> mixins) : HasValueRuleStrategy<int, Input, Output> (mixins) {}

				virtual Output matches(int value, Input input) override {
					const int idx = input.first;
					const Tokens tokens = input.second;
					auto location = tokens->poll(idx);
					if (location && value == location->character) {
						return idx + 1;
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
						return idx + value.size();
					}
					return FAILURE;
				}
			};

			class CharRangeRuleStrategy : public MixinsRuleStrategy<Input, Output> {
			public:
				CharRangeRuleStrategy(_sp<BaseMixinsCombined<Input, Output>> mixins) : MixinsRuleStrategy< Input, Output>(mixins) {}

				virtual Output accept(const _sp<RuleVisitor<Input, Output>> visitor, const _sp<Rule> baseRule, const Input input) override {
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
						return idx + 1;
					}
					return FAILURE; // return failure.
				}

			};

			const static _sp<EvaluationMixins> evaluationMixins = make_shared<EvaluationMixins>();

			static _sp<Strategies<Input, Output>> evaluationStrategies() {
				_sp<CachingStrategies<Input, Output, Key>> strategies = make_shared<CachingStrategies<Input, Output, Key>>(evaluationMixins);
				addLogicStrategies<Input, Output>(evaluationMixins, strategies);

				strategies->setStrategy(StringRules::EqualChar, make_shared<HasCharRuleStrategy>(evaluationMixins));
				strategies->setStrategy(StringRules::EqualString, make_shared<HasStringRuleStrategy>(evaluationMixins));
				strategies->setStrategy(StringRules::CharRange, make_shared<CharRangeRuleStrategy>(evaluationMixins));
				return strategies;
			}
		}

	}
}
#endif