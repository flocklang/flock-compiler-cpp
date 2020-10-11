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
			// identifierEnd ::= alpha | number | '_' | '$'
			R identifierEnd = r_or({ alpha(),number(),symbol('_'),symbol('$')});
			// identifierBegin ::= alpha | ( '_',  identifierEnd)
			R identifierBegin = alpha() || (symbol('_') >> identifierEnd);

			/// identifier ::= identifierBegin, (identifierEnd)*
			library.rule("identifier", identifierBegin >> repeat(identifierEnd));

			library.rule("integer", number());
			library.rule("decimal", { number(), symbol('.'), number(), r_not({symbol('.'), number()}) });
			library.rule("number", grammar("decimal") || grammar("integer"));
			// probably capture escapes as we go through.
			library.rule("string", { symbol('"') , repeat(anybut({anybut(symbol('\\')), symbol('"') }) )});

			library.rule("use", seq({ keyword("use") , whitespace() , option({ symbol('('), whitespace(), grammar("identifier"), whitespace(), symbol(')') }) }));
			return library;
		};
		
		
	}
}
#endif