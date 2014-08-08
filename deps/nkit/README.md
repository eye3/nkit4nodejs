# nkit c++ library

## Intro

C++ library with following components:

1. **Dynamic data typing**. It is realized in **Dynamic class** (see src/nkit/dynamic.h) which supports following data types:
    - **INTEGER**: represents integer values (implemented via int64_t type)
    - **UNSIGNED_INTEGER**: represents unsigned integer values (implemented via uint64_t type)
    - **FLOAT**: represents numbers with floating point (implemented via double type)
    - **BOOL**: represents boolean values (implemented via bool type)
    - **DATE_TIME**: represents simple date-time values with microseconds and 0 <= year <= 9999 (implemented via uint64_t type)
    - **STRING**: represents string values (implemented via std::string)
    - **LIST**: represents list of Dynamic values (implemented via std::vector<Dynamic>)
    - **DICT**: represents map of string keys to Dynamic values (implemented via std::map<std::string, Dynamic>)
    - **MONGODB_OID**: represents string values with limited behaviour for holding MongoDb ObjectID (implemented via std::string)
    - **TABLE**: represents table of Dynamic values with support of multiple indexes, sorting and grouping by multiple fields
    - **UNDEFINED**: represents uninitialized value, that can be changed in the future
    - **NONE**: represents NULL value, that can not be changed

2. Simple test framework

3. Logger

**Dynamic** values can be used to deal with JSON: it is possible to convert JSON to Dynamic variable and vice-versa.

## Requirements

It is mandatory to have **yajl** library for JSON paring (https://github.com/lloyd/yajl, version >= 2.0.4)

If your C++ compiler does not support std::shared_ptr, then it is mandatory to have Boost library (http://boost.org, version >= 1.53)

## Build & install
### Linux & Mac

    cd to/nkit/root

This commands will configure, build and install debug version of nkit library (with non-system boost):

    ./bootstrap.sh --prefix=/path/to/installation/folder --with-boost=/path/to/boost \
        --with-yajl=/path/to/yajl --debug
    make -C Debug-build
    make -C Debug-build install

This commands will configure, build and install release version of nkit library (with system boost):

    ./bootstrap.sh --prefix=/path/to/installation/folder --with-boost \
        --with-yajl=/path/to/yajl --release
    make -C Release-build
    make -C Release-build install

This commands will configure, build and install release-with-debug-version of nkit library (without boost, if C++ compiler supports std::shared_ptr):

    ./bootstrap.sh --prefix=/path/to/installation/folder --with-yajl=/path/to/yajl --rdebug
    make -C RelWithDebInfo-build
    make -C RelWithDebInfo-build install

For all configure options

    ./bootstrap.sh --help
    
### Windows

This commands will create Microsoft Visual C++ 2012 solution for nkit library (without using boost):

    cd c:\path\to\nkit\root
    mkdir win32
    cd win32
    cmake -G "Visual Studio 11" -DPREFIX=c:/path/to/install/folder -DYAJL_ROOT=c:/path/to/yajl/root -DYAJL_USE_DYN_LIBS=1 ..
    

## Usage

    #include "nkit/dynamic.h"
    #include "nkit/logger_brief.h"
    
    void main(int argc, char ** argv)
    {
      using namespace nkit;
    
      // Creating int64 data type
      Dynamic d_i64(1);
      CINFO(d_i64);
    
      // Creating uint64 data type
      Dynamic d_ui64 = Dynamic::UInt64(uint64_t(-1));
      CINFO(d_ui64);
    
      // Creating 'boolean' data type
      Dynamic d_true = Dynamic(true);
      CINFO(d_true);
    
      // Creating 'string' data type
      Dynamic d_str = Dynamic("string");
      CINFO(d_str);
    
      // Creating 'datetime' data type
      std::string error;
      Dynamic d_datetime = Dynamic::DateTimeLocal();
      CINFO(d_datetime);
    
      // Creating 'list' data type
      Dynamic list = DLIST(d_i64 << d_ui64 << d_true << d_str << d_datetime);
      CINFO(list);
    
      // Creating 'dictionary' data type
      Dynamic dict = DDICT("key1" << "str"
        << "key2" << d_i64
        << "key3" << list
        << "key4" << DLIST(d_i64 << d_ui64 << d_true << d_str << d_datetime)
        << "key5" << d_true
        << "key6" << d_str
        << "key7" << d_datetime
        << "key8" << DDICT("key in child dict" << "value in child dict")
      );
      CINFO(json_hr << dict);
    
      // Creating 'table' data type
      Dynamic table = DTBL("col1, col2",
        "11" << "12" <<
        "21" << "22");
      CINFO(json_hr_table << table);
    }


Additional user friendly examples will be added here as soon as possible.

See **test/test_*.cpp** files for various use cases.

## Author

Boris T. Darchiev (boris.darchiev at gmail.com)

On github: https://github.com/eye3

## THANKS TO

Vasiliy Soshnikov (dedok.mad at gmail.com):

On github: https://github.com/dedok

Wrote all cmake build files and bootstrap.sh

Participated in development of following parts of nkit:
- TABLE data type in Dynamic class
- Logger

