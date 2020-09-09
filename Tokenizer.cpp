
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
			int nextChar = supplier.supply();
			char_at++;
			column++;

			charQueue.emplace(charQueue.end(), line, column, nextChar);
			if (nextChar == '\n' || nextChar == '\r') {
				line++;
				column = 0;
			}
		}
		SourceLocation Tokenizer::poll(const int idx) {
			for (int i = charQueue.size(); i <= idx; i++) {
				fetch();
			}
			return charQueue.at(idx);
		}

		int Tokenizer::pollChar(const int idx) {
			return poll(idx).getSourceChar();
		}

		/**
		* Start Index inclusive,
		*/
		std::string Tokenizer::pollString(const int amount, const int startIdx) {
			const int endIdx = startIdx + amount;
			std::string toCollect;
			for (int i = std::min(startIdx, (int)charQueue.size()); i < endIdx; i++) {
				if (i >= charQueue.size()) {
					fetch();
				}
				if (i >= startIdx) {
					toCollect += charQueue.at(i).getSourceChar();
				}
			}
			return toCollect;
		}

		std::string Tokenizer::pop(const int amount) {
			std::string sourceString;
			sourceString += pollString(amount);
			if (!charQueue.empty()) {
				charQueue.erase(charQueue.begin(), std::min(charQueue.begin() + amount, charQueue.end()));
			}
			return sourceString;
		}



		Tokenizer::Tokenizer(Supplier<int>& supplier) :supplier(supplier) {

			line = 1;
			column = 0;
			char_at = 0;
		}

		// Basic tokenizer
		TypedToken Tokenizer::nextToken() {
			const SourceLocation start = poll();
			if (pollChar() == EOF) {

				Source mySource("", start, start);
				EOF_Token eof_Token(mySource);
				return EOF_Token(eof_Token);
			}
			std::string sourceString;
			if (isnewline(pollChar())) {
				while (isnewline(pollChar())) {
					sourceString += pop();
				}

				Source mySource(sourceString, start, poll());
				NewLine_Token newLine_Token(mySource);
				return NewLine_Token(newLine_Token);
			}
			if (isblank(pollChar())) {
				while (isblank(pollChar())) {
					sourceString += pop();
				}

				Source mySource(sourceString, start, poll());
				Whitespace_Token whitespace_Token(mySource);
				return Whitespace_Token(whitespace_Token);
			}
			if (isequal(pollString(2), "//")) {
				sourceString += pop(2);
				while (!isnewline(pollChar())) {
					sourceString += pop();
				}

				Source mySource(sourceString, start, poll());
				Comment_Token comment_Token(mySource);
				return Comment_Token(comment_Token);
			}
			if (isequal(pollString(2), "/*")) {
				sourceString += pop(2);
				while (!isequal(pollString(2), "*/")) {
					sourceString += pop();
				}
				sourceString += pop(2);

				Source mySource(sourceString, start, poll());
				Comment_Token comment_Token(mySource);
				return Comment_Token(comment_Token);
			}

			if (isalpha(pollChar()) || pollChar() == '_') { // [A-Z_]

				while (isalnum(pollChar()) || pollChar() == '_') { // [A-Z0-9_]+
					sourceString += pop();
				}
				Source mySource(sourceString, start, poll());
				String_Token identifier_token(mySource);
				return String_Token(identifier_token);
			}
			if (isdigit(pollChar())) { // [0-9.]+  we support [12.34, 1234] but not [.1234]
				bool isDecimal = false;
				sourceString += pop();
				for(int currentChar = pollChar() ; isdigit(currentChar) || currentChar == '.' ; currentChar = pollChar()){// [0-9.]+
					if (currentChar == '.') {
						if (isDecimal || !isdigit(pollChar(1))) {
							break;
						}
						isDecimal = true;
					}
					sourceString += pop();
				}
				Source mySource(sourceString, start, poll());
				Number_Token number_token(mySource);
				return Number_Token(number_token);
			}

			if (ispunct(pollChar())) { // everything else
				sourceString += pop();
				Source mySource(sourceString, start, start);
				Symbol_Token symbol_token(mySource);
				return Symbol_Token(symbol_token);
			}

			// Safety
			Source mySource(pop(), start, start);
			EOF_Token eof_token(mySource);
			return EOF_Token(eof_token);
		}

		std::ostream& operator<<(std::ostream& os, const SourceLocation& sourceLocation)
		{
			return os << "line: " << sourceLocation.line << ", column: " << sourceLocation.column << "";
		}
		//type:%s, value:'%s', start:{line: %i, column: %i}, end:{line: %i, column: %i}
		std::ostream& operator<<(std::ostream& os, const Source& source)
		{
			return os << "start: [" << source.start << "], end: [" << source.end << "], text: \"" << source.text << "\"";
		}

		//type:%s, value:'%s', start:{line: %i, column: %i}, end:{line: %i, column: %i}
		std::ostream& operator<<(std::ostream& os, const Token& token)
		{
			return os << "source: [" << token.source << "]";
		}


		std::ostream& operator<<(std::ostream& os, const TypedToken& token)
		{
			return os << "type: " << getTypeNameFor(token.type) << ", source: [" << token.source << "]";
		}

	}
}

