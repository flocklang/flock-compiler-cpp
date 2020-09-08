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
#include <deque>
#include <iostream>

namespace flock {
	namespace tokenizer {
		class SourceLocation {
		private:
			int line;
			int column;
			int sourceChar;
		public:
			int getLine() {
				return line;
			}
			int getColumn() {
				return column;
			}
			int getSourceChar() {
				return sourceChar;
			}
			SourceLocation(int line, int column, int sourceChar) : line(line), column(column), sourceChar(sourceChar) {}
			SourceLocation(const SourceLocation& copy) = default;
			~SourceLocation() = default;
			friend std::ostream& operator<<(std::ostream& os, const SourceLocation& sourceLocation);
		};

		class Source {
		private:
			const std::string text;
			const SourceLocation start, end;
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
			Source(const Source& copy) = default;
			friend std::ostream& operator<<(std::ostream & os, const Source& source);
		};

		class Token {
		protected:
			const Source source;
		public:
			enum class Type {
				Eof,
				Whitespace,
				NewLine,
				Comment,
				Identifier,
				Number
			};
			virtual ~Token() = default;
			Token(const Token& copy) = default;
			Token(Source source) : source(std::move(source)) {}
			friend std::ostream& operator<<(std::ostream& os, const Token& token);
			virtual Type getType() = 0;

			friend std::string getTypeNameFor(Type type) {
				switch (type) {
				case Type::Eof:
					return "Eof";
				case Type::Whitespace:
					return "Whitespace";
				case Type::NewLine:
					return "NewLine";
				case Type::Comment:
					return "Comment";
				case Type::Identifier:
					return "Identifier";
				case Type::Number:
					return "Number";
				default:
					return "Unknown";
				}
			}
			std::string getTypeName() {
				return getTypeNameFor(getType());
			}

			Source getSource() {
				return source;
			}
		};

		class TypedToken : public Token {
		protected:
			const Type type;
		public:
			TypedToken(const TypedToken& copy) = default;
			Type getType() override {
				return type;
			}

			TypedToken(Source source, Type type) : Token(source), type(type) {}
			friend std::ostream& operator<<(std::ostream& os, const TypedToken& token);

		};

		class Tokenizer {
		public:
			//todo make this a char stream.
			Tokenizer();
			virtual ~Tokenizer() = default;
			std::unique_ptr<TypedToken> nextToken();
		private:
			int line;
			int column;
			int char_at;
			std::deque<SourceLocation> charQueue;
			void fetch();

			SourceLocation poll(const int idx = 0);
			int pollChar(const int idx = 0);
			std::string pollString( const int amount = 1, const int startIdx = 0);
			std::string pop(const int amount = 1);
		};



		static bool isnewline(const int character)
		{
			return isspace(character) && !isblank(character);
		}
		static bool isequal(const std::string from, const std::string to)
		{
			return from.compare(to) == 0;
		}

		class EOF_Token : public TypedToken {
		public:

			EOF_Token(Source source) : TypedToken(source, Type::Eof) {}
		};

		class Whitespace_Token : public TypedToken  {
		public:

			Whitespace_Token(Source source) : TypedToken(source, Type::Whitespace) {}
		};

		class NewLine_Token : public TypedToken {
		public:

			NewLine_Token(Source source) : TypedToken(source, Type::NewLine) {}
		};

		class Comment_Token : public TypedToken {
		public:

			Comment_Token(Source source) : TypedToken(source, Type::Comment) {}
		};

		class Identifier_Token : public TypedToken {
		public:
			Identifier_Token(Source source) : TypedToken(source, Type::Identifier) {}
		};

		class Number_Token : public TypedToken {
		public:
			Number_Token(Source source) : TypedToken(source, Type::Number) {}
		};

	}
}

