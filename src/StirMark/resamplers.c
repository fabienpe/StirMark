/*----------------------------------------------------------------------------
 * StirMark -- Resampling routines
 *
 * Point by point or supersampling.
 *
 * This code was initially written by Neil Dodgson (nad@cl.cam.ac.uk)
 * and has been slightly modified for use into StirMark.
 *
 *
 * Copyright (c) 1997, 1999 Neil Dodgson
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
 * $Header: /StirMark/resamplers.c 8     7/04/99 11:26 Fapp2 $
 *----------------------------------------------------------------------------
 */

#include "stirmark.h"
#include "error.h"

#include "resamplers.h"
#include "reconstructers.h"
#include "transformations.h"


/* Default functions */
static BOOL nullFunction()  {return (TRUE);}

/* Point sampling */
static double samPointSample(double (*)(), double (*)(), PROCESS_INFO *);

/* Super sampling */
static double deltaCol, deltaRow;
static int    numSamples;
static BOOL   samSuperSampleDefine(PROCESS *);
static double samSuperSample(double (*)(), double (*)(), PROCESS_INFO *);


/* List of possible samplers together with their associated functions */
/* {Name, Description, 'S', Define, Setup, Initialise, Function, Cleanup */
PROCESS samplers[] = {
    {"Point sampler", "Point sampler",
     SAM_POINT, 'S', nullFunction, nullFunction,
     nullFunction, samPointSample, nullFunction},
    
    {"Super-sampler", "Super sampling",
     SAM_SUPER, 'S', samSuperSampleDefine, nullFunction,
     nullFunction, samSuperSample, nullFunction},
    
    {"", "", SAM_END_OF_LIST, 0, 0, 0, 0, 0, 0}
};


/*----------------------------------------------------------------------------
 * Point Sampler
 */
double samPointSample(double (*reconstructer)(),
                      double (*transformer)(),
                      PROCESS_INFO *info)
{
    double numCols;
    int    col, row, location, newval, k;

    numCols = info->dI.size.x;

    for(col = 0; col <= info->dI.size.x - 1; col++)
    {
        for(row = 0; row <= info->dI.size.y - 1; row++)
        {
            for (k = 0; k < info->dI.depth; k++)
            {
                newval = ROUND((*transformer)((double) col + 0.5, 
                        (double) row + 0.5, reconstructer, k, info));

                location = k + info->dI.depth * (col + row * (info->dI.size.x));
                assert(location >= 0 &&
                       location <= info->dI.size.x * info->dI.size.y
                                                   * info->dI.depth - 1);

                /* Thresholding */
                if (newval < 0) newval = 0;
                if (newval > info->dI.max) newval = info->dI.max;
      
                info->dI.img[location] = (unsigned char)newval;
            }
        }
    }
    
    return (TRUE); /* return zero to indicate correct termination */
}


/*----------------------------------------------------------------------------
 * Super-Sampler
 */

BOOL samSuperSampleDefine(PROCESS *info)
{
    int numInX, numInY;

    numInX = SS_DEF_X;
    numInY = SS_DEF_Y;
    
    assert(numInX >= 1 && numInY >= 1);

    deltaCol   = 1.0 / numInX;
    deltaRow   = 1.0 / numInY;
    numSamples = numInX * numInY;

    sprintf(info->description,
      "Super-sample: %d x %d regularly spaced samples per pixel",
       numInX, numInY);

    return (TRUE);
}


double samSuperSample(double (*reconstructer)(),
                      double (*transformer)(),
                      PROCESS_INFO *info)
{
    double numCols;
    int    col, row, location, newval, k;
    double x, y;
    double sum;

    numCols = info->dI.size.x;

    for(col = 0; col <= info->dI.size.x - 1; col++)
    {
        for(row = 0; row <= info->dI.size.y - 1; row++)
        {
            for (k = 0; k < info->dI.depth; k++)
            {
                sum = 0.0;
                for(x = col + 0.5 * deltaCol; x < (col + 1.0); x += deltaCol)
                {
                    for(y =  row + 0.5 * deltaRow;
                        y <  (row + 1.0);
                        y += deltaRow)
                        sum += (*transformer)(x, y, reconstructer, k, info);
                }
                
                newval = ROUND(sum / numSamples);

                location = k + info->dI.depth * (col + row * (info->dI.size.x));
                assert(location >= 0 && 
                       location <= info->dI.size.x * info->dI.size.y
                                                   * info->dI.depth - 1);

                /* thresholding */
                if (newval < 0) newval = 0;
                if (newval > info->dI.max) newval = info->dI.max;

                info->dI.img[location] = (unsigned char)newval;
            }
        }
    }
    return (TRUE); /* return zero to indicate correct termination */
}

