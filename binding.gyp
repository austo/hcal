{
    "targets": [
    {
      "target_name": "hcal",
      "sources": [ "hcal.cc", "eventWrapper.cc", "configWrapper.cc", "event.cc", "eventWriter.cc" ],
      'conditions': [
          ['OS!="win"', {
            'cflags': [ '-Wall', '-fexceptions' ],
            'cflags_cc!': [ '-fno-exceptions' ],
            'link_settings': {
              'libraries': [ '-lhpdf', '-lboost_date_time', '-lboost_exception' ]
            },
            'include_dirs': [ '/usr/local/boost_1_52_0/boost/date_time',
                              '/usr/local/boost_1_52_0/boost',
                              '/usr/local/boost_1_52_0/boost/exception',
                              '/usr/local/boost_1_52_0'
                              
            ]
          }
        ],
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }],
        ['OS=="linux"', {
          'ldflags': [ '-L/usr/local/boost_1_52_0/stage/lib']
        }]
      ]
    }
  ]
}