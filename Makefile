.PHONY: init all clean 

BASEDIR = .
BUILDDIR = $(BASEDIR)/build
SOURCEDIR = $(BASEDIR)/src
GCDIR = $(BASEDIR)/gc
INCLUDE = -I$(SOURCEDIR) -I$(GCDIR)/include/gc/
LIBS = $(GCDIR)/lib/libgc.a
TESTDIR = $(BASEDIR)/tests

CXX = g++
CXXFLAGS = $(INCLUDE) -m64 -std=c++11 -O0 -g -lpthread 

init:
	[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)

test_unsafe: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@ 
	$(BUILDDIR)/$@ 

test_pointer_lock: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@
	$(BUILDDIR)/$@

clean:
	rm -rf $(BUILDDIR)/*
