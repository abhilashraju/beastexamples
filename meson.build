project(
  'file_body',
  'cpp',
  version: '0.1',
  meson_version: '>=0.58.0',
  default_options: [
    'warning_level=3',
    'cpp_std=c++20',
  ])
openssl_dep = dependency('openssl', version: '>=1.1.1')
boost_dep = dependency('boost')
executable(
  'file_body',
  'filebody.cpp',
  #include_directories: chai_client_includes,
  dependencies:[openssl_dep,boost_dep],
  install: true,
  install_dir: get_option('bindir'))