/*----------------------------------------------------------------------------
 * StirMark -- Color quantisation
 *
 * Copyright (c) 1997, 1999 by Fabien A. P. Petitcolas
 *
 * This code is mostly based on ppmquant of the PBMPLUS distributionof
 * December 1991. That file contains the following copyright notice:
 *
 * Copyright (c) 1989, 1991 by Jef Poskanzer.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
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
 * $Header: /StirMark/quantise.h 4     7/04/99 11:28 Fapp2 $
 *----------------------------------------------------------------------------
 */

#ifndef _QUANTISE_H_
#define _QUANTISE_H_

void ColorQuantisation(IMAGE *dI, IMAGE *sI, int nColors);

#endif /* _QUANTISE_H_ */

