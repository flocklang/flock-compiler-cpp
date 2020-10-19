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
#ifndef FLOCK_UTIL_ID_COUNTER_H
#define FLOCK_UTIL_ID_COUNTER_H

//#include <mutex>

///
/// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
/// 
namespace flock {
	// Class for guarenteeing a uniue id.
	class IDCounter {
	public:

		int next() {
//			std::unique_lock<std::mutex> lock(mut); // should unlock on destructor of lock.
			return nextId();
		}
	private:
		int nextId() {
			return id++;
		}
		int id;
//		std::mutex mut;
	};
}
#endif