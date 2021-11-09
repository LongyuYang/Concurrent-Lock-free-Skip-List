.PHONY: init all clean 

BASEDIR = .
BUILDDIR = $(BASEDIR)/build
SOURCEDIR = $(BASEDIR)/src
INCLUDE = -I$(SOURCEDIR)
TESTDIR = $(BASEDIR)/tests

CXX = g++-11
CXXFLAGS = $(INCLUDE) -std=c++11 -O0

init:
	[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)

test_unsafe: init
	$(CXX) $(TESTDIR)/$@.cpp $(CXXFLAGS) -o $(BUILDDIR)/$@ 
	$(BUILDDIR)/$@

clean:
	rm -rf $(BUILDDIR)/*
