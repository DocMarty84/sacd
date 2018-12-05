CXX = g++

CXXFLAGS = -std=c++17 -Wall

VPATH = libdstdec:libdsd2pcm:libsacd

INCLUDE_DIRS = libdstdec libdsd2pcm libsacd
CPPFLAGS = $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir)) -Ofast -flto

LIBRARIES = iconv pthread
LIBRARY_DIRS = libdstdec libdsd2pcm libsacd
LDFLAGS = $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(LIBRARIES),-l$(library))

.PHONY: all clean install

all: clean str_data ac_data coded_table frame_reader dst_decoder dst_decoder_mt \
     upsampler dsd_pcm_converter \
     scarletbook sacd_disc sacd_media sacd_dsdiff sacd_dsf \
     main converter_core \
     sacd

str_data: dst_defs.h str_data.h str_data.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/str_data.cpp -o libdstdec/str_data.o

ac_data: dst_defs.h ac_data.h ac_data.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/ac_data.cpp -o libdstdec/ac_data.o

coded_table: dst_defs.h coded_table.h coded_table.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/coded_table.cpp -o libdstdec/coded_table.o

frame_reader: str_data.h coded_table.h frame_reader.h frame_reader.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/frame_reader.cpp -o libdstdec/frame_reader.o

dst_decoder: str_data.h ac_data.h coded_table.h frame_reader.h dst_decoder.h dst_decoder.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_decoder.cpp -o libdstdec/dst_decoder.o

dst_decoder_mt: dst_decoder.h dst_decoder_mt.h dst_decoder_mt.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_decoder_mt.cpp -o libdstdec/dst_decoder_mt.o

upsampler: dither.h upsampler.h upsampler.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdsd2pcm/upsampler.cpp -o libdsd2pcm/upsampler.o

dsd_pcm_converter: upsampler.h dsd_pcm_converter.h dsd_pcm_converter.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdsd2pcm/dsd_pcm_converter.cpp -o libdsd2pcm/dsd_pcm_converter.o

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

main: version.h sacd_reader.h sacd_disc.h sacd_dsdiff.h sacd_dsf.h dsd_pcm_converter.h main.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c main.cpp -o main.o

converter_core: version.h sacd_reader.h sacd_disc.h sacd_dsdiff.h sacd_dsf.h dsd_pcm_converter.h converter_core.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c converter_core.cpp -o converter_core.o

sacd: frame_reader.o ac_data.o str_data.o coded_table.o dst_decoder.o dst_decoder_mt.o dsd_pcm_converter.o sacd_media.o sacd_dsf.o sacd_dsdiff.o sacd_disc.o main.o converter_core.o
	$(CXX) $(CXXFLAGS) -o sacd libdsd2pcm/upsampler.o libdsd2pcm/dsd_pcm_converter.o libdstdec/frame_reader.o libdstdec/ac_data.o libdstdec/str_data.o libdstdec/coded_table.o libdstdec/dst_decoder.o libdstdec/dst_decoder_mt.o libsacd/sacd_media.o libsacd/sacd_dsf.o libsacd/sacd_dsdiff.o libsacd/scarletbook.o libsacd/sacd_disc.o main.o converter_core.o $(LDFLAGS)

clean:
	rm -f sacd *.o $(foreach librarydir,$(LIBRARY_DIRS),$(librarydir)/*.o)

install: sacd

	install -d $(DESTDIR)/usr/bin
	install ./sacd $(DESTDIR)/usr/bin

