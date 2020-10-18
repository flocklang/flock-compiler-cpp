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
#ifndef FLOCK_COMPILER_FLOCK_GRAMMAR_H
#define FLOCK_COMPILER_FLOCK_GRAMMAR_H
#include "RuleFunctions.h"
#include "Util.h"


 //===----------------------------------------------------------------------===//
 // Abstract Syntax Tree (aka Parse Tree) Using LLVM Kalediscope Tutorial as Base
 //===----------------------------------------------------------------------===//
using namespace std;
namespace flock {
	using namespace source;
	using namespace ebnf;
	namespace grammar {
		using R = _sp<types::Rule>;
		static types::Library createFlockLibrary() {
			types::Library library;

			library.part("eof", rule::eof());
			library.part("newline", rule::new_line());
			library.part("blank", rule::blank());
			library.part("whitespace", rule::OR(rule::RULE("blank"), rule::RULE("newline")));
			library.part("digit", rule::digit());
			library.part("alpha", rule::alpha());
			library.part("lineEnd", rule::REP(rule::SEQ(rule::RULE("blank*"), rule::OR(rule::RULE("newline"), rule::EQ(';')) ),1,0));
			library.part("alphanum", rule::OR(rule::RULE("alpha"), rule::RULE("digit")));
			library.rule("integer", rule::RULE("digit+"));
			library.rule("decimal", { rule::RULE("digit+"), rule::EQ('.'), rule::RULE("digit+"), rule::NOT({rule::EQ('.'), rule::RULE("digit+")}) });

			// identifierEnd ::= alpha | number | '_' | '$'
			R identifierEnd = rule::OR(rule::RULE("alphanum"), rule::EQ({ '_', '$' }));
			// identifierBegin ::= alpha | ( '_',  identifierEnd)
			R identifierBegin = rule::OR( rule::RULE("alpha") , rule::SEQ(rule::EQ('_') , identifierEnd));
			/// identifier ::= identifierBegin, {identifierEnd}
			library.rule("identifier", rule::SEQ(identifierBegin , rule::REP(identifierEnd)));

			library.rule("number", rule::OR(rule::RULE("decimal") ,rule::RULE("integer")));
			// we capture escapes as we go through.
			library.rule("string", { rule::EQ('"') , rule::REP(rule::OR(rule::SEQ(rule::EQ('\\'), rule::ANY()), rule::ANYBUT(rule::EQ('"')))), rule::EQ('"') });
			library.rule("comment", { rule::EQ('/'), rule::OR(rule::SEQ(rule::EQ('/'),  rule::SYM("contents", rule::UNTIL(rule::new_line()))),rule::SEQ(rule::EQ('*'), rule::SYM("contents",rule::UNTIL(rule::EQ("*/"))), rule::EQ("*/")))});
			library.rule("identifierList", rule::OR({ rule::RULE("identifier"), rule::SEQ({ rule::EQ('('), rule::RULE("whitespace*"), rule::RULE("identifierList"), rule::REP(rule::SEQ({rule::RULE("whitespace*"),rule::EQ(','),rule::RULE("whitespace*"), rule::RULE("identifierList"),rule::RULE("whitespace*")})), rule::RULE("whitespace*"),rule::EQ(')')}) }));

			library.rule("use", rule::SEQ({ rule::keyword("use") , rule::RULE("whitespace*") , rule::OPT(rule::RULE("identifierList")), rule::RULE("lineEnd")}));
			return library;
		};
		
		
	}
}
#endif