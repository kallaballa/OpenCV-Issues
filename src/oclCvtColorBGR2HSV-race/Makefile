TARGET := perf-test

SRCS    := perf-test.cpp

#precompiled headers
HEADERS := 
OBJS    := ${SRCS:.cpp=.o} 
DEPS    := ${SRCS:.cpp=.dep} 

CXXFLAGS += -fpic -I/usr/local/opencv2
LDFLAGS +=  -L/usr/local/lib64 -lopencv_core -lopencv_highgui -lopencv_imgproc
LIBS += -lm `pkg-config --libs opencv4`
.PHONY: all release debug clean distclean 

all: release
release: ${TARGET}
debug: ${TARGET}
info: ${TARGET}
profile: ${TARGET}
unsafe: ${TARGET}
asan: ${TARGET}

${TARGET}: ${OBJS}
	${CXX} ${LDFLAGS} -o $@ $^ ${LIBS}

${OBJS}: %.o: %.cpp %.dep ${GCH}
	${CXX} ${CXXFLAGS} -o $@ -c $<

${DEPS}: %.dep: %.cpp Makefile 
	${CXX} ${CXXFLAGS} -MM $< > $@ 

${GCH}: %.gch: ${HEADERS} 
	${CXX} ${CXXFLAGS} -o $@ -c ${@:.gch=.hpp}

install:
	mkdir -p ${DESTDIR}/${PREFIX}
	cp ${TARGET} ${DESTDIR}/${PREFIX}

uninstall:
	rm ${DESTDIR}/${PREFIX}/${TARGET}

clean:
	rm -f *~ ${DEPS} ${OBJS} ${CUO} ${GCH} ${TARGET} 

distclean: clean

