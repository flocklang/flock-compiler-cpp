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
#ifndef FLOCK_COMPILER_CONSOLE_FORMAT_H
#define FLOCK_COMPILER_CONSOLE_FORMAT_H
#include <string>

using namespace std;
namespace flock {
	namespace colour {

		enum class Colour {
			DARK_RED = 0,
			DARK_GREEN = 1,
			DARK_YELLOW = 2,
			DARK_BLUE = 3,
			DARK_MAGENTA = 4,
			DARK_CYAN = 5,
			RED = 6,
			GREEN = 7,
			YELLOW = 8,
			BLUE = 9,
			MAGENTA = 10,
			CYAN = 11
		};

#ifdef BLACK_WHITE_CONSOLE
		static string randomColourize(string value) {
			return value;
		}
		static string colourize(Colour colour, string value) {
			return value;
		}
		static string colourize(Colour colour, int value) {
			return "" + (char)value;
		}
#else
		const static string COLOUR_END = "\033[0m";
		const static string COLOURS[] = {
			"\x1B[31m",
			"\x1B[32m",
			"\x1B[33m",
			"\x1B[34m",
			"\x1B[35m",
			"\x1B[36m",
			"\x1B[91m",
			"\x1B[92m",
			"\x1B[93m",
			"\x1B[94m",
			"\x1B[95m",
			"\x1B[96m"
		};
		const static int NUM_OF_COLOURS = sizeof(COLOURS) / sizeof(COLOURS[0]);

		static string randomColourize(string value) {
			return COLOURS[rand() % NUM_OF_COLOURS] + value + COLOUR_END;
		}
		static string colourize(Colour colour, string value) {
			return COLOURS[static_cast<int>(colour)] + value + COLOUR_END;
		}
		static string colourize(Colour colour, int value) {
			return COLOURS[static_cast<int>(colour)] + (char)value + COLOUR_END;
		}
		static string colourStart(Colour colour) {
			return COLOURS[static_cast<int>(colour)];
		}
		static string colourEnd() {
			return COLOUR_END;
		}
#endif
	}
}
#endif
