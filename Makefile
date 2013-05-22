CXXFLAGS=-Wall -pedantic `Magick++-config  --cppflags` -DNDEBUG -O4
LDFLAGS=-lgflags -lglog `Magick++-config --ldflags --libs` -O4
BINARIES=conncomp

all: $(BINARIES)

conncomp.o: conncomp.cc
	$(CXX) -c $< $(CXXFLAGS)

conncomp: conncomp.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(BINARIES)
