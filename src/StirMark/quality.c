/*----------------------------------------------------------------------------
 * StirMark -- Quality measures
 *
 * Here a few measures to be applied on still images are implemented
 * including:
 *   SNR
 *   PSNR
 * more to come...
 *
 * Copyright (c) 1997, 1999 by Fabien A. P. Petitcolas
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
 * $Header: /StirMark/quality.c 5     7/04/99 11:25 Fapp2 $
 *----------------------------------------------------------------------------
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stirmark.h"
#include "error.h"
#include "quality.h"
#include "image.h"

#define MAX_DOUBLE  (1E300)

#define LUMINANCE(p) (0.299 * (p).r + 0.587 * (p).g + 0.114 * (p).b )


/*----------------------------------------------------------------------------
 */
double MaxLuminance(IMAGE I)
{
    unsigned char *pI;
    int i;
    double max = 0.0;
    double LI;

    pI = I.img;
    for(i = 0; i < I.size.x * I.size.y; i++)
    {
        if (I.depth == 1)
            LI = *pI++;
        else
        {
            LI = LUMINANCE(*((pixel *)pI));
            pI += 3;
        }
        if (LI > max)
            max = LI;
    }
    return (max);
}


/*----------------------------------------------------------------------------
 */
double Norm2(IMAGE I)
{
    unsigned char *pI;
    int i;
    double sum = 0.0;
    double LI;

    pI = I.img;
    for(i = 0; i < I.size.x * I.size.y; i++)
    {
        if (I.depth == 1)
            LI = *pI++;
        else
        {
            LI = LUMINANCE(*((pixel *)pI));
            pI += 3;
        }
        sum += LI * LI;
    }
    return (sum);
}


/*----------------------------------------------------------------------------
 */
double DiffNorm2(IMAGE I, IMAGE J)
{
    unsigned char *pI, *pJ;
    int i;
    double sum = 0.0;
    double LI, LJ;

    if((I.size.x != J.size.x) ||
       (I.size.y != J.size.y) ||
       (I.depth != J.depth))
        return (MAX_DOUBLE);

    pI = I.img;
    pJ = J.img;
    for(i = 0; i < I.size.x * I.size.y; i++)
    {
        if (I.depth == 1)
        {
            LI = *pI++;
            LJ = *pJ++;
        }
        else
        {
            LI = LUMINANCE(*((pixel *)pI));
            LJ = LUMINANCE(*((pixel *)pJ));
            pI += 3;
            pJ += 3;
        }
        sum += (LI - LJ) * (LI - LJ);
    }
    return (sum);
}



/*----------------------------------------------------------------------------
 * Signal to noise ration (SNR)
 *
 *                         N   2
 *                        Sum X
 *                        i=1  i
 * SNR = 10 log_10   ----------------
 *                     N           2
 *                    Sum (X  - X')
 *                    i=1   i    i
 * where X_i and and X'_i are the original and modified pixel values and N the
 * total number of pixels in the image. SNR is used to measure the signal
 * quality.
 */
double SNR(IMAGE I, IMAGE J)
{
    if((I.size.x != J.size.x) || (I.size.y != J.size.y) || (I.depth != J.depth))
        return (MIN_QUALITY);

    return (10 * log10(Norm2(I) / DiffNorm2(I, J)));
}

/*----------------------------------------------------------------------------
 * Peak Signal to noise ration (PSNR)
 *
 *                                2
 *                        N  max X
 *                            i   i
 * PSNR = 10 log_10   ------------------
 *                       N           2
 *                      Sum (X  - X')
 *                      i=1   i    i
 * where X_i and and X'_i are the original and modified pixel values and N the
 * total number of pixels in the image. SNR is used to measure the visual
 * quality, even though it is not monotonically related to subjective visual
 * quality.
 */
double PSNR(IMAGE I, IMAGE J)
{
    double max;

    if((I.size.x != J.size.x) ||
       (I.size.y != J.size.y) ||
       (I.depth != J.depth))
        return (MIN_QUALITY);

    max = MaxLuminance(I);
    return (20 * log10(max) + 10 * log10(I.size.x * I.size.y)
                            - 10 * log10(DiffNorm2(I, J)));
}

