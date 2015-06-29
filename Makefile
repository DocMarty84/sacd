ARCH = $(shell getconf LONG_BIT)

CXX = g++

CXXFLAGS_32 = -msse2
CXXFLAGS_64 =
CXXFLAGS = $(CXXFLAGS_$(ARCH)) -std=c++11 -g -Wall -O3
#CXXFLAGS += -ggdb3

VPATH = libdstdec:libdsd2pcm:libsacd

INCLUDE_DIRS = libdstdec libdsd2pcm libsacd
CPPFLAGS = $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))

LIBRARIES = rt pthread
LIBRARY_DIRS = libdstdec libdsd2pcm libsacd
LDFLAGS = $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(LIBRARIES),-l$(library))

.PHONY: all clean install

all: clean dst_ac dst_memory dst_data ccp_calc dst_unpack dst_fram dst_init dst_decoder \
     upsampler_p dsdpcm_converter_hq \
     scarletbook sacd_disc sacd_media sacd_dsdiff sacd_dsf \
     main \
     sacd

dst_ac: conststr.h types.h dst_ac.h dst_ac.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_ac.cpp -o libdstdec/dst_ac.o

dst_memory: types.h dst_memory.h dst_memory.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_memory.cpp -o libdstdec/dst_memory.o

dst_data: types.h dst_memory.h dst_data.h dst_data.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_data.cpp -o libdstdec/dst_data.o

ccp_calc: types.h ccp_calc.h ccp_calc.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/ccp_calc.cpp -o libdstdec/ccp_calc.o

dst_unpack: dst_data.h dst_unpack.h dst_unpack.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_unpack.cpp -o libdstdec/dst_unpack.o

dst_fram: types.h dst_ac.h dst_fram.h dst_unpack.h dst_fram.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_fram.cpp -o libdstdec/dst_fram.o

dst_init: types.h ccp_calc.h conststr.h dst_init.h dst_init.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_init.cpp -o libdstdec/dst_init.o

dst_decoder: types.h dst_unpack.h dst_fram.h dst_init.h dst_decoder.h dst_decoder.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_decoder.cpp -o libdstdec/dst_decoder.o

upsampler_p: dither.h upsampler.h upsampler.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdsd2pcm/upsampler.cpp -o libdsd2pcm/upsampler_p.o

dsdpcm_converter_hq: upsampler.h dsdpcm_converter_hq.h dsdpcm_converter_hq.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdsd2pcm/dsdpcm_converter_hq.cpp -o libdsd2pcm/dsdpcm_converter_hq.o

scarletbook: scarletbook.h scarletbook.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/scarletbook.cpp -o libsacd/scarletbook.o

sacd_disc: endianess.h scarletbook.h sacd_reader.h sacd_disc.h sacd_disc.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/sacd_disc.cpp -o libsacd/sacd_disc.o

sacd_media: scarletbook.h scarletbook.h sacd_media.h sacd_media.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/sacd_media.cpp -o libsacd/sacd_media.o

sacd_dsdiff: scarletbook.h sacd_dsd.h sacd_reader.h endianess.h sacd_dsdiff.h sacd_dsdiff.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/sacd_dsdiff.cpp -o libsacd/sacd_dsdiff.o

sacd_dsf: scarletbook.h sacd_dsd.h sacd_reader.h endianess.h sacd_dsf.h sacd_dsf.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/sacd_dsf.cpp -o libsacd/sacd_dsf.o

main: version.h sacd_reader.h sacd_disc.h sacd_dsdiff.h sacd_dsf.h dsdpcm_converter_hq.h main.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c main.cpp -o main.o

sacd: dst_memory.o ccp_calc.o dst_init.o dst_data.o dst_unpack.o dst_fram.o dst_decoder.o libdsd2pcm/dsdpcm_converter_hq.o sacd_media.o sacd_dsf.o sacd_dsdiff.o sacd_disc.o main.o
	$(CXX) $(CXXFLAGS) -o sacd libdsd2pcm/upsampler_p.o libdsd2pcm/dsdpcm_converter_hq.o libdstdec/dst_memory.o libdstdec/ccp_calc.o libdstdec/dst_init.o libdstdec/dst_data.o libdstdec/dst_unpack.o libdstdec/dst_fram.o libdstdec/dst_decoder.o libsacd/sacd_media.o libsacd/sacd_dsf.o libsacd/sacd_dsdiff.o libsacd/scarletbook.o libsacd/sacd_disc.o main.o $(LDFLAGS)

clean:
	rm -f sacd *.o $(foreach librarydir,$(LIBRARY_DIRS),$(librarydir)/*.o)

install: sacd

	install -d $(DESTDIR)/usr/bin
	install ./sacd $(DESTDIR)/usr/bin

