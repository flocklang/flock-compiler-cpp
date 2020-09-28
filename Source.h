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

#ifndef FLOCK_COMPILER_SOURCE_H
#define FLOCK_COMPILER_SOURCE_H
#include <string>
#include <iostream>
#include "Util.h"
 /*
  * Points to location within source code. Used for specifying debugging and breakpoints, and also for feedback as we build this.
  * There are no actual pointers here, be aware that there is memory being copied, and that these are immutable.
  */
namespace flock {
	namespace source {
		struct Location {
			Location(const int character) : line(1), column(1), position(0), character(character) {};
			Location(const int line, const int column, const int position, const int character) : line(line), column(column), position(position), character(character) {};

			Location(const Location& other) : line(other.line), column(other.column), position(other.position), character(other.character) {}


			const int line;
			const int column;
			const int position;
			const int character;
			friend std::ostream& operator<<(std::ostream& os, const Location& location) {
				return os << "line: " << location.line << ", column: " << location.column << ", position: " << location.position << ", character: " << location.character;
			};
			/*
* Helper for the providing the next location, based on the previous location.
*/
			static const Location next(Location last, const int character) {
				if (isnewline(last.character)) {
					return Location(last.line + 1, 1, last.position + 1, character);
				}
				else {
					return Location(last.line, last.column + 1, last.position + 1, character);
				}
			}
		};

		struct Range {
			Range(const int character) : Range{ std::make_shared<Location>(Location(character)) } {};
			Range(const std::shared_ptr < Location> start) : start{ start }, end{ start } {
				source.push_back(start->character); 
			};
			Range(const std::shared_ptr < Location> start, const std::shared_ptr < Location> end) : start{ start }, end{ end } {
				source.push_back(start->character);
				source.push_back(end->character);
			};
			Range(const Range start, const std::shared_ptr < Location> end) : start{ start.start }, end{ end } {
				source.append(start.source);
				source.push_back(end->character);
			};
			Range(const Range start, const Range end) : start{ start.start }, end{ end.end } {
				source.append(start.source);
				source.append(end.source);
			};

			Range(const Range& other) : start(other.start), end(other.end) {
				source.append(other.source);
			};

			const std::shared_ptr<Location> start;
			const std::shared_ptr<Location> end;
			std::string source;

			friend std::ostream& operator<<(std::ostream& os, const Range& range) {
				return os << "start: {line: " << range.start->line << ", column: " << range.start->column << ", position: " << range.start->position << "}"
					<< ", end: {line: " << range.end->line << ", column: " << range.end->column << ", position: " << range.end->position << "}, source: " << range.source;
			};

			std::string toStringNoText() {
				std::string ret;
				ret.append("start: {line: ").append(std::to_string(start->line))
					.append(", column: ").append(std::to_string(start->column))
					.append(", position: ").append(std::to_string(start->position))
					.append("}, end: {line: ").append(std::to_string(end->line))
					.append(", column: ").append(std::to_string(end->column))
					.append(", position: ").append(std::to_string(end->position))
					.append("}, sourceLength: ").append(std::to_string(source.size()));
				return ret;
			};
		};

		///*
		//	 * All of these operator overloads presume the end location or range immediately follows the start location or range,
		//	 * a Non Sequential locations exception is thrown if this is not true.
		//	 */
		//Range operator+(const Location& start, Location& end);
		//Range operator+(const Range& start, Location& end);
		//Range operator+(const Range& start, Range& end);
		//Range operator+(const Location& start, const int character);
		//Range operator+(const Range& start, const int character);
		//bool operator==(const Location& first, Location& second);
		//bool operator==(const Range& first, Range& second);
	}
}
#endif