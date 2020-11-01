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

			template< typename STORE>
			class HistoryRecord {
			public:
				HistoryRecord() : historicState(RuleHistoryState::New), outputOpt(optional<STORE>()) {};
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
				void setCompleted(STORE output) {
					outputOpt = optional(output);
					historicState = RuleHistoryState::Completed;
				}
				STORE getCompleted() {
					return outputOpt.value();
				}
				void setCyclic() {
					historicState = RuleHistoryState::Cyclic;
				}
			protected:
				RuleHistoryState historicState;
				optional<STORE> outputOpt;
			};


			template<typename KEY, typename STORE>
			class RuleHistory {
			public:

				_sp<HistoryRecord<STORE>> getRecord(const KEY key) {
					if (!records.empty()) {
						auto it = records.find(key);
						if (it != records.end()) {
							return it->second;
						}
					}

					_sp<HistoryRecord<STORE>> record = make_shared<HistoryRecord<STORE>>();
					records.emplace(key, record);
					return record;
				}
				void setProcessing(const KEY key) {
					getRecord(key)->setProcessing();
				}

				void setCompleted(const KEY key, const  STORE output) {
					getRecord(key)->setCompleted(output);
				}

			protected:
				map<const KEY, _sp<HistoryRecord<STORE>>> records;


			};


			template<typename KEY, typename STORE>
			class RuleHistories {
			public:

				_sp<RuleHistory<const KEY, STORE>> getRecords(const int ruleId) {
					if (!history.empty()) {
						auto it = history.find(ruleId);
						if (it != history.end()) {
							return it->second;
						}
					}

					auto record = make_shared<RuleHistory<const KEY, STORE>>();
					history.emplace(ruleId, record);
					return record;
				}
				virtual void clear() {
					history.clear();
				}
			protected:
				map<const int, _sp<RuleHistory<const KEY, STORE>>> history;
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
			class CachingRuleStrategy : public  WrappingRuleStrategy<IN, OUT> {
			public:
				CachingRuleStrategy(_sp<RuleHistories<KEY, OUT>> histories, _sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins, _sp<RuleStrategy <IN, OUT>> wrapped) : WrappingRuleStrategy<IN, OUT>(wrapped), histories(histories), mixins(mixins) {}

				virtual OUT accept(_sp<RuleVisitor<IN, OUT>> visitor, _sp<Rule> baseRule, IN input) override {
					_sp<RuleHistory<const KEY, OUT>> ruleHistory = histories->getRecords(baseRule->id);
					const KEY key = mixins->getKeyForInput(input);
					_sp<HistoryRecord<OUT>> record = ruleHistory->getRecord(key);
					if (record->isCompleted()) {
						return record->getCompleted();
					}
					if (record->isProcessing()) {
						record->setCyclic();
						return this->mixins->makeFailure();
					}
					if (record->isCyclic()) {
						return this->mixins->makeFailure();
					}
					record->setProcessing();
					OUT output = this->wrapped->accept(visitor, baseRule, input);
					record->setCompleted(output);
					return output;
				}

			protected:
				_sp<RuleHistories<KEY, OUT>> histories;
				_sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins;
			};


			template<typename IN, typename OUT, typename KEY = IN>
			_sp<CachingRuleStrategy<IN, OUT, KEY>> cacheResult(_sp<RuleHistories<KEY, OUT>> histories, _sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins, _sp<RuleStrategy<IN, OUT>> strategy) {
				return make_shared<CachingRuleStrategy<IN, OUT, KEY>>(histories, mixins, strategy);
			}


			template<typename IN, typename OUT, typename KEY = IN>
			class CachingStrategies : public WrappingStrategies<IN, OUT> {
			public:
				CachingStrategies(_sp<Strategies<IN, OUT>> strategies, _sp<RuleHistories<KEY, OUT>> histories, _sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins) :
					WrappingStrategies<IN, OUT>(strategies),
					histories(histories), mixins(mixins) {}

				CachingStrategies(_sp<Strategies<IN, OUT>> strategies, _sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins) :
					CachingStrategies(strategies, make_shared<RuleHistories<KEY, OUT>>(), mixins) {}

				virtual void addStrategy(const int type, _sp<RuleStrategy<IN, OUT>> strategy) override {
					getWrappedStratagies()->addStrategy(type, cacheResult<IN, OUT, KEY>(histories, mixins, strategy));
				}
				_sp<RuleHistories<KEY, OUT>> getHistories() {
					return histories;
				}

				virtual void clear() override {
					histories->clear();
					WrappingStrategies<IN, OUT>::clear();
				}
			protected:
				_sp<RuleHistories<KEY, OUT>> histories;
				_sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins;
			};

			template<typename IN, typename OUT, typename KEY = IN>
			static _sp<CachingStrategies<IN, OUT, KEY>> cache(_sp<Strategies<IN, OUT>> strategies, _sp<RuleHistories<KEY, OUT>> histories, _sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins) {
				return make_shared<CachingStrategies<IN, OUT, KEY>>(strategies, histories, mixins);
			}

			template<typename IN, typename OUT, typename KEY = IN>
			static _sp<CachingStrategies<IN, OUT, KEY>> cache(_sp<Strategies<IN, OUT>> strategies, _sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins) {
				return make_shared<CachingStrategies<IN, OUT, KEY>>(strategies, mixins);
			}

			template<typename IN, typename OUT, typename KEY = IN>
			static _sp<CachingStrategies<IN, OUT, KEY>> cache(_sp<HistoryMixinsCombined<IN, OUT, KEY>> mixins) {
				return make_shared<CachingStrategies<IN, OUT, KEY>>(make_shared<BaseStrategies<IN,OUT>>(), mixins);
			}
		}
	}
}
#endif