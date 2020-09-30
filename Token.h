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
#ifndef FLOCK_COMPILER_TOKEN_H
#define FLOCK_COMPILER_TOKEN_H
#include <memory>
#include <vector>
#include "Supplier.h"
namespace flock {
    using namespace supplier;
    namespace token {

        template<typename Type, typename Contents>
        class Token {
        public:
            Token(Type type, Contents contents) : type(type) , contents(contents){}
            Type getType()
            {
               return type;
            }
            Contents getContents()
            {
                return contents;
            }
        private:
            const Type type;
            const Contents contents;
        };
    }
}
#endif
