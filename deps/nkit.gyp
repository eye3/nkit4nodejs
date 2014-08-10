{
  'includes': ['common.gypi'],
  'targets': [
    {
      'variables': { 'target_arch%': 'ia32' }, # default for node v0.6.x
      'target_name': 'nkit',
      'product_prefix': 'lib',
      'type': 'static_library',
      'sources': [
        "nkit/src/constants.cpp",
        "nkit/src/tools.cpp",
        "nkit/src/dynamic/dynamic.cpp",
        "nkit/src/dynamic/dynamic_json.cpp",
        "nkit/src/dynamic/dynamic_path.cpp",
        "nkit/src/dynamic/dynamic_table.cpp",
        "nkit/src/dynamic/dynamic_table_index_comparators.cpp",
        "nkit/src/dynamic/dynamic_xml.cpp",
        "nkit/src/logger/rotate_logger.cpp",
        "nkit/src/vx/encodings.cpp",
        "nkit/src/vx/vx.cpp",
        "nkit/3rd/netbsd/strptime.cpp",
      ],
      'include_dirs': [
        '.',
        'nkit/src',
        'expat-2.1.0/lib',
        'include',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '.',
          'nkit/src',
          'expat-2.1.0/lib',
          'include',
        ],
      },
    },
  ]
}
