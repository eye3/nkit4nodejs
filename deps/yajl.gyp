{
  'includes': ['common.gypi'],
  'targets': [
    {
      'variables': { 'target_arch%': 'ia32' }, # default for node v0.6.x
      'target_name': 'yajl',
      'product_prefix': 'lib',
      'type': 'static_library',
      'sources': [
        "yajl-2.0.5/src/yajl_lex.c",
        "yajl-2.0.5/src/yajl_encode.c",
        "yajl-2.0.5/src/yajl_parser.c",
        "yajl-2.0.5/src/yajl_version.c",
        "yajl-2.0.5/src/yajl_alloc.c",
        "yajl-2.0.5/src/yajl_buf.c",
        "yajl-2.0.5/src/yajl_tree.c",
        "yajl-2.0.5/src/yajl_gen.c",
        "yajl-2.0.5/src/yajl.c",
      ],
      'include_dirs': [
        '.',
        'include',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '.',
          'include',
        ],
      },
    },
  ]
}
