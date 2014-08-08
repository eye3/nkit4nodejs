{
    "targets": [
        {
            "target_name": "nkit4nodejs",
            "sources": [
                "deps/yajl/src/yajl_lex.c",
                "deps/yajl/src/yajl_encode.c",
                "deps/yajl/src/yajl_parser.c",
                "deps/yajl/src/yajl_version.c",
                "deps/yajl/src/yajl_alloc.c",
                "deps/yajl/src/yajl_buf.c",
                "deps/yajl/src/yajl_tree.c",
                "deps/yajl/src/yajl_gen.c",
                "deps/yajl/src/yajl.c",
                "deps/nkit/src/constants.cpp",
                "deps/nkit/src/tools.cpp",
                "deps/nkit/src/dynamic/dynamic.cpp",
                "deps/nkit/src/dynamic/dynamic_json.cpp",
                "deps/nkit/src/dynamic/dynamic_path.cpp",
                "deps/nkit/src/dynamic/dynamic_table.cpp",
                "deps/nkit/src/dynamic/dynamic_table_index_comparators.cpp",
                "deps/nkit/src/dynamic/dynamic_xml.cpp",
                "deps/nkit/src/logger/rotate_logger.cpp",
                "deps/nkit/src/vx/encodings.cpp",
                "deps/nkit/src/vx/vx.cpp",
                "src/nkit4nodejs_module.cpp",
                "src/xml2var_builder_wrapper.cpp",
                "src/xml2var_builder_wrapper.h",
                "src/v8_var_builder.h"
            ],
            "include_dirs": [
                "deps/nkit/src",
                "deps/include"
            ],
            'dependencies': [
                'deps/expat-2.1.0/expat.gyp:expat'
            ]
        }
    ]
}
