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

//#include "Source.h"
//namespace flock {
//	namespace source {
//		Range operator+(const Location& start, const Location& end) {
//			if (start.position + 1 != end.position) {
//				throw "Non Sequential locations";
//			}
//			return Range(start, end);
//		};
//		Range operator+(const Range& start, const Location& end) {
//			if (start.end.position + 1 != end.position) {
//				throw "Non Sequential locations";
//			}
//			return Range(start, end);
//		}
//		Range operator+=(const Range& start, const Location& end) {
//			return start + end;
//		}
//		Range operator+(const Range& start, const Range& end) {
//			if (start.end.position + 1 != end.start.position) {
//				throw "Non Sequential locations";
//			}
//			return Range(start, end);
//		}
//		Range operator+=(const Range& start, const Range& end) {
//			return start + end;
//		}
//		Range operator+(const Location& start, const int character)
//		{
//			return start + Location::next(start, character);
//		}
//		Range operator+(const Range& start, const int character)
//		{
//			return start + Location::next(start.end, character);
//		}
//		Range operator+=(const Range& start, const int character) {
//			return start + character;
//		}
//		bool operator==(const Location& first, const Location& second) {
//			return first.line == second.line && first.column == second.column
//				&& first.position == second.position && first.character == second.character;
//		};
//
//		bool operator==(const Range& first, const Range& second) {
//			return first.start == second.start
//				&& first.end == second.end
//				&& isequal(first.source, second.source);
//		};
//	}
//}