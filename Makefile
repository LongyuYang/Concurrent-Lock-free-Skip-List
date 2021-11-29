.PHONY: init all clean 

BASEDIR = .
BUILDDIR = $(BASEDIR)/build
SOURCEDIR = $(BASEDIR)/src
GCDIR = $(BASEDIR)/gc
INCLUDE = -I$(SOURCEDIR) -I$(GCDIR)/include/gc/
LIBS = $(GCDIR)/lib/libgc.a
TESTDIR = $(BASEDIR)/tests

CXX = g++
CXXFLAGS = $(INCLUDE) -m64 -std=c++11 $(SOURCEDIR)/CycleTimer.h -O3 -lpthread 

init:
	[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)

test_unsafe: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@ 
	$(BUILDDIR)/$@ 

test_pointer_lock: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@
	$(BUILDDIR)/$@

test_glock: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@
	$(BUILDDIR)/$@

test_lock_free: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@
	$(BUILDDIR)/$@

test_performance: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@
	$(BUILDDIR)/$@

clean:
	rm -rf $(BUILDDIR)/*
