
#include "Util.h"
#include "Rules.h"
#include "SourceEvaluation.h"
#include "EBNFPrinter.h"
#include "FlockGrammar.h"
#include <iostream>

using namespace std;
using namespace flock;
using namespace flock::rule;
using namespace flock::rule::types;

static /*string tryItOut() {
	_sp<Library> library = make_shared<Library>(flock::grammar::createFlockLibrary());
	_sp<LibraryStrategies<printer::Input, printer::Output>> strategies = printer::printStrategies();
	
	_sp<printer::PrintVisitor>  visitor = make_shared<printer::PrintVisitor>(library, strategies);
	string value = visitor->begin();
	return value;
}*/

int main2()
{
	std::cout << "Hello\n";
	//std::cout << tryItOut();
	return 0;
}