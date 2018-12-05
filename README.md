# sacd2wav

Origin: https://github.com/DocMarty84/sacd by Robert Tari <robert.tari@gmail.com> under GPL. 

Zhao Wang <zhaow.km@gmail.com> ported to macOS and adapted for Tascam DR-100.

## Description
A utility to convert SACD (Super Audio CD) iso image, Philips DSDIFF file or Sony DSF file 
to HiRes wave files ready for Tascam DR-100 Mark 2/3. Handles both DST and DSD streams.

Robert's original implementation is for Linux distributions. With little modification it can
be adopted to macOS (tested with Mojave). The extracted wave files can be played with VLC
without any issue. However, the wave files are using a subformat which is not supported by
Tascam HiRes player. Significant modifications are introduced to produce the plain PCM wave
files acceptable by Tascam.

Because Tascam DR-100 MKII supports 2CH, 24bit, 96kHZ PCM and MKIII supports 2CH, 24bit, 
192kHZ PCM. The implementation of the other sample rates is cleanup to make sure the codebase
containes only the necessary logics.

## Usage

sacd2wav -i infile [-o outdir] [options]

  -i, --infile         : Specify the input file (*.iso, *.dsf, *.dff)  
  -o, --outdir         : The folder to write the WAVE files to. If you omit
                         this, the files will be placed in the input file's
                         directory  
  -r, --rate           : The output sample rate.
                         Valid rates are: 96000 or 192000.
                         If you omit this, 96KHz will be used.  
  -h, --help           : Show this help message  

## Development

```
make
make install
```