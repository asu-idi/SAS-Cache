#!/bin/sh

# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pull the newest version of gtest

git submodule sync

git submodule init

git submodule update

set -e

# Root directory for the CacheLib project
CLBASE="/data/gc/programs/CacheLib"

# Additional "FindXXX.cmake" files are here (e.g. FindSodium.cmake)
CLCMAKE="$CLBASE/cachelib/cmake"

# After ensuring we are in the correct directory, set the installation prefix"
PREFIX="$CLBASE/opt/cachelib/"

CMAKE_PARAMS="-DCMAKE_INSTALL_PREFIX='$PREFIX' -DCMAKE_MODULE_PATH='$CLCMAKE'"

CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}:$PREFIX/lib/cmake:$PREFIX/lib64/cmake"
# CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}:$PREFIX/lib/cmake:$PREFIX/lib64/cmake:$PREFIX/lib:$PREFIX/lib64:$PREFIX"
export CMAKE_PREFIX_PATH
# PKG_CONFIG_PATH="${PKG_CONFIG_PATH:-}:$PREFIX/lib/pkgconfig:$PREFIX/lib64/pkgconfig"
# export PKG_CONFIG_PATH
# LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}:$PREFIX/lib:$PREFIX/lib64"
# export LD_LIBRARY_PATH

rm -rf build
mkdir -p build
cd build
# cmake $CMAKE_PARAMS ..
cmake ..
# make -j32
make db_bench -j32
