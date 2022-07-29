##############################################################################
#
# Basic Makefile for StirMark
#
# Uses libjpeg.a library.
#
# If you use StirMark for your research, please cite:
#
#   Fabien A. P. Petitcolas, Ross J. Anderson, Markus G. Kuhn. Attacks
#   on copyright marking systems, in David Aucsmith (Ed), Information
#   Hiding, Second International Workshop, IH'98, Portland, Oregon,
#   USA, April 15--17, 1998, Proceedings, LNCS 1525, Springer-Verlag,
#   ISBN 3-540-65386-4, pp. 219--239.
#
#   <http://www.cl.cam.ac.uk/~fapp2/papers/ih98-attacks/>
#
# and 
#
#   Martin Kutter and Fabien A. P. Petitcolas. A fair benchmark for
#   image watermarking systems, To in E. Delp et al. (Eds), in
#   vol. 3657, proceedings of Electronic Imaging '99, Security and
#   Watermarking of Multimedia Contents, San Jose, CA, USA, 25--27
#   January 1999. The International Society for Optical
#   Engineering. To appear.
#
#   <http://www.cl.cam.ac.uk/~fapp2/papers/ei99-benchmark/>
#
# See the also the "Copyright" file provided in this package for
# copyright information about code and libraries used in StirMark.
#
# $Header: /StirMark/Makefile 8     7/04/99 11:48 Fapp2 $
##############################################################################

# Compiler flags and options

CC = gcc -O6 -W -Wall -pedantic

# Dependancies

all: stirmark

stirmark: bench.o stirmark.o transformations.o image.o libjpeg.a \
	reconstructers.o resamplers.o quality.o quantise.o lrattack.o \
	error.o
	$(CC) -o stirmark bench.o stirmark.o transformations.o \
	quality.o quantise.o lrattack.o error.o image.o \
	reconstructers.o resamplers.o libjpeg.a -lm

# Dependancy line for the object files

libjpeg.a:
	cd ../JPEGLib/;./configure;make libjpeg.a
	ln -s ../JPEGLib/libjpeg.a .



bench.o: bench.c stirmark.h error.h image.h resamplers.h lrattack.h \
	transformations.h reconstructers.h quality.h quantise.h bench.h
	$(CC) -c bench.c

error.o: error.c error.h
	$(CC) -c error.c

image.o: image.c stirmark.h error.h ../JPEGLib/jpeglib.h
	$(CC) -c image.c

lrattack.o: lrattack.c lrattack.h
	$(CC) -c lrattack.c

quality.o: quality.c stirmark.h error.h image.h quality.h
	$(CC) -c quality.c

quantise.o: quantise.c quantise.h image.h error.h stirmark.h
	$(CC) -c quantise.c

reconstructers.o: reconstructers.c reconstructers.h stirmark.h
	$(CC) -c reconstructers.c

resamplers.o: resamplers.c stirmark.h error.h resamplers.h \
	reconstructers.h transformations.h
	$(CC) -c resamplers.c

stirmark.o: stirmark.c stirmark.h image.h bench.h
	$(CC) -c stirmark.c

transformations.o: transformations.c stirmark.h error.h \
	image.h transformations.h
	$(CC) -c transformations.c



clean:
	cd ../JPEGLib/;make clean
	rm -f *.o *.a *~

