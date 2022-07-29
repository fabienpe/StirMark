/*----------------------------------------------------------------------------
 * StirMark -- Definitions for transformation routines
 *
 * Copyright (c) 1997, 1999 by Fabien A. P. Petitcolas and Markus G. Kuhn
 * Part of this code is based on Neil Dodgson's work.
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
 *   Fabien A. P. Petitcolas and Ross J. Anderson, Evaluation of 
 *   copyright marking systems. To be presented at IEEE Multimedia
 *   Systems (ICMCS'99), 7--11 June 1999, Florence, Italy. 
 *
 * See the also the "Copyright" file provided in this package for
 * copyright information about code and libraries used in StirMark.
 *
 * $Header: /StirMark/transformations.h 12    7/04/99 11:28 Fapp2 $
 *----------------------------------------------------------------------------
 */

#ifndef _TRANSFORMATIONS_H_
#define _TRANSFORMATIONS_H_


/* Basic macros */
#ifndef ABS
#define ABS(x)      (((x) > 0) ? (x) : (-(x)))
#endif

#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#define MIN(a, b)   (((a) < (b)) ? (a) : (b)) 


/* Vectorial stuff for geometrical distortion attack */
#define V(AB, A, B)    (AB.x = B.x - A.x, AB.y = B.y - A.y)
#define P(A, ax, ay)   (A.x = ax, A.y = ay)
#define ADD(v, v1, v2) (v.x = v1.x + v2.x, v.y = v1.y + v2.y)


/* Possible transformers */
#define TRA_END_OF_LIST      (0)
#define TRA_SCALE_BY_VALUE   (1)
#define TRA_ROTATION         (2)
#define TRA_DISTORTION       (3)
#define TRA_ROTATION_SCALE   (4)
#define TRA_FLIP             (5)
#define TRA_CROPPING         (6)
#define TRA_INVARIANT        (7)
#define TRA_ROW_COL_REMOVAL  (8)
#define TRA_SHEAR            (9)
#define TRA_LINEAR          (10)

extern PROCESS transformers[];

#endif

