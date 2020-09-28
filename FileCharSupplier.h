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

#ifndef FLOCK_COMPILER_FILE_CHAR_SUPPLIER_H
#define FLOCK_COMPILER_FILE_CHAR_SUPPLIER_H
#include "Supplier.h"
#include <fstream>
#include <string>
namespace flock {
    namespace supplier {
        class FileCharSupplier : public Supplier<int>
        {
        public:
            int supply() override {
                if (myfile == nullptr) {
                    myfile = &std::ifstream(fileName, std::ifstream::in);
                }
                if ((*myfile).is_open()) {
                    if ((*myfile).eof()) {
                        (*myfile).close();
                        return  -1;
                    }
                    // We may want to buffer for effeciency
                    int next = (*myfile).get();
                    if (next == -1) {
                        (*myfile).close();
                    }
                    return next;
                }
                return -1;
            }
            FileCharSupplier(std::string fileName) : fileName (fileName){ }
        private:
            /*
            * Basic Safety, close in case we are not using it.
            */
            ~FileCharSupplier() {
                if ((*myfile).is_open()) {
                    (*myfile).close();
                }
                delete myfile;
            }
            std::string fileName;
            std::ifstream *myfile = nullptr;
        };

    }
}
#endif