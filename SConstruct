FAS_CFLAGS = [
    '-Wall',
    '-std=c++11',
    '-Wno-old-style-cast',
    '-g',
    '-O2',
    '-export-dynamic',
    '-D_GNU_SOURCE',
    '-D_REENTRANT'
    ]

INCLUDE_PATH = ['.', 'routine']

routine_source = Glob("routine/*.cpp") + Glob("routine/*.S") 
StaticLibrary('./lib/routine',
        routine_source,
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        CCFLAGS = FAS_CFLAGS
)
Program('./bin/Main', './test/Main.cpp',
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        LIBS = ['routine', 'pthread'],
        CCFLAGS = FAS_CFLAGS
)
