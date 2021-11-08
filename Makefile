.PHONY: all clean 

BASEDIR = .
BUILDDIR = $(BASEDIR)/build
SOURCEDIR = $(BASEDIR)/src
INCLUDE = -I$(SOURCEDIR)
TESTDIR = $(BASEDIR)/tests

CXX = g++
CXXFLAGS = $(INCLUDE) -std=c++11 -O0

test_unsafe: 
	$(CXX) $(TESTDIR)/$@.cpp $(CXXFLAGS) -o $(BUILDDIR)/$@ 
	$(BUILDDIR)/$@

clean:
	rm -rf $(BUILDDIR)/*
