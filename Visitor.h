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
#ifndef FLOCK_COMPILER_NODE_VISITOR_H
#define FLOCK_COMPILER_NODE_VISITOR_H

#include "Util.h"
#include <memory>
#include <vector>
#include <map>
#include <string>

 ///
 /// A Reusable VisitorStrategyPattern.
 /// 
namespace flock {
	namespace visitor {
		using namespace std;

		template<typename NODE>
		using NodeMap = map<const string, _sp<NODE>>;

		template<typename NODE>
		class Library;
		template<typename IN, typename OUT, typename NODE, typename STRATEGY, typename LIBRARY_STRATEGY, typename DERIVED>
		class Visitor;

		template<typename IN, typename OUT, typename NODE, typename VISITOR>
		class Strategy;
		template<typename IN, typename OUT, typename NODE, typename VISITOR, typename LIBRARY >
		class LibraryStrategy;
		template<typename IN, typename OUT, typename NODE, typename STRATEGY, typename LIBRARY_STRATEGY >
		class Strategies;
		template<typename IN, typename OUT, typename NODE, typename STRATEGY, typename LIBRARY_STRATEGY >
		class BaseStrategies;

		template<typename NODE>
		class Library {
		public:
			_sp<NODE> addNode(const string name, _sp <NODE> node) {
				nodes.emplace(name, node);
				names.push_back(name);
				return node;
			}

			_sp<NODE> getNode(const string name) {
				auto it = nodes.find(name);
				if (it == nodes.end()) {
					return nullptr;
				}
				return it->second;
			}

			vector<string> getNames() {
				return names;
			}
		protected:
			vector<string> names;
			NodeMap<NODE> nodes;
		};

		template<typename IN, typename OUT, typename NODE, typename STRATEGY, typename LIBRARY_STRATEGY>
		class Strategies {
		public:
			virtual ~Strategies() = default;
			virtual _sp<STRATEGY> getStrategyById(const int typeId) = 0;
			virtual _sp<STRATEGY> getStrategy(_sp<NODE> node) {
				return getStrategyById(node->type);
			}
			virtual void addStrategy(const int type, _sp<STRATEGY> strategy) = 0;
			virtual _sp<LIBRARY_STRATEGY> getLibraryStrategy() = 0;
			virtual void setLibraryStrategy(_sp<LIBRARY_STRATEGY> strategy) = 0;
			virtual void clear() {
			}
		};

		template<typename IN, typename OUT, typename NODE, typename STRATEGY, typename LIBRARY_STRATEGY>
		class BaseStrategies : public Strategies<IN, OUT, NODE, STRATEGY, LIBRARY_STRATEGY> {
		public:
			BaseStrategies() {}
			virtual _sp<STRATEGY> getStrategyById(const int typeId) override {
				auto it = strategyMap.find(typeId);
				if (it == strategyMap.end()) {
					return nullptr;
				}
				return it->second;
			}

			virtual void addStrategy(const int type, _sp<STRATEGY> strategy)  override {
				strategyMap.emplace(type, strategy);
			}

			virtual _sp<LIBRARY_STRATEGY> getLibraryStrategy()  override {
				return libraryStrategy;
			}

			virtual void setLibraryStrategy(_sp<LIBRARY_STRATEGY> strategy) override {
				libraryStrategy = strategy;
			}
		protected:
			map<int, _sp<STRATEGY>> strategyMap;
			_sp<LIBRARY_STRATEGY> libraryStrategy;
		};

		template<typename IN, typename OUT, typename NODE, typename STRATEGY, typename LIBRARY_STRATEGY>
		class WrappingStrategies : public Strategies<IN, OUT, NODE, STRATEGY, LIBRARY_STRATEGY > {
		public:
			WrappingStrategies(_sp<Strategies<IN, OUT, NODE, STRATEGY, LIBRARY_STRATEGY>> strategies) : strategies(strategies) {}

			virtual _sp<STRATEGY> getStrategyById(const int typeId) override {
				return strategies->getStrategyById(typeId);
			}

			virtual _sp<STRATEGY> getStrategy(_sp<NODE> node) {
				return strategies->getStrategy(node);
			}

			virtual void addStrategy(const int type, _sp<STRATEGY> strategy)  override {
				strategies->addStrategy(type, strategy);
			}

			virtual _sp<LIBRARY_STRATEGY> getLibraryStrategy()  override {
				return strategies->getLibraryStrategy();
			}

			virtual void setLibraryStrategy(_sp<LIBRARY_STRATEGY> strategy) override {
				strategies->setLibraryStrategy(strategy);
			}

			virtual _sp<Strategies<IN, OUT, NODE, STRATEGY, LIBRARY_STRATEGY>> getWrappedStratagies() {
				return strategies;
			}
			virtual void clear() override {
				strategies->clear();
			}
		protected:
			_sp<Strategies<IN, OUT, NODE, STRATEGY, LIBRARY_STRATEGY>> strategies;
		};

		template<typename IN, typename OUT, typename NODE, typename VISITOR>
		class Strategy {
		public:
			virtual ~Strategy() = default;
			virtual OUT accept(_sp<VISITOR> visitor, _sp<NODE> node, IN input) = 0;
		};

		template<typename IN, typename OUT, typename NODE, typename VISITOR, typename LIBRARY>
		class LibraryStrategy {
		public:
			virtual ~LibraryStrategy() = default;
			virtual OUT accept(_sp<VISITOR> visitor, _sp<LIBRARY> library, IN input) = 0;
		};

		/// <summary>
		/// Sometime we may just want to wrap a strategy, to implement common functionality, for instance history.
		/// </summary>
		/// <param name="wrapped"></param>
		/// <returns></returns>
		template<typename IN, typename OUT, typename NODE, typename VISITOR >
		class WrappingStrategy : public Strategy <IN, OUT, NODE, VISITOR> {
		public:
			WrappingStrategy(_sp<Strategy <IN, OUT, NODE, VISITOR>> wrapped) : wrapped(wrapped) {}

			_sp<Strategy <IN, OUT, NODE, VISITOR>> getWrapped() {
				return wrapped;
			}
		protected:
			_sp<Strategy <IN, OUT, NODE, VISITOR>> wrapped;
		};

		/// <summary>
		/// Sometime we may just want to wrap a strategy, to implement common functionality, for instance history.
		/// </summary>
		/// <param name="wrapped"></param>
		/// <returns></returns>
		template<typename IN, typename OUT, typename NODE, typename VISITOR, typename LIBRARY>
		class WrappingLibraryStrategy : public LibraryStrategy <IN, OUT, NODE, VISITOR, LIBRARY> {
		public:
			WrappingLibraryStrategy(_sp<LibraryStrategy <IN, OUT, NODE, VISITOR, LIBRARY>> wrapped) : wrapped(wrapped) {}

			_sp<LibraryStrategy <IN, OUT, NODE, VISITOR, LIBRARY>> getWrapped() {
				return wrapped;
			}
		protected:
			_sp<LibraryStrategy <IN, OUT, NODE, VISITOR, LIBRARY>> wrapped;
		};

		/// <summary>
		/// The sole job of the visitor is to glue the aenimic nodes to the strategys, whose job it is to navigate the visitor up and doen the tree.
		/// </summary>
		/// <typeparam name="IN"></typeparam>
		/// <typeparam name="OUT"></typeparam>
		template<typename IN, typename OUT, typename NODE, typename LIBRARY, typename STRATEGIES, typename DERIVED>
		class Visitor : public std::enable_shared_from_this<DERIVED> {
		public:
			Visitor(_sp<LIBRARY> library, _sp<STRATEGIES> strategies) : library(library), strategies(strategies) {}

			virtual OUT visit(_sp<NODE> node, IN input) {
				auto strategy = strategies->getStrategy(node);
				return strategy->accept(this->shared_from_this(), node, input);
			}

			virtual OUT visitByName(const string name, IN input) {
				_sp<NODE> node = getNode(name);
				if (node) {
					return this->visit(node, input);
				}
				throw string("Node " + name + " does not exist");
			}

			virtual OUT begin(IN input) {
				auto strategy = strategies->getLibraryStrategy();
				return strategy->accept(this->shared_from_this(), library, input);
			}

			virtual _sp<NODE> getNode(const string name) {
				return library->getNode(name);
			}

			virtual void clear() {
			}
			_sp<LIBRARY> getLibrary() {
				return library;
			}

			_sp<STRATEGIES> getStrategies() {
				return strategies;
			}
		protected:
			_sp<LIBRARY> library;
			_sp<STRATEGIES> strategies;
		};

	}

}
#endif