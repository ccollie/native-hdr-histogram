{
  "targets": [
    {
      "target_name": "<(module_name)",
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'conditions': [
        ['OS=="win"', {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            }
          }
        }],
        ['OS=="mac"', {
          "xcode_settings": {
            "CLANG_CXX_LIBRARY": "libc++",
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'MACOSX_DEPLOYMENT_TARGET': '10.7'
          }
        }]
      ],
      "sources": [
        "src/hdr_encoding.h",
        "src/hdr_encoding.c",
        "src/hdr_histogram.h",
        "src/hdr_histogram.c",
        "src/hdr_histogram_log.h",
        "src/hdr_histogram_log.c",
        "src/hdr_time.h",
        "src/hdr_time.c",
        "hdr_histogram_wrap.cc",
        "hdr_histogram_iterator_wrap.cc",
        "histogram.cc"
      ],
      'defines': [ 'NAPI_CPP_EXCEPTIONS' ], 
      "dependencies": [
        "<(module_root_dir)/zlib/zlib.gyp:zlib"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "src/"
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
