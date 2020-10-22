
#include "Util.h"
#include "Rules.h"
#include "SourceEvaluation.h"
#include <iostream>

using namespace std;
using namespace flock;
using namespace flock::rule;
using namespace flock::rule::types;


struct BracketHints {
	BracketHints(const bool parentBracketed = false, const int collectionType = -1) : parentBracketed(parentBracketed), collectionType(collectionType) {}
	BracketHints withParentBracketed(const bool bracket) {
		return BracketHints(bracket, collectionType);
	}
	BracketHints withCollectionType(const int collType) {
		return BracketHints(parentBracketed, collType);
	}
	const bool parentBracketed;
	const int collectionType;
};
using PrintVisitor = RuleVisitor<BracketHints, string>;

class PrintTerminalEval : public RuleStrategy<BracketHints, string> {
public:
	PrintTerminalEval(const string value) : RuleStrategy(), value(value) {}

	virtual string accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, BracketHints bracketHints) override {
		return value;
	}
protected:
	const string value;
};

class PrintUnaryEval : public RuleStrategy<BracketHints, string> {
public:
	PrintUnaryEval(const string start, const string end = "", bool hasBrackets = false) : RuleStrategy(), start(start), end(end), hasBrackets(hasBrackets) {}

	virtual string accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, BracketHints bracketHints) override {
		const auto rule = std::dynamic_pointer_cast<UnaryRule>(baseRule);
		string collected = visitor->visit(rule->getChild(), BracketHints(hasBrackets, -1));
		return start + collected + end;
	}
protected:
	const string start;
	const string end;
	const bool hasBrackets;
};

class PrintCollectionEval : public RuleStrategy<BracketHints, string> {
public:
	PrintCollectionEval(const string seperator) : RuleStrategy(), seperator(seperator) {}
	virtual string accept(_sp<PrintVisitor> visitor, _sp<Rule> baseRule, BracketHints bracketHints) override {
		const auto rule = std::dynamic_pointer_cast<CollectionRule>(baseRule);
		const int type = (LogicRules)rule->type;
		const bool aCollection = rule->getChildren().size() > 1;
		// if we have one or less children, we aren't grouping anything
		// if the parent is bracketed, or the collection type is the same, its clearer not use to brackets
		const bool shouldBracket = aCollection && !(bracketHints.parentBracketed || type == bracketHints.collectionType);

		bool first = true;
		string collected;
		if (shouldBracket) {
			collected += "(";
		}
		for (auto rule : rule->getChildren()) {
			if (first) {
				first = false;
			}
			else {
				collected += seperator;
			}
			collected += visitor->visit(rule, BracketHints(!aCollection, type));
		}

		if (shouldBracket) {
			collected += ")";
		}
		return collected;
	}
protected:
	const string seperator;
};
/*

	AnyBut,
	Repeat,
	Optional,
	*/
static Strategies<BracketHints, string> printStrategies() {
	Strategies<BracketHints, string> strategies;
	strategies.setStrategy(LogicRules::And, make_shared<PrintCollectionEval>(" & "));
	strategies.setStrategy(LogicRules::Or, make_shared<PrintCollectionEval>(" | "));
	strategies.setStrategy(LogicRules::XOr, make_shared<PrintCollectionEval>(" ^ "));
	strategies.setStrategy(LogicRules::Sequence, make_shared<PrintCollectionEval>(", "));
	strategies.setStrategy(LogicRules::End, make_shared<PrintTerminalEval>("? End ?"));
	strategies.setStrategy(LogicRules::Any, make_shared<PrintTerminalEval>("? Any ?"));
	strategies.setStrategy(LogicRules::AnyBut, make_shared<PrintUnaryEval>("-"));
	strategies.setStrategy(LogicRules::Repeat, make_shared<PrintUnaryEval>("{", "}", true));
	strategies.setStrategy(LogicRules::Optional, make_shared<PrintUnaryEval>("[", "]", false));
	return strategies;
}



static string tryItOut() {
	Strategies<BracketHints, string> strategies = printStrategies();
	Library library;
	auto any = make_shared <TerminalRule>(LogicRules::Any);
	auto eof = make_shared <TerminalRule>(LogicRules::End);
	_sp<Rule> OR = _collectionRule(LogicRules::Or, { any, eof });
	_sp<Rule> REPEAT = make_shared<UnaryRule>(LogicRules::Repeat, OR);
	_sp<Rule> COLL = _collectionRule(LogicRules::Sequence, { OR, REPEAT, any });
	library.setSymbol("myRule", COLL);
	_sp< PrintVisitor>  visitor = make_shared<PrintVisitor>(make_shared<Library>(library), make_shared<Strategies<BracketHints, string>>(strategies));
	return visitor->visit(COLL, BracketHints(true, -1));
}

int main()
{
	std::cout << "Hello\n";
	std::cout << tryItOut();
	return 0;
}