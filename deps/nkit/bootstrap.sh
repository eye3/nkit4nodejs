#!/bin/bash

#   Copyright 2014 Vasiliy Soshnikov (dedok.mad@gmail.com)
#                  Boris T. Darchiev (boris.darchiev@gmail.com)
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

cleanup()
{
    unset BOOST_ROOT
    unset YAJL_ROOT
    unset EXPAT_ROOT
    unset USE_BOOST
    unset USE_REF_COUNT_PTR
    unset USE_VX
    unset PREFIX
}


exit_success()
{
   cleanup
   exit 0
}


die()
{
    cleanup
    echo "bootstrap.sh failed, reason : $@" 1>&2 && exit 1
}


usage()
{
    cat << EOF
usage      :  $0 [-d|-rd|-r] [--with-boost=] [--use-boost]

--help | -h             - print this message
--debug | -d            - set build type to debug
--release | -r          - set build type to release
--rdebug | -rd          - set build type to release with debug info
--build-root            - set build root for (Release|Debug|RelWithDebInfo)_build folders
--with-[boost|yajl]=    - path to non-system Boost|yajl.
--with-boost            - force to use system Boost (if any)
--boost-version         - force to use concrete boost version
--use-refcount-ptr      - force NOT to use boost::shared_ptr or std::shared_ptr, use nkit::ref_count_ptr instead
--with-vx               - add 'vx' functionality support (needs EXPAT library)
--with-expat=           - path to non-system expat.
--with-pic              - set -fPIC flag for gcc (to use libnkit.a in dynamic libraries (*.so)
--prefix=               - set install prefix[default '/usr/local']
--cmake-flags=          - add cmake flag
--with-cmake=           - set cmake
--perf                  - build performance tests

If no '--with-boost' option has been provided bootstrap.sh will try to find:
- gcc compiler of version >= 4.7 and, if there is no
- system Boost library

examples

./bootstrap.sh --prefix=/path/to/installation/folder --with-boost=/path/to/boost --with-yajl=/path/to/yajl --debug
./bootstrap.sh --prefix=/path/to/installation/folder --with-boost --with-yajl=/path/to/yajl --release
./bootstrap.sh --prefix=/path/to/installation/folder --with-yajl=/path/to/yajl --rdebug


Some influential environment variables:
	PREFIX		Install prefix[default /usr/local]
	CC			C compiler command
	CFLAGS		C compiler flags
	LDFLAGS		linker flags, e.g. -L<lib dir> if you have libraries in a
				nonstandard directory <lib dir>
	LIBS		libraries to pass to the linker, e.g. -l<library>
	CPPFLAGS	C/C++ preprocessor flags, e.g. -I<include dir>,
				if you have headers in a nonstandard directory <include dir>
	CXX			C++ compiler command
	CXXFLAGS	C++ compiler flags
	CPP			C preprocessor
	CXXCPP		C++ preprocessor

	BOOST_ROOT	      - Path to boost library (by default searches in 'sysroot')
	YAJL_ROOT	      - Path to YAJL library (by default searches in 'sysroot')
	EXPAT_ROOT	      - Path to EXPAT library (by default searches in 'sysroot')
	USE_VX            - add 'vx' functionality support (needs EXPAT library)
	USE_BOOST	      - Force using boost instead of C++11 (if there is C++11 support)
	USE_REF_COUNT_PTR - Force NOT to use boost::shared_ptr or std::shared_ptr, use nkit::ref_count_ptr instead
	CMAKE_FLAGS	      - cmake flags
EOF
}


CMAKE="env cmake"
REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_ROOT=${REPO_ROOT}
BUILD_TYPE=RelWithDebInfo
cleanup


for option; do
	case $option in
		--help    | -h)
			usage && exit_success
			;;
		--debug   | --Debug)
			BUILD_TYPE=Debug
			;;
		--rdebug  | --RelWithDebInfo)
			BUILD_TYPE=RelWithDebInfo
			;;
		--release | --Release)
			BUILD_TYPE=Release
			;;
		--build-root=*)
			export BUILD_ROOT=`expr "x$option" : "x--build-root=\(.*\)"`
			;;
		--with-boost=*)
			export BOOST_ROOT=`expr "x$option" : "x--with-boost=\(.*\)"`
			export USE_BOOST=1
			;;
		--with-yajl=*)
			export YAJL_ROOT=`expr "x$option" : "x--with-yajl=\(.*\)"`
			;;
		--with-boost)
			export USE_BOOST=1
			;;
		--use-refcount-ptr)
			export USE_REF_COUNT_PTR=1
			;;
		--with-vx)
			export USE_VX=1
			;;
		--with-expat=*)
			export EXPAT_ROOT=`expr "x$option" : "x--with-expat=\(.*\)"`
			;;
		--boost-version=*)
			export BOOST_VERSION=`expr "x$option" : "x--boost-version=\(.*\)"`
			;;
        --prefix=*)
			export PREFIX=`expr "x$option" : "x--prefix=\(.*\)"`
			;;
		--cmake-flags=*)
			CMAKE_FLAGS="$CMAKE_FLAGS `expr "x$option" : "x--cmake-flags=\(.*\)"`"
			;;
		--with-pic*)
			export CXXFLAGS="-fPIC $CXXFLAGS"
			;;
		--with-cmake=*)
			CMAKE=`expr "x$option" : "x--with-cmake=\(.*\)"`
			;;
		--perf)
			CMAKE_FLAGS="-DWITH_PERF=1 $CMAKE_FLAGS"
			;;
		* )
			die "unknown CLI parameter '$option', use --help|-h for help"
			;;
	esac
done

CMAKE_FLAGS="$CMAKE_FLAGS -Wno-dev"

mkdir -p $BUILD_ROOT/$BUILD_TYPE-build || \
  die "mkdir failed, path '$BUILD_ROOT/$BUILD_TYPE-build'"

cd $BUILD_ROOT/$BUILD_TYPE-build
$CMAKE "$CMAKE_FLAGS" "$REPO_ROOT" -DCMAKE_BUILD_TYPE=$BUILD_TYPE || \
      (cd - && die "cmake failed, error code $?")
cd -

echo "bootstrap.sh done, type make -C $BUILD_TYPE-build"

exit_success

