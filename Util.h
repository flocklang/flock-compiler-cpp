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

#include <memory>
#include <string>
#include <vector>
#include <initializer_list>

namespace flock {
	using namespace std;

	static bool isNewLine(const int character)
	{
		return character == '\n' || character == '\r';
	}

	static bool isEqual(const string from, string to) {
		return from.compare(to) == 0;
	}
	template<typename Contents>
	static bool any_of(const Contents value, initializer_list<Contents> list) {
		for (Contents elem : list)
		{
			if (value == elem) {
				return true;
			}
		}
		return false;
	}

	static bool any_of(const int value, string list) {
		for (auto elem : list)
		{
			if (value == elem) {
				return true;
			}
		}
		return false;
	}

	template<typename Contents>
	static bool none_of(const Contents value, initializer_list<Contents> list) {
		return !any_of<Contents>(value, list);
	}

	template<typename Contents>
	static bool none_of(const Contents value, string list) {
		return !any_of<Contents>(value, list);
	}

	// Short hand to help readability, (yes really, and on typing)
	template <typename Contents>
	using _sp = shared_ptr<Contents>;

	template <typename Contents>
	using _sp_vec = vector<_sp<Contents>>;

	template <typename Contents>
	using _up = unique_ptr<Contents>;

	template <typename Contents>
	using _up_vec = vector<_up<Contents>>;


}
#endif