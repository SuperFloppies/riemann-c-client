#
# TODO: Incorporate linker script, generate configuration header.
#
project('riemann-c-client', 'c', version: '1.10.4-M2', default_options: [
    'b_asneeded=false'
])

top = meson.source_root()

cc = meson.get_compiler('c')
add_global_arguments('-D_GNU_SOURCE', language: 'c')

prjstr    = meson.project_name()
verstr    = meson.project_version()
vernum    = verstr.split('-')
verscript = '@0@/@1@'.format(top, 'lib/riemann/libriemann.ver')
ldver     = cc.has_link_argument('-Wl,--version-script,' + verscript)

vermaj = vernum[0].split('.')[0]
vermin = vernum[0].split('.')[1]
verpat = vernum[0].split('.')[2]

cf = configuration_data()
cf.set_quoted('PROJECT_NAME', prjstr)
cf.set_quoted('PACKAGE_VERSION', verstr)
cf.set_quoted('PACKAGE_STRING', prjstr + ' ' + verstr)
cf.set('MAJOR_VERSION', vermaj)
cf.set('MINOR_VERSION', vermin)
cf.set('PATCH_VERSION', verpat)

# TODO: DRY. This is repetition of information already encoded above.
# I need to find a way to generate this properly, just being lazy right now.
cf.set('NUMBER_VERSION', '0x00010A04')

incdirs = include_directories('lib')

protocc  = find_program('protoc-c')
protosrc = custom_target(
    'protosrc',
    input:   'riemann.proto',
    output:  ['@BASENAME@.pb-c.c', '@BASENAME@.pb-c.h'],
    command: [protocc, '--proto_path=@CURRENT_SOURCE_DIR@', '--c_out=@OUTDIR@', '@INPUT@']
)
protohdr = protosrc[1]
libsrc = [
    'lib/riemann/client.c',
    'lib/riemann/client/tcp.c',
    'lib/riemann/client/udp.c',
    'lib/riemann/event.c',
    'lib/riemann/message.c',
    'lib/riemann/attribute.c',
    'lib/riemann/query.c',
    'lib/riemann/simple.c',
    'lib/riemann/client/tls.c',
    protosrc
]

libcheck     = dependency('check', required: get_option('tests'))
libprotobufc = cc.find_library('protobuf-c')
libdl        = cc.find_library('dl')
libjsonc     = dependency('json-c', version: '>= 0.11', required: false)
libgnutls    = dependency('gnutls', version: '> 2.8', required: get_option('tls'))

cf.set('HAVE_VERSIONING', ldver)
cf.set('HAVE_JSON_C', libjsonc.found())
cf.set('HAVE_CHECK',  libcheck.found())
cf.set('HAVE_GNUTLS', libgnutls.found())

libdeps = [ libprotobufc, libjsonc, libgnutls, libdl ]

configure_file(output: 'config.h', configuration: cf)
configure_file(input: 'lib/riemann/riemann-client.h.in', output: '@BASENAME@', configuration: cf, install_dir: join_paths(get_option('includedir'), 'riemann'))

if ldver
    liblinkargs = '-Wl,--version-script,' + verscript
endif

lib = shared_library('riemann_client', libsrc, include_directories: incdirs, dependencies: libdeps, link_args: liblinkargs)
libdep = declare_dependency(link_with: lib, include_directories: incdirs)
testdeps = [ libdl, libdep, libcheck ]

if get_option('tests')
    if libcheck.found()
        test('main', executable('test-main',
            [ 'tests/mocks.c', 'tests/check_libriemann.c', protohdr], dependencies: testdeps))
        test('symver', executable('test-symver',
            [ 'tests/mocks.c', 'tests/check_symver.c', protohdr], dependencies: testdeps))
        test('networked', executable('test-networked',
            [ 'tests/mocks.c', 'tests/check_networked.c', protohdr], dependencies: testdeps))
    endif
endif
