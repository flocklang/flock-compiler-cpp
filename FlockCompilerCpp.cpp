/*
 * Copyright 2020 John Orlando Keleshian Moxley All Rights Reserved
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

// FlockCompilerCpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
 // Fixes for specific compilers and IDEs go in here, try not to polute the rest of the code.
#include "CompilerFix.h"

#include <iostream>
#include <string>
//#include "old_Tokenizer.h"
#include "RawToken.h"
#include "ConsoleCharSupplier.h"

using namespace flock;
using namespace flock::token;


/// top ::= definition | external | expression | ';'
static void MainLoop() {
	//ConsoleSupplier consoleSupplier;
	ConsoleCharSupplier consoleSupplier;
	RawTokenizer tokenizer(&consoleSupplier);
	while (true) {
		fprintf(stderr, "\nready> ");

		//std::unique_ptr<TypedToken> token = tokenizer.supply();
		std::shared_ptr<RawToken> token = tokenizer.supply();
		std::cout << "\n" << *token;

		switch (token->getType()) {
		case RawType::Eof:
		//case Token::Type::NewLine:
			return;
		default:
			break;
		}
	}
}

int main()
{
	std::cout << "Hello Flock!\n";
	MainLoop();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
