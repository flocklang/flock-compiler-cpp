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
#include <initializer_list>

using namespace std;
namespace flock {

	static bool isNewLine(const int character)
	{
		return character == '\n' || character == '\r';
	}

	static bool isEqual(const string from, string to) {
		return from.compare(to) == 0;
	}
	template<typename T>
	static bool any_of(const T value, initializer_list<T> list) {
		for (T elem : list)
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

	template<typename T>
	static bool none_of(const T value, initializer_list<T> list) {
		return !any_of<T>(value, list);
	}

	template<typename T>
	static bool none_of(const T value, string list) {
		return !any_of<T>(value, list);
	}

	template <typename T>
	using _sp = shared_ptr<T>;

	template <typename T>
	using _sp_vec = vector<_sp<T>>;
}
#endif