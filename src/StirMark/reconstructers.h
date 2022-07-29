/*----------------------------------------------------------------------------
 * StirMark -- Definitions for reconstruction routines
 *
 * Copyright (c) 1997, 1999 by Neil Dodgson and Fabien A. P. Petitcolas
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
 * $Header: /StirMark/reconstructers.h 11    7/04/99 11:28 Fapp2 $
 *----------------------------------------------------------------------------
 */

#ifndef _RECONSTRUCTERS_H_
#define _RECONSTRUCTERS_H_


/* Default parameter */

#define QUADRATIC_PARAMETER (1)   /* reconstructer parameter */
#define SINC_RESOLUTION     (20)  /* number of table points per unity distance */


/* Possible reconstructers */

#define REC_END_OF_LIST         (0)
#define REC_LINEAR              (1)
#define REC_OVERHAUSER_CUBIC    (2)
#define REC_TWO_POINT_CUBIC     (3)
#define REC_INTERP_QUADRATIC    (4)
#define REC_APPROC_QUADRATIC    (5)
#define REC_APPROC_B_SPLINE     (6)
#define REC_GENRAL_QUADRATIC    (7)
#define REC_NULL                (8)
#define REC_MEDIAN_FILTER       (9)
#define REC_CONVOLUTION_FILTER  (10)
#define REC_NYQUIST             (11)
#define REC_NEAREST_NEIGHBOUR   (12)

/* Default reconstructer  used when resampling needed */

#define DEFAULT_RECONSTRUCTER   (REC_INTERP_QUADRATIC)


/* Macros */

#define ROUND(x)  ((x) < 0 ? (int) ((x) - 0.5) : (int) ((x) + 0.5))

#define FLOOR(x)  ((x) < 0 ? (int) ((x) - 1) : (int) (x))

#define ABS(x)    (((x) > 0) ? (x) : (-(x)))


/* Global variable */

extern PROCESS reconstructers[];

#endif

