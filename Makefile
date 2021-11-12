.PHONY: init all clean 

BASEDIR = .
BUILDDIR = $(BASEDIR)/build
SOURCEDIR = $(BASEDIR)/src
INCLUDE = -I$(SOURCEDIR)
TESTDIR = $(BASEDIR)/tests

CXX = g++
CXXFLAGS = $(INCLUDE) -m64 -std=c++11 -O3

init:
	[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)

test_unsafe: init
	$(CXX) $(TESTDIR)/$@.cpp $(CXXFLAGS) -o $(BUILDDIR)/$@ 
	$(BUILDDIR)/$@

test_pointer_lock: init
	$(CXX) $(TESTDIR)/$@.cpp $(CXXFLAGS) -fopenmp -o $(BUILDDIR)/$@ 
	$(BUILDDIR)/$@

clean:
	rm -rf $(BUILDDIR)/*
