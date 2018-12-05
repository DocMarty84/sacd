# Sacd

Origin: https://github.com/DocMarty84/sacd by Robert Tari <robert.tari@gmail.com> under GPL. 

Zhao Wang <zhaow.km@gmail.com> ported to macOS.

## Description
Super Audio CD decoder. 
Converts SACD image files, Philips DSDIFF or Sony DSF file to 24-bit high resolution wave files. Handles both DST and DSD streams.

## Usage

sacd -i infile [-o outdir] [options]

  -i, --infile         : Specify the input file (*.iso, *.dsf, *.dff)  
  -o, --outdir         : The folder to write the WAVE files to. If you omit
                         this, the files will be placed in the input file's
                         directory  
  -r, --rate           : The output samplerate.
                         Valid rates are: 96000 or 192000.
                         If you omit this, 96KHz will be used.  
  -h, --help           : Show this help message  

