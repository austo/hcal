{
    "targets": [
    {
      "target_name": "hcal",
      "sources": [  "hcal.cc", "eventWrapper.cc",
                    "configWrapper.cc", "event.cc",
                    "hcal_utils.cc", "monthWriter.cc",
                    "weekWriter.cc", "dataLayer.cc"
                  ],
      'conditions': [
          ['OS!="win"', {
            'cflags': [ '-Wall', '-fexceptions', '-g' ],
            'cflags_cc!': [ '-fno-exceptions' ],
            'link_settings': {
              'libraries': [ '-lhpdf', '-lboost_date_time', '-lboost_exception', '-lpqxx', '-lpq', '-lhyphen' ]
            },
            'include_dirs': [ '/usr/local/boost_1_52_0/boost/date_time',
                              '/usr/local/boost_1_52_0/boost',
                              '/usr/local/boost_1_52_0/boost/exception',
                              '/usr/local/boost_1_52_0',
                              '/usr/local/include/pqxx'
                              
            ]
          }
        ],
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          },
          'ldflags': [ '-L/Applications/Postgres.app/Contents/MacOS/lib' ],
        }],
        ['OS=="linux"', {
          'ldflags': [ '-L/usr/local/boost_1_52_0/stage/lib']
        }]
      ]
    }
  ]
}