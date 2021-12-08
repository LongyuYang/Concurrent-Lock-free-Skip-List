.PHONY: init all clean 

BASEDIR = .
BUILDDIR = $(BASEDIR)/build
SOURCEDIR = $(BASEDIR)/src
GCDIR = $(BASEDIR)/gc
INCLUDE = -I$(SOURCEDIR) -I$(GCDIR)/include/gc/
LIBS = $(GCDIR)/lib/libgc.so
TESTDIR = $(BASEDIR)/tests
TEST_THREAD_SAFE = "test_thread_safe"

CXX = g++
CXXFLAGS = $(INCLUDE) -m64 -std=c++11 $(SOURCEDIR)/CycleTimer.h -O3 -lpthread 

init:
	[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)

test_unsafe: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@ 
	$(BUILDDIR)/$@ 

compile_thread_safe: init
	$(CXX) $(TESTDIR)/$(TEST_THREAD_SAFE).cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$(TEST_THREAD_SAFE)

test_glock: compile_thread_safe
	$(BUILDDIR)/$(TEST_THREAD_SAFE) -v 0

test_pointer_lock: compile_thread_safe
	$(BUILDDIR)/$(TEST_THREAD_SAFE) -v 1

test_lock_free: compile_thread_safe
	$(BUILDDIR)/$(TEST_THREAD_SAFE) -v 2

test_performance_1: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@
	bash $(TESTDIR)/scripts/performance_1.sh

test_performance_2: init
	$(CXX) $(TESTDIR)/$@.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@
	bash $(TESTDIR)/scripts/performance_2.sh

test_performance_3: init
	$(CXX) $(TESTDIR)/test_performance_2.cpp $(LIBS) $(CXXFLAGS) -o $(BUILDDIR)/$@
	bash $(TESTDIR)/scripts/performance_3.sh

clean:
	rm -rf $(BUILDDIR)/*
