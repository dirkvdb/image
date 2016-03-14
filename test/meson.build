imagetestfiles = files(
    'gmock-gtest-all.cpp',
    'main.cpp',
    'imageloadingtest.cpp'
)

testinc = include_directories(meson.current_build_dir() + '/..')
gtestinc = include_directories(meson.current_source_dir() + '/gmock', is_system : true)

config = configuration_data()
config.set('IMAGE_TEST_DATA_DIR', meson.current_source_dir() + '/data')
configure_file(input : 'imagetestconfigmeson.h.in', output : 'imagetestconfig.h', configuration : config)

imagetest = executable('imagetest',
                       imagetestfiles,
                       include_directories : [testinc, gtestinc, utilssub.get_variable('utilsinc')],
                       dependencies : image_dep)

test('image test', imagetest)