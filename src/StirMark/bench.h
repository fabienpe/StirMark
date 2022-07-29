/*----------------------------------------------------------------------------
 * StirMark -- Global definitions macros, types, etc.
 *
 * Copyright (c) 1997, 1999 Fabien A. P. Petitcolas and Markus G. Kuhn
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
 * $Header: /StirMark/bench.h 3     11/08/99 19:01 Fapp2 $
 *----------------------------------------------------------------------------
 */

#ifndef _BENCH_H_
#define _BENCH_H_

#include "stirmark.h"

/* Parameters used for testing */

#define INWARDS             (2.0) /* Default inward corner displacement */
#define OUTWARDS            (0.7) /* Default outward corner displacement */
#define RELATIVE_INWARDS    (1)
#define RELATIVE_OUTWARDS   (0)
#define DEVIATION           (1.5) /* Corlor deviation */
#define RANDOM_FACTOR       (0.1) /* Default randomisation factor */
#define BENDING_FACTOR      (2.0) /* Default bending deviation */
#define ROLLOFF             (1.0)
#define HIGHPASS            (0)
#define CUTOFF              (6)   /* Number of pseudo-period used in sinc */

#define JPEG_QUALITY_STIR   (70)  /* Def JPEG quality used after distortion */
#define JPEG_QUALITY        (90)  /* Def JPEG quality after other transformations */

/* List of rotation angles for the rotation + autocrop and */
/* rotation + autocrop + autoscale transformations         */
#define ROTATION_ANGLES  { -2.00, -1.00, -0.75, -0.5,  -0.25, \
                            2.00,  1.00,  0.75,  0.5,   0.25, \
                            5.00, 10.00, 15.00, 30.00, 45.00, \
                           90.00 } /* degrees */

/* List of scale factors for the rescaling transformation      */
/* First component is for the x axis, second component for the */
/* y axis. Different components change the aspect ratio of the */
/* picture. See ASPECT_RATIOS.                                 */
#define SCALE_FACTORS {{0.50, 0.50}, {0.75, 0.75}, {0.90, 0.90}, \
                       {1.10, 1.10}, {1.50, 1.50}, {2.00, 2.00}}

/* Change of aspect ratio tests. First component is for the x */
/* axis, second component for the y axis.                     */
#define ASPECT_RATIOS {{1.00, 0.80}, {1.00, 0.90}, {1.00, 1.10}, \
                       {1.00, 1.20}, {0.80, 1.00}, {0.90, 1.00}, \
                       {1.10, 1.00}, {1.20, 1.00}}

/* List of JPEG quality factors used for JPEG compression test */
#define JPEG_QUALITY_FACTORS {90, 80, 70, 60, 50, 40, \
                              35, 30, 25, 20, 15, 10} /* % */

/* Centered cropping test. Remove n% of the border.       */
/* In other word the size of the new width of the picutre */
/* is original_width * n / 100 and the new height is      */
/* original_height * n  / 100                             */
#define CROP_FACTORS {1, 2, 5, 10, 15, 20, 25, 50, 75} /* % */

/* Symmetric and assymetric rows and columns removal */
/* First component is number of columns removed and  */
/* second component the number or rows               */
#define NUMBER_ROW_COL_REMOVED {{ 1, 1}, {1,  5}, \
                                { 5, 1}, {5, 17},\
                                {17, 5}}

/* Shearing in X, Y and X-Y direction. The first component */
/* is the shift in X direction (% of width) and the second */
/* is the shift in the Y direction (% of height)           */
#define SHIFTS {{0, 1}, {0, 5}, \
                {1, 0}, {5, 0}, \
                {1, 1}, {5, 5}}

/* Matrices for convolution filters. See MAX_FILTER_SIZE to */
/* increase the maximum size of the filter. First component */
/* is a string containing the name of the filter, secon     */
/* component the actual size of the filter                  */
/* (less than MAX_FILTER_SIZE) and the trhird one is the    */
/* matrix of the filter.                                    */
#define FILTERS {{"Gaussian filtering", 3, {{ 1,  2,  1},  \
                                            { 2,  4,  2},  \
                                            { 1,  2,  1}}},\
                 /* Sharpening using Laplacian         
                    f'(i, j) = 5f(i, j) - [
                               f(i + 1, j) + f(i - 1, j) +
                               f(i, j + 1) + f(i, j -1) ] */ \
                 {"Sharpening", 3, {{ 0, -1,  0},  \
                                    {-1,  5, -1},  \
                                    { 0, -1,  0}}}}

/* List of sizes for median filters */
#define MEDIAN_FILTER_SIZES    {{3, 3}, {5, 5}, {7, 7}, {9, 9}}

/* List of parameters for general linear transformation.  */
/* These transformation are expressed with a 2x2 matrix   */
/* as follow:                                             */
/* |x'| = | a  b | |x|                                    */
/* |y'|   | c  d | |y|                                    */
/* where (x, y) are the coordinates of the original pixel */
/* and (x', y') the coordinates of the pixel after        */
/* transformation. Enter the coeeficients as follow:      */
/*                   {{a, b}, {c, d}}                     */
/* a = d = 1 and b = c = 0 corresponds to the identity.   */
/* Only three digits are displayed and used in the output */
/* filname but all are taken into account.                */
#define LINEAR_TRANSFORMATIONS \
                        {{{1.010, 0.013}, {0.009, 1.011}}, \
                         {{1.007, 0.010}, {0.010, 1.012}}, \
                         {{1.013, 0.008}, {0.011, 1.008}}}

/* List of parameters for geometrcal distortions. */
/* See struct OPTIONS                             */
/* Currently only default parameters              */
#define DISTORTIONS {{INWARDS, OUTWARDS, RELATIVE_INWARDS,  \
                      RELATIVE_OUTWARDS, RANDOM_FACTOR,     \
                      BENDING_FACTOR,    JPEG_QUALITY_STIR, \
                      DEVIATION,         DIST_OP_NEW,       \
                      NULL}}
/* Original StirMark {INWARDS, OUTWARDS, RELATIVE_INWARDS,
                      RELATIVE_OUTWARDS, 0, 0,
                      JPEG_QUALITY_STIR, DEVIATION,
                      DISP_OP_NEW, NULL}, */

/* FMLR attack parameters
 * See: R. Barnett and D.E. Pearson, Frequency mode LR attack operator
 * for digitally watermarked images, Electronics Letters, 34(19), Sept.
 * 1998, pp. 1837--1839. ISSN 0013-5194
 * 6 parameters corresponding to scale 1, 2, 4, 8, 16 and 32
 */
#define LR_STRENGTHS    {0.02,  0.02,  0.02, 0.0, 0.0, 0.0}
#define JNDLR_STRENGTHS {0.005, 0.005, 0.0,  0.0, 0.0, 0.0}
#define LR_QUALITY      (90)  /* JPEG quality factor used after LR attack */

#endif /* _BENCH_H_ */

