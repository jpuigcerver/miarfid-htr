CXXFLAGS=-Wall -pedantic `Magick++-config  --cppflags` -DNDEBUG -O4
LDFLAGS=-lgflags -lglog `Magick++-config --ldflags --libs` -O4
BINARIES=conncomp connfeat

all: $(BINARIES)

connfeat.o: connfeat.cc
	$(CXX) -c $< $(CXXFLAGS)

connfeat: connfeat.o
	$(CXX) -o $@ $^ $(LDFLAGS)

conncomp.o: conncomp.cc
	$(CXX) -c $< $(CXXFLAGS)

conncomp: conncomp.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(BINARIES)
