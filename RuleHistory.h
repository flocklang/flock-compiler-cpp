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
#ifndef FLOCK_COMPILER_RULE_HISTORY_H
#define FLOCK_COMPILER_RULE_HISTORY_H

#include "Rules.h"
#include <memory>
#include <map>
#include <optional>

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
namespace flock {
	namespace rule {
		using namespace std;
		using namespace types;
		namespace history {

			enum class RuleHistoryState {
				New,
				Processing,
				Cyclic,
				Completed
			};

			template< typename OUT>
			class HistoryRecord {
			public:
				HistoryRecord() : historicState(RuleHistoryState::New), outputOpt(optional<OUT>()) {};
				RuleHistoryState getState() {
					return historicState;
				}
				bool isProcessing() {
					return historicState == RuleHistoryState::Processing;
				}
				bool isCompleted() {
					return historicState == RuleHistoryState::Completed;
				}
				bool isCyclic() {
					return historicState == RuleHistoryState::Cyclic;
				}
				void setProcessing() {
					historicState = RuleHistoryState::Processing;
				}
				void setCompleted(OUT output) {
					outputOpt = optional(output);
					historicState = RuleHistoryState::Completed;
				}
				OUT getCompleted() {
					return outputOpt.value();
				}
				void setCyclic() {
					historicState = RuleHistoryState::Cyclic;
				}
			protected:
				RuleHistoryState historicState;
				optional<OUT> outputOpt;
			};


			template<typename KEY, typename OUT>
			class RuleHistory {
			public:

				_sp<HistoryRecord<OUT>> getRecord(const KEY key) {
					if (!records.empty()) {
						auto it = records.find(key);
						if (it != records.end()) {
							return it->second;
						}
					}

					_sp<HistoryRecord<OUT>> record = make_shared<HistoryRecord<OUT>>();
					records.emplace(key, record);
					return record;
				}
				void setProcessing(const KEY key) {
					getRecord(key)->setProcessing();
				}

				void setCompleted(const KEY key, const  OUT output) {
					getRecord(key)->setCompleted(output);
				}

			protected:
				map<const KEY, _sp<HistoryRecord<OUT>>> records;


			};

			
			template<typename KEY, typename OUT>
			class RuleHistories {
			public:

				_sp<RuleHistory<const KEY, OUT>> getRecords(const int ruleId) {
					if (!history.empty()) {
						auto it = history.find(ruleId);
						if (it != history.end()) {
							return it->second;
						}
					}

					auto record = make_shared<RuleHistory<const KEY, OUT>>();
					history.emplace(ruleId, record);
					return record;
				}
			protected:
				map<const int, _sp<RuleHistory<const KEY, OUT>>> history;
			};
	


			template<typename IN, typename OUT, typename KEY = IN>
			class HistoryMixinsCombined : public BaseMixinsCombined<IN, OUT> {
			public:
				virtual KEY getKeyForInput(IN input) = 0;
			};

			/// <summary>
			/// Sometime we may just want to wrap a strategy, to implement common functionality, for instance history.
			/// </summary>
			/// <param name="rule"></param>
			/// <param name="input"></param>
			/// <returns></returns>
			template<typename IN, typename OUT, typename KEY = IN>
			class CachingStrategy : public  WrappingRuleStrategy<IN, OUT> {
			public:
				CachingStrategy(_sp<RuleHistories<KEY, OUT>> histories, _sp<HistoryMixinsCombined<IN,OUT,KEY>> mixins, _sp<RuleStrategy <IN, OUT>> wrapped) : WrappingRuleStrategy<IN, OUT>(wrapped), histories(histories), mixins(mixins){}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					_sp<RuleHistory<const KEY, OUT>> ruleHistory = histories->getRecords(baseRule->id);
					const KEY key = mixins->getKeyForInput(input);
					_sp<HistoryRecord<OUT>> record = ruleHistory->getRecord(key);
					if (record->isCompleted()) {
						return record->getCompleted();
					}
					if (record->isProcessing()) {
						record->setCyclic();
						return mixins->makeFailure();
					}
					if (record->isCyclic()) {
						return mixins->makeFailure();
					}
					record->setProcessing();
					OUT output = wrapped->accept(visitor, baseRule, input);
					record->setCompleted(output);
					return output;
				}

			protected:
				_sp<RuleHistories<KEY, OUT>> histories;
				_sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins;
			};

			template<typename IN, typename OUT, typename KEY = IN>
			_sp<CachingStrategy<IN, OUT, KEY>> cacheResult(_sp<RuleHistories<KEY, OUT>> histories, _sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins, _sp<RuleStrategy<IN, OUT>> strategy) {
				return make_shared<CachingStrategy<IN, OUT, KEY>>(histories, mixins, strategy);
			}

			template<typename IN, typename OUT, typename KEY = IN>
			class CachingStrategies : public Strategies<IN,OUT> {
			public:
				CachingStrategies(_sp<RuleHistories<KEY, OUT>> histories, _sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins) : 
					Strategies<IN,OUT>(), histories(histories), mixins(mixins){}

				CachingStrategies(_sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins) : 
					CachingStrategies(make_shared<RuleHistories<KEY, OUT>>(), mixins) {}

				virtual void setStrategy(const int type, _sp<RuleStrategy<IN, OUT>> strategy) override {
					Strategies<IN,OUT>::setStrategy(type, cacheResult<IN,OUT,KEY>(histories, mixins,strategy));
				}
			protected:
				_sp<RuleHistories<KEY, OUT>> histories;
				_sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins;
			};

		}
	}
}
#endif