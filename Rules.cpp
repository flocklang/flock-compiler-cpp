
#include "Util.h"
#include "Rules.h"
using namespace flock;
using namespace flock::rule;
using namespace flock::rule::trial;


static string tryItOut() {
	Evaluators<BracketHints, string> evaluators = printEvaluators();
	Library library;
	auto any = make_shared <TerminalRule>((int)Any);
	auto eof = make_shared <TerminalRule>((int)Eof);
	_sp<Rule> OR = make_shared<CollectionRule>(CollectionRule((int)Or, { any, eof }));
	_sp<Rule> REPEAT = make_shared<UnaryRule>((int)Repeat, OR);
	_sp<Rule> COLL = make_shared<CollectionRule>(CollectionRule((int)Sequence, { OR, REPEAT, any }));
	library.symbol("myRule", COLL);
	_sp< PrintVisitor>  visitor = make_shared<PrintVisitor>(make_shared<Library>(library), make_shared<Evaluators<BracketHints, string>>(evaluators));
	_sp< PrintVisitor> visitorSP = visitor->shared_from_this();
	return visitor->visit(COLL, BracketHints(true, -1));
}

int main()
{
	std::cout << "Hello\n";
	std::cout << tryItOut();
}