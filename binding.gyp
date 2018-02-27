{
    "targets": [
        {
            "target_name": "freefare",
            "defines": [ 'V8_DEPRECATION_WARNINGS=1', '_FILE_OFFSET_BITS=32' ],
            "sources": [ "src/addon.cpp", "src/freefare.cpp",  "src/device.cpp", "src/tag.cpp", "src/tag_mifareultralight.cpp", "src/tag_mifareclassic.cpp", "src/tag_mifaredesfire.cpp", "src/tag_ntag.cpp" ],
            "include_dirs" : [
                "<!(node -e \"require('nan')\")",
                "/usr/include"
            ],
            'link_settings': {
                 'libraries': [ '-lnfc', '-lfreefare' ],
                 'library_dirs': [ ]
            }
        }
    ]
}
