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
#include <string>
#include <memory>

namespace flock {
	namespace tokenizer {
		class SourceLocation {
		private:
			int line;
			int column;
		public:
			int getLine() {
				return line;
			}
			int getColumn() {
				return column;
			}
			SourceLocation(int line, int column) : line(line), column(column) {}
		};

		class Source {
		private:
			std::string text;
			SourceLocation start, end;

		public:
			SourceLocation getStart() {
				return start;
			}
			SourceLocation getEnd() {
				return end;
			}
			std::string getText() {
				return text;
			}
			Source(const std::string text, SourceLocation start, SourceLocation end)
				: text(std::move(text)), start(std::move(start)), end(std::move(end)) {
			}
		};

	

		class Token {
		protected:
			Source source;
		public:
			enum class Type {
				Eof,
				Identifier,
				Number
			};
			virtual ~Token() = default;

			
			Source  getSource() {
				return source;
			}

			virtual Type getType() = 0;

			Token(Source source) : source(std::move(source)){}

		};


		class EOF_Token : public Token {
		public:
			Type getType() override {
				return Type::Eof;
			}

			EOF_Token(Source source) : Token(source) {}
		};

		class Identifier_Token : public Token {
		public:
			Type getType() override {
				return Type::Identifier;
			}
			Identifier_Token(Source source) : Token(source) {}
		};

		class Number_Token : public Token {
		public:
			Type getType() override {
				return Type::Number;
			}
			Number_Token(Source source) : Token(source) {}
		};


		class Tokenizer{
		private:
			int line;
			int column;
			int char_at;

			int getNextChar() {
				int nextChar = getchar();
				char_at++;
				if (nextChar == '\n' || nextChar == '\r') {
					line++;
					column = 0;
				} else {
					column++;
				}
				return nextChar;
			}


		public:

			//todo make this a char stream.
			Tokenizer() {
				line = 1;
				column = 0;
				char_at = 0;
			}
			virtual ~Tokenizer() = default;

			std::unique_ptr<Token> nextToken() {
				int currentChar = ' ';
				while (isspace(currentChar)) {
					currentChar = getNextChar();
				}
				SourceLocation start(line, column);
				SourceLocation end = start;

				if (isalpha(currentChar)) {
					std::string identifierString;
					identifierString = currentChar;
					
					// TODO support underscores
					while (isalnum((currentChar = getNextChar()))) {
						identifierString += currentChar;
						end = SourceLocation(line, column);
					}
					Source mySource(identifierString, start, end);
					Identifier_Token identifier_token(mySource);
					return std::make_unique<Identifier_Token>(identifier_token);
					//TODO do keyword detection here.
				}
				if (isdigit(currentChar) || currentChar == '.') { // Number: [0-9.]+
					std::string numberString;
					do {
						numberString += currentChar;
						end = SourceLocation(line, column);
						currentChar = getNextChar();
					} while (isdigit(currentChar) || currentChar == '.');
					Source mySource(numberString, start, end);
					Number_Token number_token(mySource);
					return std::make_unique<Number_Token>(number_token);
				}

				Source mySource("unknown", start, end);
				EOF_Token eof_token(mySource);
				return std::make_unique<EOF_Token>(eof_token);
			}
		};
	}
}

