# location of libFFAUDES
LIBFAUDES=../../..

# link to libFAUDES DSO
LNKLIBS += -lfaudes -L $(LIBFAUDES)
LDFLAGS += -Wl,-rpath,@executable_path/.,-rpath,@executable_path/$(LIBFAUDES)
#LDFLAGS += -Wl,-rpath='$$ORIGIN:$$ORIGIN/../lib:$$ORIGIN/../:$$ORIGIN/../../../'

# CXXFLAGS
CXXFLAGS=-O3 -std=gnu++11 -Wall -Wno-unused-private-field -Wno-inconsistent-missing-override

# default comes first
all: hoa2gen gen2hoa

# my targets
hoa2gen: src/hoa2gen.cc include/cpphoafparser/*/*.hh include/*.hh
	$(CXX) $(CXXFLAGS) -I include -I $(LIBFAUDES)/include -o $@ $< $(LNKLIBS) $(LDFLAGS) 
gen2hoa: src/gen2hoa.cc 
	$(CXX) $(CXXFLAGS) -I include -I $(LIBFAUDES)/include -o $@ $< $(LNKLIBS) $(LDFLAGS) 

# take care
.PHONY: all
