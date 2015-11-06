CXXFILES = \
	callback_default_alloc.cpp \
	callback_custom_alloc.cpp \
	coroutine.cpp

EXES = \
	build\os_dll\callback_default_alloc.exe \
	build\os_dll\callback_custom_alloc.exe \
	build\os_dll\coroutine.exe \
	build\os_static\callback_default_alloc.exe \
	build\os_static\callback_custom_alloc.exe \
	build\os_static\coroutine.exe

DLLS = \
	build\os_dll\os.dll

STATICLIBS = \
	build\os_static\os.lib

OBJS = $(DLLS:.dll=.obj) $(STATICLIBS:.lib=.obj) $(EXES:.exe=.obj)
ILKS = $(DLLS:.dll=.ilk) $(EXES:.exe=.ilk)
PDBS = $(DLLS:.dll=.pdb) $(EXES:.exe=.pdb) vc140.pdb
LIBS = $(DLLS:.dll=.lib)
TESTS = $(EXES:.exe=.test)

CXXFLAGS = /await /EHsc /Iinclude /Ox /Ob2 /Ot /Zi /GL /MT

.SUFFIXES: .test .exe .lib

all: $(DLLS) $(STATICLIBS) $(EXES)

test: $(TESTS)

clean:
	del $(OBJS) $(DLLS) $(EXES) $(ILKS) $(PDBS) $(LIBS) $(TESTS)

build\os_dll\os.dll: src\os\os.cpp
	if not exist build\os_dll mkdir build\os_dll
	cl /LD $(CXXFLAGS) -DOS_HAS_DLL -DOS_BUILD_DLL -Fe$@ -Fo$(@:.dll=.obj) src\os\os.cpp

build\os_static\os.lib: src\os\os.cpp
	if not exist build\os_static mkdir build\os_static
	cl /LD $(CXXFLAGS) -c -Fo$(@:.lib=.obj) src\os\os.cpp
	lib $(@:.lib=.obj)

{src\test}.cpp{build\os_dll}.exe:
	if not exist build\os_dll mkdir build\os_dll
	cl $(CXXFLAGS) -DOS_HAS_DLL -Fe$@ -Fo$(@:.exe=.obj) $< $(LIBS)

{src\test}.cpp{build\os_static}.exe:
	if not exist build\os_static mkdir build\os_static
	cl $(CXXFLAGS) -Fe$@ -Fo$(@:.exe=.obj) $< $(STATICLIBS)

.exe.test:
	@echo Running $<
	@-start /b /wait /high $<
