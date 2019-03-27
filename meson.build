#
# TODO: Incorporate linker script, generate configuration header.
#
project('riemann-c-client', 'c', version: '1.10.4-M2')

prjstr = meson.project_name()
verstr = meson.project_version()
vernum = verstr.split('-')

vermaj = vernum[0].split('.')[0]
vermin = vernum[0].split('.')[1]
verpat = vernum[0].split('.')[2]

cf = configuration_data()
cf.set_quoted('PROJECT_NAME', prjstr)
cf.set_quoted('PROJECT_VERSION', verstr)
cf.set_quoted('PROJECT_STRING', prjstr + ' ' + verstr)

# TODO: DRY. This is repetition of information already encoded above.
# I need to find a way to generate this properly, just being lazy right now.
cf.set('NUMBER_VERSION', '0x00010A04')

cc = meson.get_compiler('c')

incdirs = include_directories('.', 'lib', 'lib/riemann', 'lib/riemann/client')

protocc  = find_program('protoc-c')
protogen = generator(protocc, output: ['@BASENAME@.pb-c.c', '@BASENAME@.pb-c.h'], arguments: ['--proto_path=@CURRENT_SOURCE_DIR@', '--c_out=@BUILD_DIR@', '@INPUT@'])

libgensrc = protogen.process('riemann.proto')
libsrc = [
    'lib/riemann/client.c',
    'lib/riemann/client/tcp.c',
    'lib/riemann/client/tls.c',
    'lib/riemann/client/udp.c',
    'lib/riemann/event.c',
    'lib/riemann/message.c',
    'lib/riemann/attribute.c',
    'lib/riemann/query.c',
    'lib/riemann/simple.c',
    libgensrc
]

libcheck     = dependency('check', required: false)
libprotobufc = cc.find_library('protobuf-c')
libjsonc     = dependency('json-c', version: '>= 0.11', required: false)
libgnutls    = dependency('gnutls', version: '> 2.8', required: get_option('tls'))

ldver = cc.has_link_argument('-Wl,--version-script,conftest.ver')
cf.set('HAVE_VERSIONING', ldver)

cf.set('HAVE_JSON_C', libjsonc.found())
cf.set('HAVE_CHECK',  libcheck.found())
cf.set('HAVE_GNUTLS', libgnutls.found())

libdeps = [ libcheck, libprotobufc, libjsonc, libgnutls ]

configure_file(output: 'config.h', configuration: cf)

lib = shared_library('riemann_client', libsrc, include_directories: incdirs)
libdep = declare_dependency(link_with: lib, include_directories: incdirs)
if libcheck.found()
    test('main', executable('test-main',
        [ 'tests/mocks.c', 'tests/check_libriemann.c', libgensrc], dependencies: [libdep]))
endif
