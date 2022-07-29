/*----------------------------------------------------------------------------
 * StirMark -- Reconstruction routines
 *
 * Includes various resampling methods.
 *
 * The code for resampling was originally written by Neil Dodgson
 * (nad@cl.cam.ac.uk) for another project. The resampling algorithms
 * are explained in:
 *   Neil Dodgsn, "Quadratic Interpolation for Image Resampling,"
 *   IEEE Transactions on Image Processing, vol. 6, no. 9,
 *   pp. 1322--1326, Sept. 1997.
 *
 *
 * Note that the coordinates in this sub-system obey the idea set out in:
 *   Paul S. Heckbert, What are the coordinates of a pixel, in Andrew S. 
 *   Glassner Ed., Graphics Gems, pp. 246--248, 1990. ISBN 0-12-286165-5
 *
 * The discrete coordinates are offset from the continuous ones by half
 * a unit:
 *
 *      0       1       2       3
 *      *       *       *       *      discrete
 *  +-------+-------+-------+-------+  continuous
 *  0       1       2       3       4
 *
 * Thus: the conversion from continuous to discrete is:
 *        discrete = TRUNC(continuous)
 * i.e. pixel 0 goes from 0.0 to 0.999...; pixel 1 from 1.0 to 1.999... etc.
 *
 * Thus the continuous coordinate of the centre of pixel n is (n+0.5)
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
 * $Header: /StirMark/reconstructers.c 11    12/08/99 19:40 Fapp2 $
 *----------------------------------------------------------------------------
 */

#include <malloc.h>
#include <math.h>
#include <string.h>

#include "stirmark.h"
#include "reconstructers.h"

void *MemoryAlloc(size_t n); /* It's in image.c */

/* Default functions */
static BOOL nullFunction(PROCESS_INFO *info) {return (TRUE); UNREFERENCED_PARAMETER(info)}
static BOOL defRecSetup(PROCESS_INFO *);
static double nullReconstructor(double, double, int, PROCESS_INFO *);


/* Linear and cubic reconstrcuters */
static double recNearestNeighbour(double, double, int, PROCESS_INFO *);
static double recLinearInterpolation(double, double, int, PROCESS_INFO *);
static double recOverhauserInterpolation(double, double, int, PROCESS_INFO *);
static double recTwoPointCubic(double, double, int, PROCESS_INFO *);
static double recApproxCubicBSpline(double, double, int, PROCESS_INFO *);


/* Quadratic reconstructers
 * See:  Neil Dodgsn, "Quadratic Interpolation for Image Resampling,"
 *       IEEE Transactions on Image Processing, vol. 6, no. 9,
 *       pp. 1322--1326, Sept. 1997.
 */
static double quadraticparameter;
static double quadparam0, quadparam1, quadparam2;
static BOOL   recGeneralQuadraticDefine(PROCESS *);
static double recGeneralQuadratic(double, double, int, PROCESS_INFO *);
static double recApproxQuadratic(double, double, int, PROCESS_INFO *);
static double recInterpQuadratic(double, double, int, PROCESS_INFO *);

/* Nyquist interpolation */
static int    cutoff = 6; /* sinc(x) = 0 for all x > cutoff */
static double *SincTable; /* array of values for sinc interpolation, */
                          /* size = SINC_RESOLUTION * cutoff + 1     */
static BOOL   recNyquistInterpSetup(PROCESS_INFO *);
static double recNyquistInterp(double, double, int, PROCESS_INFO *);
static BOOL   recNyquistInterpCleanup(PROCESS_INFO *);

/* Filters - These are not really reconstructers but their */
/*           definition fits nicely into the structure     */
static IVECTOR offset; /* Offset of matrix / pixel */

/* Median filtering */
static double *table;
static int    iMedian;
static BOOL   recMedianFilterSetup(PROCESS_INFO *);
static double recMedianFilter(double, double, int, PROCESS_INFO *);
static BOOL   recMedianFilterCleanup(PROCESS_INFO *);

/* Convolution filter */
static int    sum; /* sum of the coefficients in the filter */
static BOOL   recConvolutionFilterSetup(PROCESS_INFO *);
static double recConvolutionFilter(double, double, int, PROCESS_INFO *);



/* List of possible reconstructer together with their associated functions */
/* {Name, Description, 'T', Define, Setup, Initialise, Function, Cleanup   */
PROCESS reconstructers[] = {
    {"Nyquist", "Nyquist interpolation",
     REC_NYQUIST, 'R', nullFunction, recNyquistInterpSetup, 
     nullFunction, recNyquistInterp, recNyquistInterpCleanup},
    
    {"Nearest-neighbour", "Nearest-neighbour",
     REC_NEAREST_NEIGHBOUR, 'R', nullFunction, defRecSetup,
     nullFunction, recNearestNeighbour, nullFunction},
    
    {"Linear interpolation", "Linear interpolation", REC_LINEAR, 'R',
     nullFunction, defRecSetup, nullFunction, 
     recLinearInterpolation, nullFunction},
    
    {"Overhauser cubic interpolation", "Overhauser cubic interpolation",
     REC_OVERHAUSER_CUBIC, 'R', nullFunction, defRecSetup,
     nullFunction, recOverhauserInterpolation, nullFunction},
    
    {"Two point cubic interpolation", "Two point cubic interpolation",
     REC_TWO_POINT_CUBIC, 'R', nullFunction, defRecSetup,
     nullFunction, recTwoPointCubic, nullFunction },
    
    {"Interpolating quadratic", "Interpolating quadratic",
     REC_INTERP_QUADRATIC, 'R', nullFunction, defRecSetup,
     nullFunction, recInterpQuadratic, nullFunction },
    
    {"Approximating quadratic", "Approximating quadratic",
     REC_APPROC_QUADRATIC, 'R', nullFunction, defRecSetup,
     nullFunction, recApproxQuadratic, nullFunction },
    
    {"Approximating cubic B-spline", "Approximating cubic B-spline", 
     REC_APPROC_B_SPLINE, 'R', nullFunction, defRecSetup,
     nullFunction, recApproxCubicBSpline, nullFunction },
    
    {"General quadratic", "SET BY General quadratic",
     REC_GENRAL_QUADRATIC, 'R', recGeneralQuadraticDefine, defRecSetup,
     nullFunction, recGeneralQuadratic, nullFunction },
    
    {"Null reconstructor", "Null reconstructor",
     REC_NULL, 'R', nullFunction, defRecSetup, 
     nullFunction, nullReconstructor, nullFunction },
    
    {"Median", "Median filtering",
     REC_MEDIAN_FILTER, 'T', nullFunction, recMedianFilterSetup,
     nullFunction, recMedianFilter, recMedianFilterCleanup},
    
    {"Filter", "Convolution filter",
     REC_CONVOLUTION_FILTER, 'T', nullFunction, recConvolutionFilterSetup,
     nullFunction, recConvolutionFilter, nullFunction},
    
    {"", "", REC_END_OF_LIST, 0, 0, 0, 0, 0, 0}
};


/*---------------------------------------------------------------------------
 * Used when pixels outside the image are need for the interpolation
 * or the convolution.
 */
static void _MirrorBorders(IVECTOR *M, IVECTOR size)
{
    while (M->x < 0 || M->x >= size.x)
    {
        if (M->x < 0) M->x = - M->x;
        if (M->x >= size.x) M->x = 2 * (size.x - 1) - M->x;
    }
    while (M->y < 0 || M->y >= size.y)
    {
        if (M->y < 0) M->y = - M->y;
        if (M->y >= size.y) M->y = 2 * (size.y - 1) - M->y;
    }
}


static unsigned char _GetPixelValue(int PVcol,
                                    int PVrow,
                                    int k,
                                    PROCESS_INFO *PVinfo)
{
    IVECTOR M;

    M.x = PVcol + PVinfo->offset.x;
    M.y = PVrow + PVinfo->offset.y;
    
    if (PVinfo->mirrorBorders == 1)
        _MirrorBorders(&M, PVinfo->sI.size);
    else
    {
        if (M.x < 0 || M.x >= PVinfo->sI.size.x || 
            M.y < 0 || M.y >= PVinfo->sI.size.y)
        return 0;
    }

    return (PVinfo->sI.img[k + PVinfo->sI.depth * ((M.y)*(PVinfo->sI.size.x)
                             + (M.x))]);
}



/*----------------------------------------------------------------------------
 * Reconstructors
 *----------------------------------------------------------------------------
 */


/*----------------------------------------------------------------------------
 * Default Reconstructer Setup
 */
BOOL defRecSetup(PROCESS_INFO *info)
{
    return (TRUE);
    UNREFERENCED_PARAMETER(info)
}



/*----------------------------------------------------------------------------
 * Null Reconstruction -- returns a gray value
 */
double nullReconstructor(double x, double y, int k, PROCESS_INFO *info)
{
    return (0.0);
    UNREFERENCED_PARAMETER(info)
    UNREFERENCED_PARAMETER(x)
    UNREFERENCED_PARAMETER(y)
    UNREFERENCED_PARAMETER(k)
}



/*----------------------------------------------------------------------------
 * Nearest-neighbour Reconstruction
 */
double recNearestNeighbour(double x, double y, int k, PROCESS_INFO *info)
{
    int col, row;

    col = FLOOR(x);
    row = FLOOR(y);

    return (_GetPixelValue(col, row, k, info));
}



/*----------------------------------------------------------------------------
 * Linear Interpolation
 */
double recLinearInterpolation(double x, double y, int k, PROCESS_INFO *info)
{
    double xMinusHalf, yMinusHalf;
    int    col, row;
    double fracX, fracY;
    double oneMinusFracX;

    xMinusHalf = x - 0.5;
    yMinusHalf = y - 0.5;
    col = FLOOR(xMinusHalf);
    row = FLOOR(yMinusHalf);
    fracX = xMinusHalf - col;
    fracY = yMinusHalf - row;
    oneMinusFracX = 1 - fracX;

    return (( _GetPixelValue(col,     row,     k, info) * oneMinusFracX
           +  _GetPixelValue(col + 1, row,     k, info) * fracX) * (1 - fracY)
           + (_GetPixelValue(col,     row + 1, k, info) * oneMinusFracX
           +  _GetPixelValue(col + 1, row + 1, k, info) * fracX) * fracY);
}



/*----------------------------------------------------------------------------
 * Overhauser Cubic Interpolation
 *
 * The old version used oiU(s) for all calls to oiU.  An optimisation is
 * to use prior knowledge of the range in which each call will fall to 
 * define four oiUx(s)'s where x = -2, -1, 0, 1 is the lower limit of
 * the unit-length range of s calculated by the special case oiUx.
 * This should speed up execution a little bit!!!
 */
#define oiU(s) \
(((s) < -2) ? 0.0 : \
    ((s) < -1) ? (2.0 + (s) * (4.0 + (s) * (2.5 + (s) * 0.5))) : \
    ((s) <  0) ? (1.0 + (s) * (s) * ((-2.5) + (s) * (-1.5))) : \
    ((s) <  1) ? (1.0 + (s) * (s) * ((-2.5) + (s) * 1.5)) : \
    ((s) <  2) ? (2.0 + (s) * ((-4.0) + (s) * (2.5 + (s) * (-0.5)))) : \
    0.0)
#define oiU_2(s) (2.0 + (s) * (4.0 + (s) * (2.5 + (s) * 0.5)))
#define oiU_1(s) (1.0 + (s) * (s) * ((-2.5) + (s) * (-1.5)))
#define oiU0(s)  (1.0 + (s) * (s) * ((-2.5) + (s) * 1.5))
#define oiU1(s)  (2.0 + (s) * ((-4.0) + (s) * (2.5 + (s) * (-0.5))))

double recOverhauserInterpolation(double x,
                                  double y,
                                  int k,
                                  PROCESS_INFO *info)
{
    double xMinusHalf, yMinusHalf;
    int    col, row;
    double fracX, fracY;
    double fx1, fx_1, fx_2, fy1, fy_1, fy_2;
    double ufx1, ufx0, ufx_1, ufx_2;
    double sum;

    xMinusHalf = x - 0.5;
    yMinusHalf = y - 0.5;
    col = FLOOR(xMinusHalf);
    row = FLOOR(yMinusHalf);
    fracX = xMinusHalf - col;
    fracY = yMinusHalf - row;

    fx1  = fracX + 1;
    fx_1 = fracX - 1;
    fx_2 = fracX - 2;

    fy1  = fracY + 1;
    fy_1 = fracY - 1;
    fy_2 = fracY - 2;

    ufx1  = oiU1(fx1);
    ufx0  = oiU0(fracX);
    ufx_1 = oiU_1(fx_1);
    ufx_2 = oiU_2(fx_2);

    sum =  (_GetPixelValue(col-1, row-1, k, info) * ufx1
        +   _GetPixelValue(col,   row-1, k, info) * ufx0
        +   _GetPixelValue(col+1, row-1, k, info) * ufx_1
        +   _GetPixelValue(col+2, row-1, k, info) * ufx_2) * oiU1(fy1);
    
    sum += (_GetPixelValue(col-1, row,   k, info) * ufx1
        +   _GetPixelValue(col,   row,   k, info) * ufx0
        +   _GetPixelValue(col+1, row,   k, info) * ufx_1
        +   _GetPixelValue(col+2, row,   k, info) * ufx_2) * oiU0(fracY);
    
    sum += (_GetPixelValue(col-1, row+1, k, info) * ufx1
        +   _GetPixelValue(col,   row+1, k, info) * ufx0
        +   _GetPixelValue(col+1, row+1, k, info) * ufx_1
        +   _GetPixelValue(col+2, row+1, k, info) * ufx_2) * oiU_1(fy_1);
    
    sum += (_GetPixelValue(col-1, row+2, k, info) * ufx1
        +   _GetPixelValue(col,   row+2, k, info) * ufx0
        +   _GetPixelValue(col+1, row+2, k, info) * ufx_1
        +   _GetPixelValue(col+2, row+2, k, info) * ufx_2) * oiU_2(fy_2);
  return (sum);
}

/*----------------------------------------------------------------------------
 *
 */
double recApproxCubicBSpline(double x, double y, int k, PROCESS_INFO *info)
#define Fr6 0.1666666666666666667
#define Fr3 0.3333333333333333333
{
    double xMinusHalf, yMinusHalf;
    int    col, row;
    double t, s;
    double ufx1, ufx0, ufx_1, ufx_2;
    double ufy1, ufy0, ufy_1, ufy_2;
    double sum;

    xMinusHalf = x - 0.5;
    yMinusHalf = y - 0.5;
    col = FLOOR(xMinusHalf);
    row = FLOOR(yMinusHalf);
    t = xMinusHalf - col;
    s = yMinusHalf - row;

    ufx1  = (Fr6) + t * (-0.5 + t * (0.5 + t * -1.0 * (Fr6)));
    ufx0  = 2.0 * (Fr3) + t * t * (-1.0 + t * 0.5);
    ufx_1 = (Fr6) + t * (0.5 + t * (0.5 + t * -0.5));
    ufx_2 = t * t * t * (Fr6);

    ufy1  = (Fr6) + s * (-0.5 + s * (0.5 + s * -1.0 * (Fr6)));
    ufy0  = 2.0 * (Fr3) + s * s * (-1.0 + s * 0.5);
    ufy_1 = (Fr6) + s * (0.5 + s * (0.5 + s * -0.5));
    ufy_2 = s * s * s * (Fr6);

    sum =  (_GetPixelValue(col-1, row-1, k, info) * ufx1
        +   _GetPixelValue(col,   row-1, k, info) * ufx0
        +   _GetPixelValue(col+1, row-1, k, info) * ufx_1
        +   _GetPixelValue(col+2, row-1, k, info) * ufx_2) * ufy1;

    sum += (_GetPixelValue(col-1, row,   k, info) * ufx1
        +   _GetPixelValue(col,   row,   k, info) * ufx0
        +   _GetPixelValue(col+1, row,   k, info) * ufx_1
        +   _GetPixelValue(col+2, row,   k, info) * ufx_2) * ufy0;

    sum += (_GetPixelValue(col-1, row+1, k, info) * ufx1
        +   _GetPixelValue(col,   row+1, k, info) * ufx0
        +   _GetPixelValue(col+1, row+1, k, info) * ufx_1
        +   _GetPixelValue(col+2, row+1, k, info) * ufx_2) * ufy_1;

    sum += (_GetPixelValue(col-1, row+2, k, info) * ufx1
        +   _GetPixelValue(col,   row+2, k, info) * ufx0
        +   _GetPixelValue(col+1, row+2, k, info) * ufx_1
        +   _GetPixelValue(col+2, row+2, k, info) * ufx_2) * ufy_2;
    return (sum);
}


/*----------------------------------------------------------------------------
 * Two Point Cubic Interpolation
 */
double recTwoPointCubic(double x, double y, int k, PROCESS_INFO *info)
{
    double xMinusHalf, yMinusHalf;
    int    col, row;
    double t, s; /* x and y fractional parts */
    double coeffX1, coeffX2; 
    double coeffY1, coeffY2;

    xMinusHalf = x - 0.5;
    yMinusHalf = y - 0.5;
    col = FLOOR(xMinusHalf);
    row = FLOOR(yMinusHalf);
    t = xMinusHalf - col;
    s = yMinusHalf - row;
    coeffX1 = 1.0 + (t * t * (-3.0 + t *  2.0));
    coeffX2 =       (t * t * ( 3.0 + t * -2.0));
    coeffY1 = 1.0 + (s * s * (-3.0 + s *  2.0));
    coeffY2 =       (s * s * ( 3.0 + s * -2.0));
    
    return (( _GetPixelValue(col,     row,     k, info) * coeffX1
           +  _GetPixelValue(col + 1, row,     k, info) * coeffX2) * coeffY1
           + (_GetPixelValue(col,     row + 1, k, info) * coeffX1
           +  _GetPixelValue(col + 1, row + 1, k, info) * coeffX2) * coeffY2);
}

/*----------------------------------------------------------------------------
 * Interpolating quadratic
 */
double recInterpQuadratic(double x, double y, int k, PROCESS_INFO *info)
{
    double xMinusHalf, yMinusHalf;
    int    col, row;
    double t, s; /* x and y fractional parts */
    double coeffX0, coeffX1, coeffX2; 
    double coeffY0, coeffY1, coeffY2;

    xMinusHalf = x - 0.5;
    yMinusHalf = y - 0.5;
    col = ROUND(xMinusHalf);
    row = ROUND(yMinusHalf);
    t = xMinusHalf - col;
    s = yMinusHalf - row;
    coeffX0 =       (t * (t - 0.5));
    coeffX1 = 1.0 + (-2.0 * t * t);
    coeffX2 =       (t * (t + 0.5));
    coeffY0 =       (s * (s - 0.5));
    coeffY1 = 1.0 + (-2.0 * s * s);
    coeffY2 =       (s * (s + 0.5));

    return ((_GetPixelValue(col - 1, row - 1, k, info) * coeffX0
         +   _GetPixelValue(col,     row - 1, k, info) * coeffX1
         +   _GetPixelValue(col + 1, row - 1, k, info) * coeffX2) * coeffY0
         + ( _GetPixelValue(col - 1, row,     k, info) * coeffX0
         +   _GetPixelValue(col,     row,     k, info) * coeffX1
         +   _GetPixelValue(col + 1, row,     k, info) * coeffX2) * coeffY1
         + ( _GetPixelValue(col - 1, row + 1, k, info) * coeffX0
         +   _GetPixelValue(col,     row + 1, k, info) * coeffX1
         +   _GetPixelValue(col + 1, row + 1, k, info) * coeffX2) * coeffY2);
}


/*----------------------------------------------------------------------------
 * Approximating quadratic
 */

double recApproxQuadratic(double x, double y, int k, PROCESS_INFO *info)
{
    double xMinusHalf, yMinusHalf;
    int    col, row;
    double t, s; /* x and y fractional parts */
    double coeffX0, coeffX1, coeffX2; 
    double coeffY0, coeffY1, coeffY2;

    xMinusHalf = x - 0.5;
    yMinusHalf = y - 0.5;
    col = ROUND(xMinusHalf);
    row = ROUND(yMinusHalf);
    t = xMinusHalf - col;
    s = yMinusHalf - row;
    coeffX0 = 0.125 + (t * (-0.5 + 0.5 * t));
    coeffX1 = 0.75  + (-t * t);
    coeffX2 = 0.125 + (t * ( 0.5 + 0.5 * t));
    coeffY0 = 0.125 + (s * (-0.5 + 0.5 * s));
    coeffY1 = 0.75  + (-s * s);
    coeffY2 = 0.125 + (s * ( 0.5 + 0.5 * s));

    return ((_GetPixelValue(col - 1, row - 1, k, info) * coeffX0
         +   _GetPixelValue(col,     row - 1, k, info) * coeffX1
         +   _GetPixelValue(col + 1, row - 1, k, info) * coeffX2) * coeffY0
         + ( _GetPixelValue(col - 1, row,     k, info) * coeffX0
         +   _GetPixelValue(col,     row,     k, info) * coeffX1
         +   _GetPixelValue(col + 1, row,     k, info) * coeffX2) * coeffY1
         + ( _GetPixelValue(col - 1, row + 1, k, info) * coeffX0
         +   _GetPixelValue(col,     row + 1, k, info) * coeffX1
         +   _GetPixelValue(col + 1, row + 1, k, info) * coeffX2) * coeffY2);

}


/*----------------------------------------------------------------------------
 * General quadratic reconstruction
 */
BOOL recGeneralQuadraticDefine(PROCESS *info)
{
    quadraticparameter = QUADRATIC_PARAMETER;
    quadparam0 = 0.25 * (1 - quadraticparameter);
    quadparam1 = 0.5  * (1 + quadraticparameter);
    quadparam2 = -2.0 * quadraticparameter;

    sprintf(info->description,
        "Quadratic reconstruction; parameter =%7.4f", quadraticparameter);
    
    return (TRUE);
}
    
double recGeneralQuadratic(double x, double y, int k, PROCESS_INFO *info)
{
    double xMinusHalf, yMinusHalf;
    int    col, row;
    double t, s; /* x and y fractional parts */
    double coeffX0, coeffX1, coeffX2; 
    double coeffY0, coeffY1, coeffY2;

    xMinusHalf = x - 0.5;
    yMinusHalf = y - 0.5;
    col = ROUND(xMinusHalf);
    row = ROUND(yMinusHalf);
    t = xMinusHalf - col;
    s = yMinusHalf - row;
    coeffX0 = quadparam0 + (t * (-0.5 + quadraticparameter * t));
    coeffX1 = quadparam1 + (quadparam2 * t * t);
    coeffX2 = quadparam0 + (t * ( 0.5 + quadraticparameter * t));
    coeffY0 = quadparam0 + (s * (-0.5 + quadraticparameter * s));
    coeffY1 = quadparam1 + (quadparam2 * s * s);
    coeffY2 = quadparam0 + (s * ( 0.5 + quadraticparameter * s));
    
    return ((_GetPixelValue(col - 1, row - 1, k, info) * coeffX0
         +   _GetPixelValue(col,     row - 1, k, info) * coeffX1
         +   _GetPixelValue(col + 1, row - 1, k, info) * coeffX2) * coeffY0
         + ( _GetPixelValue(col - 1, row,     k, info) * coeffX0
         +   _GetPixelValue(col,     row,     k, info) * coeffX1
         +   _GetPixelValue(col + 1, row,     k, info) * coeffX2) * coeffY1
         + ( _GetPixelValue(col - 1, row + 1, k, info) * coeffX0
         +   _GetPixelValue(col,     row + 1, k, info) * coeffX1
         +   _GetPixelValue(col + 1, row + 1, k, info) * coeffX2) * coeffY2);
}


/*----------------------------------------------------------------------------
 * Median filtering
 */
BOOL recMedianFilterSetup(PROCESS_INFO *info)
{
    IVECTOR *pFilterSize;

    assert((info != NULL) && (info->param != NULL));
    pFilterSize = (IVECTOR *)(info->param);

  	sprintf(info->pszTestName, "%dx%d Median Filtering", 
            pFilterSize->x, pFilterSize->y);
	sprintf(info->pszFileName, "_%dx%d_median_filter", 
            pFilterSize->x, pFilterSize->y);

    iMedian = (int) (pFilterSize->x * pFilterSize->y / 2);

    if ((table = (double *) MemoryAlloc(pFilterSize->x * pFilterSize->y * 
                                        sizeof(double))) == NULL)
        return (FALSE);

    offset.x = (int) (pFilterSize->x / 2);
    offset.y = (int) (pFilterSize->y / 2);

    return (TRUE);
}

double recMedianFilter(double x, double y, int k, PROCESS_INFO *info)
{
    IVECTOR d, N, M, *pFilterSize;
    int     i, j, isave; /* Temporary variables */
    double  tmp;         /* for bubble sort     */
    int     npixels;     /* Number of pixels to take into */
                         /* account (border effects)      */

    assert((info != NULL) && (info->param != NULL));
    pFilterSize = (IVECTOR *)(info->param);
    npixels = 0;

    /* This could be improved by merging the table */
    /* filling and the sort                        */
    for(d.x = 0; d.x < pFilterSize->x; d.x++)
    {
        for(d.y = 0; d.y < pFilterSize->y; d.y++)
        {
            N.x = (int)x + d.x - offset.x;
            N.y = (int)y + d.y - offset.y;
            
            M.x = N.x + info->offset.x;
            M.y = N.y + info->offset.y;

            /* Check that the pixel is inside the picutre */
            if (M.x >= 0 && M.x < info->sI.size.x &&
                M.y >= 0 && M.y < info->sI.size.y)
                table[npixels++] = _GetPixelValue(N.x, N.y, k, info);
        }
    }
                
    /* Bubble sort the temporary table */
    for(i = 0; i < npixels - 1; i++)
    {
        isave = i;
        for(j = i + 1; j < npixels; j++)
        {
            if(table[i] > table[j])
                isave = j;
        }
        if(i != isave)
        {
            tmp = table[i];
            table[i] = table[isave];
            table[isave] = tmp;
        }
    }
    
    return (table[npixels / 2]);
}

BOOL recMedianFilterCleanup(PROCESS_INFO *info)
{
    free(table);
    
    return (TRUE);
    UNREFERENCED_PARAMETER(info)
}


/*----------------------------------------------------------------------------
 * Convolution filter
 */
BOOL recConvolutionFilterSetup(PROCESS_INFO *info)
{
    FILTER *f; /* The filter matrix */
    IVECTOR d; /* Index in filter   */

    assert((info != NULL) && (info->param != NULL));
    f = (FILTER *)(info->param);

	sprintf(info->pszTestName, "%s %dx%d", f->name, f->size, f->size);
#ifdef WIN32
	sprintf(info->pszFileName, "_% .200s_%d_%d", f->name, f->size, f->size);
#else
    sprintf(info->pszFileName, "_%.200s_%d_%d", f->name, f->size, f->size);
#endif /* WIN32 */

    sum = 0;

    /* Sum of the coefficients of the filter */
    /* kernel f for normalisation later      */
    offset.x = offset.y = f->size / 2;

    for (d.x = 0; d.x < f->size; d.x++)
    {
        for (d.y = 0; d.y < f->size; d.y++)
        {
            sum += f->coef[d.x][d.y];
        }
    }

    return (TRUE);
}

double RandN(FILE *, int);
double recConvolutionFilter(double x, double y, int k, PROCESS_INFO *info)
{
    FILTER  *f;
    IVECTOR N, d;
    double  v;

    assert((info != NULL) && (info->param != NULL));
    f = (FILTER *)info->param;

    /* Convolution of the image with the filter */
    v = 0;
    for (d.x = 0; d.x < f->size; d.x++)
    {
        for (d.y = 0; d.y < f->size; d.y++)
        {
            N.x = (int)x + d.x - offset.x;
            N.y = (int)y + d.y - offset.y;

            v += f->coef[d.x][d.y] * _GetPixelValue(N.x, N.y, k, info);
        }
    }
    if (sum)
        v /= sum;
            
    return (v + RandN(NULL, 0));
}


/*----------------------------------------------------------------------------
 * Nyquist interpolation
 */
BOOL recNyquistInterpSetup(PROCESS_INFO *info)
{
    int i, j;

    /* sin(PI*x)/(PI*x) interpolation code*/
    SincTable = (double *)MemoryAlloc(sizeof(double) *
                     (cutoff * SINC_RESOLUTION + 1));
    
    SincTable[0] = 1.0;
    
    for (j = 1; j <= cutoff * SINC_RESOLUTION; j++)
        SincTable[j] = 0.0;
  
    /*
     * Add sinc values outside the cutoff distance to the values
     * inside the cutoff distance to preserve overall energy. For
     * numerical stability, start with the small outside values.
     */
    for (i = SINC_RESOLUTION * cutoff * 100; i > 0;)
    {
        for (j = 0; j < cutoff * SINC_RESOLUTION; j++, i--)
            SincTable[j] += sin(PI * i / (double) SINC_RESOLUTION) / 
                             (PI * i / (double) SINC_RESOLUTION);
        
        for (j = cutoff * SINC_RESOLUTION; j > 0; j--, i--)
            SincTable[j] += sin(PI * i / (double) SINC_RESOLUTION) /
                         (PI * i / (double) SINC_RESOLUTION);
    }
    return (TRUE);
    UNREFERENCED_PARAMETER(info)
}


static double _sinc(double x)
{
    int b;
    
    x = fabs(x) * SINC_RESOLUTION;
    b = (int)x;
    if (b >= cutoff * SINC_RESOLUTION)
        return (0.0);

    /* linear interpolation */
    return (1 - (x - b)) * SincTable[b] + (x - b) * SincTable[b+1];
}    


double recNyquistInterp(double x, double y, int k, PROCESS_INFO *info)
{
    IVECTOR p;            /* neighbor pix for conv */
    IVECTOR pul;        /* corner pix in patch */
    IVECTOR winsize;
    NYQUIST *n;
    double  w, v;
    int     l, m;

    assert((info != NULL) && (info->resampling != NULL));
    n = (NYQUIST *)info->resampling;

    /* Coordinates of the corners of the patch in which the new sample */
    /* point (x,y) is located are (pul.x, pul.y), (pul.x+1, pul.y),    */
    /* (pul.x, pul.y+1), (pul.x+1, pul.y+1)                            */
    pul.x = (int) x;
    pul.y = (int) y;
    
    if (pul.x < 0) pul.x = 0;
    if (pul.x > info->sI.size.x - 2) pul.x = info->sI.size.x - 2;
    if (pul.y < 0) pul.y = 0;
    if (pul.y > info->sI.size.y - 2) pul.y = info->sI.size.y - 2;

    /* convolute with 2D sinc to get a sample value v */
    winsize.x = (int) (n->cutoff / n->rolloff.x + 0.5);
    winsize.y = (int) (n->cutoff / n->rolloff.y + 0.5);
    v = 0;
    for (l = -winsize.x+1; l <= winsize.x; l++)
    {
        p.x = pul.x + l;
        for (m = -winsize.y+1; m <= winsize.y; m++)
        {
            p.y = pul.y + m;
            _MirrorBorders(&p, info->sI.size);

            if (n->highpass.x)
                w = _sinc(pul.x + l - x) - 
                    _sinc((pul.x + l - x) * n->rolloff.x) * n->rolloff.x;
            else
                w = _sinc((pul.x + l - x) * n->rolloff.x) * n->rolloff.x;
            
            if (n->highpass.y)
                w *= _sinc(pul.y + m - y) -
                     _sinc((pul.y + m - y) * n->rolloff.y) * n->rolloff.y;
            else
                w *= _sinc((pul.y + m - y) * n->rolloff.y) * n->rolloff.y;
            
            v += info->sI.img[k + info->sI.depth * (p.x + info->sI.size.x * 
                                                                    p.y)] * w;
        }
    }

    return (v);
}

BOOL recNyquistInterpCleanup(PROCESS_INFO *info)
{
    free(SincTable);

    return (TRUE);
    UNREFERENCED_PARAMETER(info)
}

