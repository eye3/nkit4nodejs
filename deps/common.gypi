{
  'target_defaults': {
    'default_configuration': 'Release',
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG', '_DEBUG' ],
        'conditions': [
          ['OS!="win"',{
            'cflags': [
              '-std=gnu++0x',
            ],
          }],
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 1, # static debug
            'ExceptionHandling': 1,     # /EHsc  doesn't work.
          },
        },
      },
      'Release': {
        'defines': [ 'NDEBUG' ],
        'conditions': [
          ['OS=="win"', {
            'cflags': [
              '-std=gnu++0x',
            ],
          }],
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 0, # static release
            'ExceptionHandling': 1,     # /EHsc  doesn't work.
          },
        },
      }
    },
    'msvs_settings': {
      'VCCLCompilerTool': {
      },
      'VCLibrarianTool': {
      },
      'VCLinkerTool': {
        'GenerateDebugInformation': 'true',
      },
    },
  },
}
