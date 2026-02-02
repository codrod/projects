CXX = g++
CXXFLAGS = --std=c++11 -Iinclude `pkg-config --cflags gtkmm-3.0`
LDFLAGS = -Llib `pkg-config --libs gtkmm-3.0`
LDLIBS = -lpthread

AR = ar
ARFLAGS = crv

INCLUDES = include/gtb.h
OBJS = src/controller.o src/coord.o src/error.o src/gtb.o src/inputer.o src/outputer.o src/sequence.o src/tag.o src/window.o

#TESTS =
#EXAMPLES =

%.o: %.cpp $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS) $(LDLIBS)

#%.ct: %.c lib/libtb.a
#	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS)

bin/gtw: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

#test: $(TESTS)
#	rm -f test.sh
#	echo "#!/bin/sh" >> test.sh
#	ls test/*.ct -1 >> test.sh
#	chmod u+x test.sh

.PHONY: docs
docs:
	doxygen

#examples: $(EXAMPLES);

clean:
	rm -fr src/*.o bin/* docs/* #test/*.ct examples/*.ct test.sh
