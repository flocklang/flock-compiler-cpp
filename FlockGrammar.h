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
			library.part("whitespace", rule::OR(rule::grammar("blank"), rule::grammar("newline")));
			library.part("digit", rule::digit());
			library.part("alpha", rule::alpha());
			library.part("lineEnd", rule::repeat(rule::seq(rule::grammar("blank*"), rule::OR(rule::grammar("newline"), rule::equal(';')) ),1,0));
			library.part("alphanum", rule::OR(rule::grammar("alpha"), rule::grammar("digit")));
			library.rule("integer", rule::grammar("digit+"));
			library.rule("decimal", { rule::grammar("digit+"), rule::equal('.'), rule::grammar("digit+"), rule::NOT({rule::equal('.'), rule::grammar("digit+")}) });

			// identifierEnd ::= alpha | number | '_' | '$'
			R identifierEnd = rule::OR(rule::grammar("alphanum"), rule::equal({ '_', '$' }));
			// identifierBegin ::= alpha | ( '_',  identifierEnd)
			R identifierBegin = rule::OR( rule::grammar("alpha") , rule::seq(rule::equal('_') , identifierEnd));
			/// identifier ::= identifierBegin, {identifierEnd}
			library.rule("identifier", rule::seq(identifierBegin , rule::repeat(identifierEnd)));

			library.rule("number", rule::OR(rule::grammar("decimal") ,rule::grammar("integer")));
			// we capture escapes as we go through.
			library.rule("string", { rule::equal('"') , rule::repeat(rule::OR(rule::seq(rule::equal('\\'), rule::any()), rule::anybut(rule::equal('"')))), rule::equal('"') });
			library.rule("comment", { rule::equal('/'), rule::OR(rule::seq(rule::equal('/'),  rule::symbol("contents", rule::until(rule::new_line()))),rule::seq(rule::equal('*'), rule::symbol("contents",rule::until(rule::equal("*/"))), rule::equal("*/")))});

			library.rule("use", rule::seq({ rule::keyword("use") , rule::grammar("whitespace*") , rule::option({ rule::equal('('), rule::grammar("whitespace*"), rule::grammar("identifier"), rule::grammar("whitespace*"), rule::equal(')') }), rule::grammar("lineEnd")}));
			return library;
		};
		
		
	}
}
#endif