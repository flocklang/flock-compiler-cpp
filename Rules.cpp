
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

static string tryItOut() {
	_sp<Library> library = make_shared<Library>(flock::grammar::createFlockLibrary());
	_sp<Strategies<printer::Input, printer::Output>> strategies = printer::evaluationStrategies();
	
	_sp<printer::PrintVisitor>  visitor = make_shared<printer::PrintVisitor>(library, strategies);
	string value = visitor->begin();
	return value;
}

int main()
{
	std::cout << "Hello\n";
	std::cout << tryItOut();
	return 0;
}