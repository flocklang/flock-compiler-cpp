#include <string>

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
		private:
			Source source;
		public:
			Source  getSource() {
				return source;
			}
			Token(Source source) : source(std::move(source)) {}
		};

		class Tokenizer{
		public:
			Token nextToken() {
				std::string sourceText = "MySource";
				SourceLocation start(1, 2);
				SourceLocation end(3, 4);
				Source mySource(sourceText, SourceLocation(start), SourceLocation(end));
				Token token(mySource);
				return token;
			}
		};
	}
}

