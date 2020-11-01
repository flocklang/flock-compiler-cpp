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
#include "Rules.h"
#include "LogicRules.h"
#include "StringRules.h"
#include "Util.h"


 //===----------------------------------------------------------------------===//
 // Abstract Syntax Tree (aka Parse Tree) Using LLVM Kalediscope Tutorial as Base
 //===----------------------------------------------------------------------===//
using namespace std;
namespace flock {
	using namespace source;
	using namespace rule;
	using namespace rule;
	namespace grammar {
		using R = _sp<types::Rule>;
		static _sp < types::RuleLibrary>  createFlockLibrary() {
			_sp < types::RuleLibrary> library = make_shared< types::RuleLibrary>(unwrap());


			library->addPart("eof", rule::END());
			library->addPart("newline*+?-", rule::NEW_LINE());
			library->addPart("blank*+?-", rule::BLANK());
			library->addPart("wsp*+?-", rule::OR(rule::RULE("blank"), rule::RULE("newline")));
			library->addPart("digit*+?-", rule::DIGIT());
			library->addPart("alpha*+?-", rule::ALPHA());
			library->addPart("lineEnd*+?-", rule::SEQ(rule::RULE("blank*"), rule::OR(rule::RULE("newline"), rule::EQ(';'))));
			library->addPart("alphanum*+?-", rule::OR(rule::RULE("alpha"), rule::RULE("digit")));
			library->addPart("integer", rule::RULE("digit+"));
			library->addPart("decimal", rule::SEQ({ rule::RULE("digit+"), rule::EQ('.'), rule::RULE("digit+"), rule::NOT({rule::EQ('.'), rule::RULE("digit+")}) }));
			library->addPart("_identifier", rule::SEQ({ rule::RULE("wsp*"), rule::RULE("identifier"), rule::RULE("wsp*") }));
			library->addPart("_aliasList", rule::SEQ({ rule::RULE("wsp*"), rule::RULE("aliasList"), rule::RULE("wsp*") }));

			// identifierEnd ::= alpha | number | '_' | '$'
			R identifierEnd = rule::OR(rule::RULE("alphanum"), rule::EQ({ '_', '$' }));
			// identifierBegin ::= alpha | ( '_',  identifierEnd)
			R identifierBegin = rule::OR(rule::RULE("alpha"), rule::SEQ(rule::EQ('_'), identifierEnd));
			/// identifier ::= identifierBegin, {identifierEnd}
			library->addSymbol("identifier", rule::SEQ(identifierBegin, rule::REP(identifierEnd)));

			library->addSymbol("number", rule::OR(rule::RULE("decimal"), rule::RULE("integer")));
			// we capture escapes as we go through.
			library->addSymbol("string", rule::SEQ({ rule::EQ('"') , rule::REP(rule::OR(rule::SEQ(rule::EQ('\\'), rule::ANY()), rule::BUT(rule::EQ('"')))), rule::EQ('"') }));
			library->addSymbol("comment", rule::SEQ({ rule::EQ('/'), rule::OR(rule::SEQ(rule::EQ('/'),   rule::UNTIL(rule::NEW_LINE())),rule::SEQ({rule::EQ('*'), rule::UNTIL(rule::EQ("*/")), rule::EQ("*/")})) }));
			library->addSymbol("alias", rule::SEQ({ rule::RULE("_identifier"),
					rule::EQ('='),
					rule::RULE("_identifier") }));
			library->addPart("aliasOrIdentifier", rule::OR({ rule::RULE("alias"),rule::RULE("_identifier") }));
			library->addSymbol("aliasList", rule::OR({ rule::RULE("aliasOrIdentifier"),
				rule::SEQ({ 
					rule::EQ('('),
					rule::OR( 
						rule::RULE("aliasOrIdentifier"),
						rule::RULE("_aliasList")),
					rule::REP(rule::SEQ({
						rule::EQ(','),
						rule::OR(
							rule::RULE("aliasOrIdentifier"),
							rule::RULE("_aliasList"))
						})),
					rule::EQ(')')
					})
				}));
			library->addSymbol("use", rule::SEQ({ rule::EQ("use") , rule::RULE("wsp*") , rule::OPT(rule::RULE("aliasList")), rule::RULE("lineEnd+") }));
			return library;
		};


	}
}
#endif