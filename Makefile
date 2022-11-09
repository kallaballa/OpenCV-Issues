CXX      := g++
CXXFLAGS := -std=c++20 -pthread -fno-strict-aliasing -pedantic -Wall -march=native -flto
LDFLAGS  := -L/opt/local/lib -flto
LIBS     := -lnanovg
.PHONY: all release debian-release info debug asan clean debian-clean distclean 
DESTDIR := /
PREFIX := /usr/local

all: release

release: CXXFLAGS += -g0 -O3
release: dirs

info: CXXFLAGS += -g3 -O0
info: dirs

debug: CXXFLAGS += -g3 -O0 -rdynamic
debug: dirs

profile: CXXFLAGS += -g3 -O1
profile: dirs

unsafe: CXXFLAGS += -g0 -Ofast -DNDEBUG -ffast-math -ftree-vectorizer-verbose=1 -funroll-loops -ftree-vectorize -fno-signed-zeros -fno-trapping-math -frename-registers
unsafe: dirs

asan: CXXFLAGS += -g3 -O0 -fno-omit-frame-pointer -fsanitize=address
asan: LDFLAGS += -fsanitize=address
asan: LIBS+= -lbfd -ldw
asan: dirs

clean: dirs

export LDFLAGS
export CXXFLAGS
export LIBS

dirs:
	${MAKE} -C src/hls-hsv-opencl/ ${MAKEFLAGS} CXX=${CXX} ${MAKECMDGOALS}
	${MAKE} -C src/slow-hog-opencl/ ${MAKEFLAGS} CXX=${CXX} ${MAKECMDGOALS}
	${MAKE} -C src/v4l2-vaapi/ ${MAKEFLAGS} CXX=${CXX} ${MAKECMDGOALS}
	
debian-release:
	${MAKE} -C src/hls-hsv-opencl/ ${MAKEFLAGS} CXX=${CXX} release 
	${MAKE} -C src/slow-hog-opencl/ ${MAKEFLAGS} CXX=${CXX} release 
	${MAKE} -C src/v4l2-vaapi/ ${MAKEFLAGS} CXX=${CXX} release

debian-clean:
	${MAKE} -C src/hls-hsv-opencl/ ${MAKEFLAGS} CXX=${CXX} clean 
	${MAKE} -C src/slow-hog-opencl/ ${MAKEFLAGS} CXX=${CXX} clean
	${MAKE} -C src/v4l2-vaapi/ ${MAKEFLAGS} CXX=${CXX} clean
	
install: ${TARGET}
	true

distclean:
	true

