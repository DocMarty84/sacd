cmake_minimum_required(VERSION 3.12)
project(sacd)

set(CMAKE_CXX_STANDARD 17)

include_directories(libdsd2pcm)
include_directories(libdstdec)
include_directories(libsacd)

add_executable(sacd
        libdsd2pcm/dither.h
        libdsd2pcm/dsd_pcm_constants.h
        libdsd2pcm/dsd_pcm_converter.cpp
        libdsd2pcm/dsd_pcm_converter.h
        libdsd2pcm/upsampler.cpp
        libdsd2pcm/upsampler.h
        libdstdec/ac_data.cpp
        libdstdec/ac_data.h
        libdstdec/coded_table.cpp
        libdstdec/coded_table.h
        libdstdec/dst_consts.h
        libdstdec/dst_decoder.cpp
        libdstdec/dst_decoder.h
        libdstdec/dst_decoder_mt.cpp
        libdstdec/dst_decoder_mt.h
        libdstdec/dst_defs.h
        libdstdec/frame_reader.cpp
        libdstdec/frame_reader.h
        libdstdec/str_data.cpp
        libdstdec/str_data.h
        libsacd/endianess.h
        libsacd/sacd_disc.cpp
        libsacd/sacd_disc.h
        libsacd/sacd_dsd.h
        libsacd/sacd_dsdiff.cpp
        libsacd/sacd_dsdiff.h
        libsacd/sacd_dsf.cpp
        libsacd/sacd_dsf.h
        libsacd/sacd_media.cpp
        libsacd/sacd_media.h
        libsacd/sacd_reader.h
        libsacd/scarletbook.cpp
        libsacd/scarletbook.h
        libsacd/version.h
        converter_core.cpp
        converter_core.h
        main.cpp
        )

target_link_libraries(sacd iconv)