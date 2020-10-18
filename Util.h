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

	template<typename BaseType>
	static void into(std::vector<BaseType>& collection) {/* do nothing, but we may have empty variadics.*/};

	template<typename BaseType, typename LastRule = _sp<BaseType>>
	static void into(_sp_vec<BaseType>& collection, LastRule last)
	{
		collection.push_back(last);
	};

	template<typename BaseType, typename FirstRule = _sp<BaseType>, typename ...OtherRules>
	static void into(_sp_vec<BaseType>& collection, FirstRule const& first, OtherRules... others)
	{
		collection.push_back(first);
		into(collection, others...);
	};
	template <typename BaseType, typename... Types>
	static _sp_vec<BaseType> from(_sp<Types>... values) {
		_sp_vec<BaseType> collection;
		into(collection, values...);
		return collection;
	}


}
#endif