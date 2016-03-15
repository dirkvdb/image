project('image', 'cpp', default_options : ['cpp_std=c++14'])

utilssub = subproject('utils')

imageinc = include_directories('inc')

imagefiles = files(
    'inc/image/image.h',
    'inc/image/imagefactory.h', 'src/imagefactory.cpp',
    'inc/image/imageloadstoreinterface.h'
)

png_dep = dependency('libpng', required : false)
jpeg_dep = find_library('jpeg', required : false)

if not jpeg_dep.found()
    jpeg_dep = find_library('jpeg', dirs : '/usr/local/lib', required : false)
    if jpeg_dep.found()
        jpeginc = include_directories('/usr/local/include', is_system : true)        
    endif    
endif

config = configuration_data()
config.set10('HAVE_PNG', png_dep.found())
config.set10('HAVE_JPEG', jpeg_dep.found())

if png_dep.found()
    imagefiles += files('src/imageloadstorepng.h', 'src/imageloadstorepng.cpp')
endif

if jpeg_dep.found()
    imagefiles += files('src/imageloadstorejpeg.h', 'src/imageloadstorejpeg.cpp')
endif

configure_file(input : 'imageconfigmeson.h.in', output : 'imageconfig.h', configuration : config)

imagelib = static_library('image',
                          imagefiles,
                          include_directories : [imageinc, jpeginc],
                          dependencies : [png_dep, jpeg_dep, utilssub.get_variable('utils_dep')])

image_dep = declare_dependency(link_with : imagelib, include_directories : imageinc)

subdir('test')
