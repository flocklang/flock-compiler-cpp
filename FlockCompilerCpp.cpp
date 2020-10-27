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

#include "Util.h"
#include "Rules.h"

#include "ConsoleCharSupplier.h"
#include "LocationSupplier.h"
#include "SourceEvaluation.h"
#include "EBNFPrinter.h"
#include "FlockGrammar.h"
#include <iostream>

using namespace std;
using namespace flock;
using namespace flock::source;
using namespace flock::supplier;
using namespace flock::rule;
using namespace flock::rule::types;

static string printRules(_sp<RuleLibrary> library) {
	_sp<LibraryStrategies<printer::Input, printer::Output>> strategies = printer::printStrategies();

	_sp<printer::PrintVisitor>  visitor = make_shared<printer::PrintVisitor>(library, strategies);
	string value = visitor->begin(printer::BracketHints());
	return value;
}



static void MainLoop( _sp<RuleLibrary> library) {
	_sp<ConsoleCharSupplier> consoleSupplier = make_shared<ConsoleCharSupplier>();
	_sp<LocationSupplier> locationSupplier  = make_shared<LocationSupplier>(consoleSupplier);

	_sp<RuleStrategies<evaluator::Input, evaluator::Output>> strategies = evaluator::evaluationStrategies();
	
	_sp<evaluator::CollectingRuleVisitor>  visitor = make_shared<evaluator::CollectingRuleVisitor>(library, strategies);

	std::cout << colourize(Colour::DARK_CYAN, "\nready> ");
	while (true) {
		std::cout << colourize(Colour::DARK_CYAN, "\nready> ");
		evaluator::Input input = make_pair(0, locationSupplier); 
			evaluator::Output output = visitor->begin(input);
		if (output.first == -1) {
			std::cout << colourize(Colour::RED, "NOT FOUND");
		}
		else {
			std::cout << colourize(Colour::GREEN, "FOUND: "+to_string(output.first)) << *output.second;
		}
		consoleSupplier->clear();
		locationSupplier->clear();
		visitor->clear();
		strategies->clear();
		/*std::pair<string, _sp<types::SyntaxNode>> ret = types::evaluateAgainstAllRules(locationSupplier, library);

		string ruleName = get<0>(ret);
		if (ruleName.empty() || ruleName == "eof") {
			consoleSupplier->clear();
			locationSupplier->clear();
			std::cout << colourize(Colour::DARK_MAGENTA, "\nEOF")<< colourize(Colour::DARK_CYAN, "\nready> ");
		} else {
			std::cout << "Rule: " << ruleName;

			_sp<types::SyntaxNode> syntaxNode = get<1>(ret);
			if (syntaxNode && syntaxNode->getRange()) {
				std::cout << " " << *(syntaxNode->getRange());
			}
			std::cout << "\n";
		}*/
	}
}

int main()
{
	std::cout << colourize(Colour::YELLOW, "==== Hello Flock ====\n\n");
	_sp<RuleLibrary> library = make_shared<RuleLibrary>(flock::grammar::createFlockLibrary());
	std::cout << printRules(library);
	MainLoop(library);
	return 0;
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
