/*----------------------------------------------------------------------------
 * StirMark -- Definitions for image routines
 *
 * Copyright (c) 1997, 1999 by Fabien A. P. Petitcolas and Markus G. Kuhn
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any non-commercial purpose and without fee is hereby
 * granted (GPL), provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission notice
 * appear in supporting documentation. This software is provided "as is" 
 * without express or implied warranty. The authors shall not be held
 * liable in any event for incidental or consequential damages in
 * connection with, or arising out of, the furnishing, performance, or
 * use of this program.
 *
 * If you use StirMark for your research, please cite:
 *
 *   Fabien A. P. Petitcolas, Ross J. Anderson, Markus G. Kuhn. Attacks
 *   on copyright marking systems, in David Aucsmith (Ed), Information
 *   Hiding, Second International Workshop, IH'98, Portland, Oregon,
 *   USA, April 15--17, 1998, Proceedings, LNCS 1525, Springer-Verlag,
 *   ISBN 3-540-65386-4, pp. 219--239.
 *
 *   <http://www.cl.cam.ac.uk/~fapp2/papers/ih98-attacks/>
 *
 * and 
 *
 *   Martin Kutter and Fabien A. P. Petitcolas. A fair benchmark for
 *   image watermarking systems, To in E. Delp et al. (Eds), in
 *   vol. 3657, proceedings of Electronic Imaging '99, Security and
 *   Watermarking of Multimedia Contents, San Jose, CA, USA, 25--27
 *   January 1999. The International Society for Optical
 *   Engineering. To appear.
 *
 *   <http://www.cl.cam.ac.uk/~fapp2/papers/ei99-benchmark/>
 *
 * See the also the "Copyright" file provided in this package for
 * copyright information about code and libraries used in StirMark.
 *
 * $Header: /StirMark/image.h 9     7/04/99 11:28 Fapp2 $
 *----------------------------------------------------------------------------
 */


#ifndef _IMAGE_H_
#define _IMAGE_H_

void ImageNew(IMAGE *I, int width, int height, int depth, unsigned char max);
void ImageDuplicate(IMAGE *dI, const IMAGE *sI);
void ImageClear(IMAGE *I);
void ImageRead(IMAGE *pI, char *strzFilename);

void ImageReadPPM(IMAGE *I, FILE *stream);
void ImageSavePPM(IMAGE I, char *strzFilename);
void ImageWritePPM(IMAGE I, FILE *stream);

void ImageReadJPEG(IMAGE *I, FILE *stream);
void ImageSaveJPEG(IMAGE I, char *strzFilename, int quality);
void ImageWriteJPEG(IMAGE I, FILE *stream, int quality);

void ImageSaveGIF(IMAGE I, char *strzFilename);
void ImageWriteGIF(IMAGE I, FILE *outfile);

void *MemoryAlloc(size_t n);

#endif

