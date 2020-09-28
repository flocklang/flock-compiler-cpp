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
#ifndef FLOCK_COMPILER_OLD_TOKENIZER_H
#define FLOCK_COMPILER_OLD_TOKENIZER_H

#include <string>
#include <memory>
#include <deque>
#include <iostream>
#include "Source.h"

namespace flock {

	template <class T>
	class Supplier {
	public:
		virtual T supply() = 0;
	};


	class ConsoleSupplier : public Supplier<int> {
	public:
		int supply() override {
			return getchar();
		};
	};

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
				String,
				Number,
				Symbol
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
				case Type::String:
					return "String";
				case Type::Number:
					return "Number";
				case Type::Symbol:
					return "Symbol";
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

		class Tokenizer : public Supplier<std::unique_ptr<TypedToken>> {
		public:
			Tokenizer(Supplier<int>& supplier);
			virtual ~Tokenizer() = default;
			std::unique_ptr<TypedToken> supply() override {
				return std::make_unique<TypedToken>(nextToken());
			}
		private:
			int line;
			int column;
			int char_at;
			Supplier<int> &supplier;
			std::deque<SourceLocation> charQueue;
			void fetch();

			TypedToken nextToken();
			SourceLocation poll(const int idx = 0);
			int pollChar(const int idx = 0);
			std::string pollString( const int amount = 1, const int startIdx = 0);
			std::string pop(const int amount = 1);
		};



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

		class String_Token : public TypedToken {
		public:
			String_Token(Source source) : TypedToken(source, Type::String) {}
		};

		class Number_Token : public TypedToken {
		public:
			Number_Token(Source source) : TypedToken(source, Type::Number) {}
		};

		class Symbol_Token : public TypedToken {
		public:
			Symbol_Token(Source source) : TypedToken(source, Type::Symbol) {}
		};

	}
}
#endif
