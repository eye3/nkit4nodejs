{
    "targets": [
        {
            "target_name": "nkit",
            "sources": [
                "src/nkit_module.cpp",
                "src/xml2var_builder_wrapper.cpp",
            ],
            "include_dirs": [
                "$(NKIT_ROOT)/include",
                "/usr/local/include",
                "/usr/include"
            ],
            "libraries": [
                "-L$(NKIT_ROOT)/lib",
                "-L/usr/local/lib",
                "-L/usr/lib",
                "-L/usr/lib/x86_64-linux-gnu",
                "-lnkit",
                "-lexpat",
                "-lyajl",
                "-lboost_system",
                "-lrt"
            ]
        }
    ]
}
