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
#include "Grammar.h"
#include "Util.h"


 //===----------------------------------------------------------------------===//
 // Abstract Syntax Tree (aka Parse Tree) Using LLVM Kalediscope Tutorial as Base
 //===----------------------------------------------------------------------===//
using namespace std;
namespace flock {
	using namespace source;
	namespace grammar {
		using R = _sp<Rule>;
		static Library createFlockLibrary() {
			Library library;

			library.rule("newline", new_line());
			library.rule("whitespace", whitespace());
			library.rule("digit", digit());
			library.rule("alpha", alpha());
			library.rule("alphanum", r_or(grammar("alpha"), grammar("digit")));
			library.rule("integer", grammar("digit+"));
			library.rule("decimal", { grammar("digit+"), equal('.'), grammar("digit+"), r_not({equal('.'), grammar("digit+")}) });

			// identifierEnd ::= alpha | number | '_' | '$'
			R identifierEnd = r_or(grammar("alphanum"), equal({ '_', '$' }));
			// identifierBegin ::= alpha | ( '_',  identifierEnd)
			R identifierBegin = r_or( grammar("alpha") , seq(equal('_') , identifierEnd));
			/// identifier ::= identifierBegin, {identifierEnd}
			library.rule("identifier", seq(identifierBegin ,repeat(identifierEnd)));

			library.rule("number", r_or(grammar("decimal") ,grammar("integer")));
			// we capture escapes as we go through.
			library.rule("string", { equal('"') , repeat(r_or(seq(equal('\\'), any()), anybut(r_or(equal('"'),eof())))), equal('"') });

			library.rule("use", seq({ equal("use") , grammar("whitespace*") , option({ equal('('), grammar("whitespace*"), grammar("identifier"), grammar("whitespace*"), equal(')') }) }));
			return library;
		};
		
		
	}
}
#endif