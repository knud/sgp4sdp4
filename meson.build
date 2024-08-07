project('sgp4sdp4', 'c', 'cpp',
    license: 'proprietary',
    version: '0.1.0',
    default_options : [
#        'c_std=c11', 'cpp_std=c++14',
        'warning_level=3',
        'buildtype=release',
        #'werror=true',
        ],
    )

git = find_program('git')
git_version = run_command(git, ['describe', '--tags', '--always'], check:true).stdout().strip()
sgp4sdp4_major = 0
sgp4sdp4_minor = 1
sgp4sdp4_patch = 0
message('Ported NORAD SGP4 and SDP4 routines version @0@.@1@.@2@'.format(sgp4sdp4_major, sgp4sdp4_minor, sgp4sdp4_patch))

add_project_arguments(
    '-DGIT_VERSION="@0@"'.format(git_version),
    '-DSATELLITE_SGP4SDP4_MAJOR=@0@'.format(sgp4sdp4_major),
    '-DSATELLITE_SGP4SDP4_MINOR_VERSION=@0@'.format(sgp4sdp4_minor),
    '-DSATELLITE_SGP4SDP4_PATCH=@0@'.format(sgp4sdp4_patch),
    language: 'cpp')

cpp = meson.get_compiler('cpp')
cc = meson.get_compiler('c')

curl_dep = dependency('libcurl')

boost_dep = dependency('boost', modules: [
        'program_options',
        'iostreams',
        'filesystem',
        'system',
        'thread'
        ],
    )

core_source_files = [
    'lib/sgp4sdp4.c',
    'lib/sgp_in.c',
    'lib/sgp_math.c',
    'lib/sgp_obs.c',
    'lib/sgp_time.c',
    'lib/solar.c',
#    'lib/tle_utils.c'
    ]


incdir = include_directories(
    'include',
    )


executable('example', 'src/example.c',
    sources: core_source_files,
    include_directories: incdir,
    dependencies: [boost_dep, curl_dep],
    #    link_with: ,
    install: true,
    )

executable('range_rate', 'src/range_rate.c',
    sources: core_source_files,
    include_directories: incdir,
    dependencies: [boost_dep, curl_dep],
    #    link_with: ,
    install: true,
    )


#install_subdir('lib/error_control/qcldpc/fec/ldpc/802.11/proto_H/',
#    install_dir: '/opt/darkstar/fec/ldpc/802.11/'
#    )

#subdir('atest')
#gtest_dep = dependency('gtest_main', required: false)
#if gtest_dep.found()
#    subdir('unit_tests')
#endif

#subdir('doc')
