project('riemann-c-client', 'c', version: '1.10.4-MESON')

config = configuration_data()
config.set('MAJOR_VERSION', '1')
config.set('MINOR_VERSION', '10')
config.set_quoted('NUMBER_VERSION', '1.10.4-MESON')
config.set('PATCH_VERSION', '4')
config.set_quoted('PACKAGE_VERSION', meson.project_version())
config.set_quoted('PACKAGE_STRING', meson.project_name())

cc  = meson.get_compiler('c')
dep = cc.find_library('protobuf-c')

protoc = find_program('protoc-c')
gen = generator(protoc, output: ['@BASENAME@.pb-c.c', '@BASENAME@.pb-c.h'], arguments: ['--proto_path=@CURRENT_SOURCE_DIR@', '--c_out=@BUILD_DIR@', '@INPUT@'])

libclientgensrc = gen.process('riemann.proto')

libclientsrc = [
    'lib/riemann/client.c',
    'lib/riemann/client/tcp.c',
    'lib/riemann/client/tls.c',
    'lib/riemann/client/udp.c',
    'lib/riemann/event.c',
    'lib/riemann/message.c',
    'lib/riemann/attribute.c',
    'lib/riemann/query.c',
    'lib/riemann/simple.c',
    libclientgensrc
]

incdirs = include_directories('lib', 'lib/riemann', 'lib/riemann/client')

configure_file(input: 'lib/riemann/riemann-client.h.in', output: 'riemann-client.h', configuration: config)
configure_file(output: 'platform.h', configuration: config)

shared_library('riemann_client', libclientsrc, include_directories: incdirs, dependencies: dep)
