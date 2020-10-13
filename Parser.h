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
#ifndef FLOCK_COMPILER_PARSER_H
#define FLOCK_COMPILER_PARSER_H
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include "CompilerFix.h"
#include "AST.h"
#include "CachedSupplier.h"

 //===----------------------------------------------------------------------===//
 // Parser Using LLVM Kalediscope Tutorial as Base
 //===----------------------------------------------------------------------===//
using namespace std;
namespace flock {
	using namespace ast;
	namespace parser {

		static const int END_OF_EXPRESSION = ';';

		//using Tokens = CachedSupplier<LexToken>*;

		//static unique_ptr<BlockExprAST> handleRoot(Tokens tokens) {

		//	_up_vec<ExprAST> expressions;
		//	while (true) {
		//		_sp<LexToken> currentToken = tokens->pop();
		//		if (!currentToken) { // if it is a nullptr, we are done.
		//			return make_unique<BlockExprAST>(BlockExprAST(expressions));
		//		}
		//		switch (currentToken->getType()) {
		//		case LexType::Eof:
		//			return make_unique<BlockExprAST>(BlockExprAST(expressions));
		//			// Eof.
		//		case LexType::Identifier: { // ignore top-level semicolons. 

		//			string identifier = currentToken->getString();
		//			if (identifier == "use") {
		//				_up<ExprAST> expression = handleImport(tokens);
		//				if (expression) {
		//					expressions.push_back(move(expression));
		//				}
		//			}
		//			else if (identifier == "") {
		//			}
		//			break;
		//		}
		//								/*case tok_def:
		//									HandleDefinition();
		//									break;
		//								case tok_extern:
		//									HandleExtern();
		//									break;*/
		//		case LexType::Comment:
		//		case LexType::Whitespace:
		//		case LexType::Unknown:
		//		default:
		//			HandleTopLevelExpression(); // move up
		//			break; // just ignore.
		//		}
		//	}
		//}
		/**
		* deconstructoring is of the form
		* (a, b) = some_struct
		* we also want to support
		* (x:a, y:b) = some struct in this case ':' means 'in'
		* we can also go several layers down (a,(c,d):b)
		*/
		static _up_vec<ExprAST> handleDestructuring() {
		}
		/**
		* Imports are of the form
		* use import_expression; // we are importing everything an we alias it at the current level.
		* or
		* use a import_expression; // in this case a is the alias for the whole import
		* or
		* use (deconstruction_args) import_expression; // in this case we are aliasing a subset of the import
		* 
		* This means if we want to explicity support exports 
		* >> might make sense.
		*/
		//static unique_ptr<ImportExprAST> handleImport(Tokens tokens) {
		//	ignoreWhitespace(tokens);
		//	_sp<LexToken> next = tokens->poll();
		//	if (next) {
		//		vector<string> args;
		//		string source;
		//		if (next->getType() == LexType::Symbol) {
		//			// TOOO: collect arguments
		//		}
		//		ignoreWhitespace(tokens);
		//		while (next && !(next->getType() == LexType::Symbol && next->getChar() == END_OF_EXPRESSION)) {
		//			// collect path TODO: either specfify an expression or check its a proper string.
		//			source += next->getString();
		//			ignoreWhitespace(tokens);
		//		}
		//		tokens->pop();
		//		return make_unique<ImportExprAST>(ImportExprAST(source, args));
		//	}
		//	return nullptr;
		//}


		//static void ignoreWhitespace(Tokens tokens) {
		//	_sp<LexToken> next = tokens->poll();
		//	while (next && next->getType() == LexType::Whitespace) {
		//		tokens->pop();
		//		next = tokens->poll();
		//	}
		//}
		//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.

		static std::string IdentifierStr; // Filled in if tok_identifier
		static double NumVal;             // Filled in if tok_number
		enum Token {
			tok_eof = -1,

			// commands
			tok_def = -2,
			tok_extern = -3,

			// primary
			tok_identifier = -4,
			tok_number = -5
		};
		/// taken from LLVM tutorial we won't need these


		static int CurTok;
		static int getNextToken() { return CurTok = -1; }

		/// BinopPrecedence - This holds the precedence for each binary operator that is
		/// defined.
		static map<char, int> binopPrecedence = declareBinopPreceence();

		static map<char, int> declareBinopPreceence() {
			map<char, int> prec;
			// Install standard binary operators.
// 1 is lowest precedence.
			prec['<'] = 10;
			prec['+'] = 20;
			prec['-'] = 20;
			prec['*'] = 40; // highest
			return prec;

		}


		/// GetTokPrecedence - Get the precedence of the pending binary operator token.
		static int GetTokPrecedence() {
			if (!__isascii(CurTok))
				return -1;

			// Make sure it's a declared binop.
			int TokPrec = binopPrecedence[CurTok];
			if (TokPrec <= 0)
				return -1;
			return TokPrec;
		}

		/// LogError* - These are little helper functions for error handling.
		std::unique_ptr<ExprAST> LogError(const char* Str) {
			fprintf(stderr, "Error: %s\n", Str);
			return nullptr;
		}
		std::unique_ptr<PrototypeAST> LogErrorP(const char* Str) {
			LogError(Str);
			return nullptr;
		}

		static std::unique_ptr<ExprAST> ParseExpression();

		/// numberexpr ::= number
		static std::unique_ptr<ExprAST> ParseNumberExpr() {
			auto Result = std::make_unique<DecimalExprAST>(NumVal);
			getNextToken(); // consume the number
			return std::move(Result);
		}

		/// parenexpr ::= '(' expression ')'
		static std::unique_ptr<ExprAST> ParseParenExpr() {
			getNextToken(); // eat (.
			auto V = ParseExpression();
			if (!V)
				return nullptr;

			if (CurTok != ')')
				return LogError("expected ')'");
			getNextToken(); // eat ).
			return V;
		}

		/// identifierexpr
		///   ::= identifier
		///   ::= identifier '(' expression* ')'
		static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
			std::string IdName = IdentifierStr;

			getNextToken(); // eat identifier.

			if (CurTok != '(') // Simple variable ref.
				return std::make_unique<VariableExprAST>(IdName);

			// Call.
			getNextToken(); // eat (
			std::vector<std::unique_ptr<ExprAST>> Args;
			if (CurTok != ')') {
				while (true) {
					if (auto Arg = ParseExpression())
						Args.push_back(std::move(Arg));
					else
						return nullptr;

					if (CurTok == ')')
						break;

					if (CurTok != ',')
						return LogError("Expected ')' or ',' in argument list");
					getNextToken();
				}
			}

			// Eat the ')'.
			getNextToken();

			return std::make_unique<CallExprAST>(IdName, std::move(Args));
		}

		/// primary
		///   ::= identifierexpr
		///   ::= numberexpr
		///   ::= parenexpr
		static std::unique_ptr<ExprAST> ParsePrimary() {
			switch (CurTok) {
			default:
				return LogError("unknown token when expecting an expression");
			case tok_identifier:
				return ParseIdentifierExpr();
			case tok_number:
				return ParseNumberExpr();
			case '(':
				return ParseParenExpr();
			}
		}

		/// binoprhs
		///   ::= ('+' primary)*
		static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
			std::unique_ptr<ExprAST> LHS) {
			// If this is a binop, find its precedence.
			while (true) {
				int TokPrec = GetTokPrecedence();

				// If this is a binop that binds at least as tightly as the current binop,
				// consume it, otherwise we are done.
				if (TokPrec < ExprPrec)
					return LHS;

				// Okay, we know this is a binop.
				int BinOp = CurTok;
				getNextToken(); // eat binop

				// Parse the primary expression after the binary operator.
				auto RHS = ParsePrimary();
				if (!RHS)
					return nullptr;

				// If BinOp binds less tightly with RHS than the operator after RHS, let
				// the pending operator take RHS as its LHS.
				int NextPrec = GetTokPrecedence();
				if (TokPrec < NextPrec) {
					RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
					if (!RHS)
						return nullptr;
				}

				// Merge LHS/RHS.
				LHS =
					std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
			}
		}

		/// expression
		///   ::= primary binoprhs
		///
		static std::unique_ptr<ExprAST> ParseExpression() {
			auto LHS = ParsePrimary();
			if (!LHS)
				return nullptr;

			return ParseBinOpRHS(0, std::move(LHS));
		}

		/// prototype
		///   ::= id '(' id* ')'
		static std::unique_ptr<PrototypeAST> ParsePrototype() {
			if (CurTok != tok_identifier)
				return LogErrorP("Expected function name in prototype");

			std::string FnName = IdentifierStr;
			getNextToken();

			if (CurTok != '(')
				return LogErrorP("Expected '(' in prototype");

			std::vector<std::string> ArgNames;
			while (getNextToken() == tok_identifier)
				ArgNames.push_back(IdentifierStr);
			if (CurTok != ')')
				return LogErrorP("Expected ')' in prototype");

			// success.
			getNextToken(); // eat ')'.

			return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
		}

		/// definition ::= 'def' prototype expression
		static std::unique_ptr<FunctionAST> ParseDefinition() {
			getNextToken(); // eat def.
			auto Proto = ParsePrototype();
			if (!Proto)
				return nullptr;

			if (auto E = ParseExpression())
				return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
			return nullptr;
		}

		/// toplevelexpr ::= expression
		static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
			if (auto E = ParseExpression()) {
				// Make an anonymous proto.
				auto Proto = std::make_unique<PrototypeAST>("__anon_expr",
					std::vector<std::string>());
				return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
			}
			return nullptr;
		}

		/// external ::= 'extern' prototype
		static std::unique_ptr<PrototypeAST> ParseExtern() {
			getNextToken(); // eat extern.
			return ParsePrototype();
		}

		//===----------------------------------------------------------------------===//
		// Top-Level parsing
		//===----------------------------------------------------------------------===//

		static void HandleDefinition() {
			if (ParseDefinition()) {
				fprintf(stderr, "Parsed a function definition.\n");
			}
			else {
				// Skip token for error recovery.
				getNextToken();
			}
		}

		static void HandleExtern() {
			if (ParseExtern()) {
				fprintf(stderr, "Parsed an extern\n");
			}
			else {
				// Skip token for error recovery.
				getNextToken();
			}
		}

		static void HandleTopLevelExpression() {
			// Evaluate a top-level expression into an anonymous function.
			if (ParseTopLevelExpr()) {
				fprintf(stderr, "Parsed a top-level expr\n");
			}
			else {
				// Skip token for error recovery.
				getNextToken();
			}
		}

		/// top ::= definition | external | expression | ';'
		static void MainLoop() {
			while (true) {
				fprintf(stderr, "ready> ");
				switch (CurTok) {
				case tok_eof:
					return;
				case ';': // ignore top-level semicolons.
					getNextToken();
					break;
				case tok_def:
					HandleDefinition();
					break;
				case tok_extern:
					HandleExtern();
					break;
				default:
					HandleTopLevelExpression();
					break;
				}
			}
		}
	}
}
#endif