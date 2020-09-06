
/*
 * Copyright 2020 John Orlando Keleshian Moxley
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
#include "Tokenizer.h"
#include <iostream>
namespace flock {
	namespace tokenizer {

		void Tokenizer::fetch()
		{
			int nextChar = getchar();
			char_at++;
			column++;

			charQueue.emplace(charQueue.end(), line, column, nextChar);
			if (nextChar == '\n' || nextChar == '\r') {
				line++;
				column = 0;
			}
		}

		int Tokenizer::current() {
			return currentLocation().getSourceChar();
		}

		SourceLocation Tokenizer::currentLocation() {
			if (charQueue.empty()) {
				fetch();
			}
			return charQueue.front();
		}

		void Tokenizer::pop() {
			if (!charQueue.empty()) {
				charQueue.pop_front();
			}
		}
		int Tokenizer::next() {
			return nextLocation().getSourceChar();
		}
		int Tokenizer::next(int idx) {
			return nextLocation(idx).getSourceChar();
		}
		SourceLocation Tokenizer::nextLocation() {
			return nextLocation(1);
		}
		SourceLocation Tokenizer::nextLocation(int idx) {
			if (idx == 0) {
				return currentLocation();
			}
			for(int i = charQueue.size(); i <= idx;i++) {
				fetch();
			}
			return charQueue.at(idx);
		}

		void Tokenizer::advance(std::string& sourceString)
		{
			sourceString += current();
			pop();
		}

		//todo make this a char stream.
		Tokenizer::Tokenizer() {
			line = 1;
			column = 0;
			char_at = 0;
		}

		std::unique_ptr<Token> Tokenizer::nextToken() {
			SourceLocation start = currentLocation();
			std::string sourceString;

			if (isspace(current())) {
				while (isspace(current())) {
					advance(sourceString);
				}

				Source mySource(sourceString, start, currentLocation());
				Whitespace_Token whitepsace_Token(mySource);
				return std::make_unique<Whitespace_Token>(whitepsace_Token);
			}
			if (current() == '/') {
				int nextChar = next();
				if (nextChar == '/') {
					int currentLine = currentLocation().getLine();
					advance(sourceString);
					do {
						advance(sourceString);
					} while (currentLine == nextLocation().getLine());
					advance(sourceString);

					Source mySource(sourceString, start, currentLocation());
					Comment_Token comment_Token(mySource);
					return std::make_unique<Comment_Token>(comment_Token);
				}
				else if (nextChar == '*') {

					advance(sourceString);
					do {
						advance(sourceString);
					} while (!(current() == '*' && next() == '/'));
					advance(sourceString); 
					advance(sourceString);

					Source mySource(sourceString, start, currentLocation());
					Comment_Token comment_Token(mySource);
					return std::make_unique<Comment_Token>(comment_Token);
				}
			}

			if (isalpha(current())) {

				// TODO support underscores
				while (isalnum(current())) {
					advance(sourceString);
				}
				Source mySource(sourceString, start, currentLocation());
				Identifier_Token identifier_token(mySource);
				return std::make_unique<Identifier_Token>(identifier_token);
				//TODO do keyword detection here.
			}
			if (isdigit(current()) || current() == '.') { // Number: [0-9.]+
				while(isdigit(current()) || current() == '.') {
					advance(sourceString);
				};
				Source mySource(sourceString, start, currentLocation());
				Number_Token number_token(mySource);
				return std::make_unique<Number_Token>(number_token);
			}

			Source mySource("unknown", start, currentLocation());
			EOF_Token eof_token(mySource);
			return std::make_unique<EOF_Token>(eof_token);
		}

	};
}

