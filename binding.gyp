{
    "targets": [
    {
      "target_name": "hcal",
      "sources": [ "hcal.cc", "eventWrapper.cc", "event.cc", "eventWriter.cc" ],
      'conditions': [
          ['OS!="win"', {
            'cflags': [ '-Wall' ],
            'link_settings': {
              'libraries': [ '-lhpdf', '-lboost_date_time' ]
            },
            'include_dirs': [ '/usr/local/boost_1_52_0/boost/date_time' ]
          }
        ]
      ]
    }
  ]
}