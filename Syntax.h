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
#ifndef FLOCK_COMPILER_SYNTAX_H
#define FLOCK_COMPILER_SYNTAX_H

#include "Source.h"
#include "Visitor.h"

 ///
 /// Basic Grammar, that allows to employ basic BNF style grammars in language detection.
 /// 
namespace flock {
	namespace syntax {
		using namespace flock::source;
		class SyntaxNode;
		template<typename IN, typename OUT>
		class SyntaxVisitor;

		class SyntaxLibrary;

		template<typename IN, typename OUT>
		using SyntaxStrategy = visitor::Strategy<IN, OUT, SyntaxNode, SyntaxVisitor<IN, OUT>>;

		template<typename IN, typename OUT>
		using LibrarySyntaxStrategy = visitor::LibraryStrategy<IN, OUT, SyntaxNode, SyntaxVisitor<IN, OUT>, SyntaxLibrary>;
		template<typename IN, typename OUT>
		using SyntaxStrategies = visitor::BaseStrategies<IN, OUT, SyntaxNode, SyntaxStrategy<IN, OUT>, LibrarySyntaxStrategy<IN, OUT>>;



		class SyntaxNode : public enable_shared_from_this<SyntaxNode> {
		public:
			SyntaxNode(string type) : type(type), range(nullptr) {}
			SyntaxNode(_sp<Range> range) : range(range) {}
			SyntaxNode(string type, _sp<Range> range) : type(type), range(range) {}

			_sp<SyntaxNode> clone() {
				_sp<SyntaxNode> me = make_shared<SyntaxNode>(type, range);

				for (_sp<SyntaxNode> child : SyntaxNode::children) {
					me->append(child->clone());
				}
				return me;
			}
			_sp_vec<SyntaxNode> getChildren() {
				return children;
			}
			_sp<Range> getRange() {
				return range;
			}
			_sp<SyntaxNode> getParent() {
				return parent;
			}

			void setParent(_sp<SyntaxNode> parentToSet) {
				parent = parentToSet;
			}
			void setRange(_sp<Range> newRange) {
				range = newRange;
			}

			void append(_sp<SyntaxNode> syntaxNode) {
				children.push_back(syntaxNode);
				syntaxNode->setParent(this->shared_from_this());
			}

			/// <summary>
			/// TODO probably remove
			/// </summary>
			/// <returns></returns>
			_sp<Range> getFullRange() {
				if (range == nullptr) {
					range = getCombinedChildrenRange();
				}
				return range;
			}

			friend std::ostream& operator<<(std::ostream& os, const SyntaxNode& node) {
				string printRange = node.range ? ": " + node.range->source : "";
				os << "{ " << node.type << printRange;
				if (!node.children.empty()) {

					os << ": ";
					if (node.children.size() > 1) {
						os << "[";
					}
					for (_sp<SyntaxNode> child : node.children) {
						os << *child;
					}
					if (node.children.size() > 1) {
						os << "]";
					}
				}
				return os << " }";
			};

		protected:
			string type;
			_sp<Range> range = nullptr;
			_sp_vec<SyntaxNode> children;
			_sp<SyntaxNode> parent = nullptr;


			/// <summary>
			/// TODO probably remove
			/// </summary>
			/// <returns></returns>
			_sp<Range> getCombinedChildrenRange() {

				if (children.empty()) {
					return nullptr;
				}
				auto option = children.at(0);
				if (!option || option->range) {
					return nullptr;
				}
				_sp<Range> range = option->range;
				for (int i = 1; i < children.size(); i++) {
					auto option = children.at(i);

					if (!option || option->range) {
						return range;
					}
					range = std::make_shared<Range>(Range(range, option->range));
				}
				return range;
			}
		};
	}
}

#endif