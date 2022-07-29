/*----------------------------------------------------------------------------
 * StirMark -- The Laplacian Removal attack.
 *
 * This code has been given by Richard Barnettm, University of Sussex.
 * This is an implementation of the Laplacian removal attack.
 *
 * See: R. Barnett and D.E. Pearson, Frequency mode LR attack operator
 * for digitally watermarked images, Electronics Letters, 34(19), Sept.
 * 1998, pp. 1837--1839. ISSN 0013-5194
 *
 * Copyright (c) 1998, 1999 by Richard Barnett
 *
 * The FML Operator uses code from the SIUE distribution:
 *
 * Copyright (c) 1996, 1997 SIUE - by Scott E Umbaugh.
 *
 * Permission to use, copy, modify, and distribute this 
 * software and its documentation for any non-commercial purpose and 
 * without fee is hereby granted, provided that the above 
 * copyright notice appear in all copies and that both 
 * that copyright notice and this permission notice appear
 * in supporting documentation. This software is provided "as is" 
 * without express or implied warranty. The developers
 * and SIUE shall not be held liable in any event for incidental
 * or consequential damages in connection with, or arising out of, 
 * the furnishing, performance, or use of this program.
 *
 * $Header: /StirMark/lrattack.h 5     18/01/99 18:01 Fapp2 $
 *----------------------------------------------------------------------------
 */


/* LRAttack.h - LR attack copnfigured for StirMark integration.
 *
 * Copyright (c) Richard Barnett 1998
 *
 * Rev 1.0 - 27/11/98
 *
 * This file contains defines for the LR attack when integrated
 * into the StirMark program.
 */


#ifndef _LRATTACK_H_
#define _LRATTACK_H_

#include "stirmark.h"

typedef struct    tagYUV
{
    double    Y;  /* luminance     */
    double    U;  /* U chrominance */
    double    V;  /* V chrominance */
} YUV;

#define IMAGE_WHITE   (255) /* white value */
#define IMAGE_GREY    (128) /* grey value  */
#define IMAGE_BLACK     (0) /* black       */


/* Function prototypes*/

int     LRAttack(IMAGE *pSImage, IMAGE *pDImage, double dStrength, double dJNDStrength, int nScale);
void    LRAddImage(YUV *pLap, YUV *pDim, int w, int h, double dStrength, double dJNDStrength);
void    LRSharpenLaplace(YUV *pSim, YUV *pDim, int w, int h);
void    LRShrinkImage(YUV *pSim, YUV *pDim, int w, int h);
void    LRExpandImage(YUV *pSim, YUV *pDim, int w, int h);
void    LRCopyImage(YUV *pSim, YUV *pDim, int w, int h);
void    LRHisteq(YUV *pSim, YUV *pDim, int w, int h);
void    LREqualiseLuminance(YUV *pRef, YUV *pDim, int w, int h);
void    LRImageToYUV(IMAGE *pSimage, YUV *pDim, int nw, int nh);
void    LRYUVToImage(YUV *pSim, IMAGE *pDImage, int nw);
int     LRRound(double val);
double *LRCalculateJND(YUV *image, int w, int h);
double  LRCalculateJNDbg(YUV *image, int col, int row, int w, int h);
double  LRCalculateJNDmg(YUV *image, int col, int row, int w, int h);
double  LRCalculateJNDmggrad(YUV *image, int col, int row, int w, int h, int *G);
int     LRInRange(int col, int row, int w, int h);

#endif /* _LRATTACK_H_ */

