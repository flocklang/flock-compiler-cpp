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
#ifndef FLOCK_COMPILER_LOCATION_SUPPLIER_H
#define FLOCK_COMPILER_LOCATION_SUPPLIER_H

 // your declarations (and certain types of definitions) here

#include "Source.h"
#include "CachedSupplier.h"

namespace flock {
	using namespace source;
	namespace supplier {
		class LocationSupplier : public CachedSupplier <Location, _sp<Range>> {
		public:
			LocationSupplier(_sp<Supplier<int>> charSupplier) : charSupplier(charSupplier) {}

			_sp<Range> pollRange(const int amount = 1, const int startIdx = 0) override {
				auto option = poll(startIdx);
				if (!option) {
					return nullptr;
				}
				Range* range = new Range(option);
				for (int nextId = startIdx + 1; nextId < startIdx + amount; nextId++) {
					auto option = poll(nextId);

					if (!option) {
						return std::make_shared<Range>(*range);
					}
					range = new Range(*range, option);
				}
				return std::make_shared<Range>(*range);
			};

			_sp<Location> supply() override
			{
				int next = charSupplier->supply();
				if (next == EOF) {
					return previous = nullptr;
				}
				return previous = Location::next(previous, next);
			}
			void clear() {
				store.clear();
				previous = nullptr;
			}

		protected:
			_sp<Location> previous = nullptr;
			_sp<Supplier<int>> charSupplier;
		};

	}
}

#endif
