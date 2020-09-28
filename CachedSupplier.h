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

#ifndef FLOCK_COMPILER_CACHED_SUPPLIER_H
#define FLOCK_COMPILER_CACHED_SUPPLIER_H

#include "Supplier.h"
#include <deque>
#include <memory>

namespace flock {
    namespace supplier {

        template<typename T, typename R>
        class CachedSupplier : public  Supplier<std::shared_ptr<T>> {
        public:

            virtual std::shared_ptr<R> pollRange(const int amount = 1, const int startIdx = 0) = 0;

            std::shared_ptr<T> poll(const int idx = 0) {
                for (int i = store.size(); i <= idx; i++) {
                    auto value = supply();
                    if (!value) {
                        return nullptr;
                    }

                    store.push_back(value);
                    // why bother looking up if we have fetched it.
                    if (i == idx) {
                        return value;
                    }
                }
                return store.at(idx);
            };

            std::shared_ptr<T> pop() {
                if (store.empty()) {
                    auto value = supply();
                    if (!value) {
                        return nullptr;
                    }
                    // no need to store as it will be poped anyway.
                    return value;
                }
                else {
                    auto value = store.front();
                    store.pop_front();
                    return value;
                }
            };

            std::shared_ptr<R> popRange(const int amount = 1) {
                auto range = pollRange(amount);
                if (!store.empty()) {
                    store.erase(store.begin(), std::min(store.begin() + amount, store.end()));
                }
                return range;
            };
        protected:
            std::deque<std::shared_ptr<T>> store;
        };
    }
}
#endif