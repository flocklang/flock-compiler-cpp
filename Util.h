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
#ifndef FLOCK_COMPILER_UTIL_H
#define FLOCK_COMPILER_UTIL_H

#include <string>
#include <vector>

using namespace std;
namespace flock {

	static bool isnewline(const int character)
	{
		return character == '\n' || character == '\r';
	}

	static bool isequal(const string from, string to) {
		return from.compare(to) == 0;
	}

	template <typename T>
	using _sp = shared_ptr<T>;

	template <typename T>
	using _sp_vec = vector<shared_ptr<T>>;
}
#endif