{
  'includes': ['common.gypi'],
  'targets': [
    {
      'variables': { 'target_arch%': 'ia32' }, # default for node v0.6.x
      'target_name': 'expat',
      'product_prefix': 'lib',
      'type': 'static_library',
      'sources': [
        'expat-2.1.0/lib/xmlparse.c',
        'expat-2.1.0/lib/xmltok.c',
        'expat-2.1.0/lib/xmlrole.c',
      ],
      'defines': [
        'PIC',
        'HAVE_EXPAT_CONFIG_H'
      ],
      'include_dirs': [
        '.',
        'include',
        'expat-2.1.0/lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '.',
          'include',
          'expat-2.1.0/lib',
        ],
        'conditions': [
          ['OS=="win"', {
            'defines': [
              'XML_STATIC'
            ]
          }]
        ],
      },
    },
  ]
}
