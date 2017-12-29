# This file is used with the GYP meta build system.
# http://code.google.com/p/gyp
# To build try this:
#   svn co http://gyp.googlecode.com/svn/trunk gyp
#   ./gyp/gyp -f make --depth=. inih.gyp
#   make
#   ./out/Debug/test
{
  'targets': [
    {
      'target_name': 'inih',
      'type': 'static_library',
      'sources': [
        'inih/ini.c',
      ],
      'include_dirs': [
        'inih',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'inih',
        ],
      },
    },
  ]
}
