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
#ifndef FLOCK_COMPILER_CONSOLE_CHAR_SUPPLIER_H
#define FLOCK_COMPILER_CONSOLE_CHAR_SUPPLIER_H

#include "Supplier.h"
#include <iostream>
#include <string>

namespace flock {
	using namespace std;
	namespace supplier {
		class ConsoleCharSupplier : public Supplier <int> {
		public:
			ConsoleCharSupplier() : ConsoleCharSupplier("") {}
			ConsoleCharSupplier(const string end) : end(end) {}
			void clear() {
				inputComplete = false;
				line = "";
				pos = 0;
				size = -1;
			}
		protected:
			int supply() override {
				if (inputComplete) {
					return EOF;
				}
				else {
					if (pos > size) {
						getline(std::cin, line);
						pos = 0;
						size = line.size();
					}
					if (line == end) {
						inputComplete = true;
						return EOF;
					}

					if (pos < size) {
						int c = line.at(pos++);
						return c;
					}
					pos++; // pos should be equal to size, so we needd to increment to ensure pos > size.
					return '\n';
				}
			}
			std::string line;
			int pos = 0;
			int size = -1;
			bool inputComplete = false;
			const string end;
		};

	}
}
#endif