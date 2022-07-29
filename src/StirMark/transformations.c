/*----------------------------------------------------------------------------
 * StirMark -- Transformation routines
 *
 * Include geometric tranformations.
 * Does not include convolution filters, median filter and LR attack.
 *
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
 * $Header: /StirMark/transformations.c 15    7/04/99 11:28 Fapp2 $
 *----------------------------------------------------------------------------
 */


#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "stirmark.h"
#include "error.h"
#include "image.h"
#include "transformations.h"


/* Default functions */
static BOOL traDefaultSetup(PROCESS_INFO *);
static BOOL nullFunction(PROCESS_INFO *info)
            {return (TRUE); UNREFERENCED_PARAMETER(info)}

/* Invariant */
static double traInvariant(double, double, double (*)(), int, PROCESS_INFO *);

/* Rotatations */
static double rotateAngle, sinAngle, cosAngle;
static double rotateXsOffset, rotateYsOffset;
static double rotateXdOffset, rotateYdOffset;
static BOOL   traRotationSetup(PROCESS_INFO *);
static BOOL   traRotationInitialise(PROCESS_INFO *);
static double traRotation(double, double, double (*)(), int, PROCESS_INFO *);

/* Shearing */
static DVECTOR slope;
static BOOL   traShearSetup(PROCESS_INFO *);
static double traShear(double, double, double (*)(), int, PROCESS_INFO *);

/* General linear transformation */
DVECTOR linearDOffset;
static BOOL   traLinearSetup(PROCESS_INFO *);
static double traLinear(double, double, double (*)(), int, PROCESS_INFO *);

/* Scaling & rotation-scale */
static double colFac, rowFac; /* Scaling factor for row and column */
static BOOL   traScaleSetup(PROCESS_INFO *);
static double traScale(double, double, double (*)(), int, PROCESS_INFO *);
static BOOL   traRotateScaleSetup(PROCESS_INFO *);
static double traRotateScale(double, double, double (*)(), int, PROCESS_INFO *);

/* Geometrical distortion */
static DISTORTION d; /* Distortion parameters */
static FILE      *paramFile;
static BOOL   traDistortionSetup(PROCESS_INFO *);
static double traDistortion(double, double, double (*)(), int, PROCESS_INFO *);
static BOOL   traDistortionCleanUp(PROCESS_INFO *);

/* Horizontal flip */
static BOOL   traFlipSetup(PROCESS_INFO *);
static double traFlip(double, double, double (*)(), int, PROCESS_INFO *);

/* Cropping */
static BOOL   traCroppingSetup(PROCESS_INFO *);
static double traCropping(double, double, double (*)(), int, PROCESS_INFO *);

/* Remover */
static int    *dR, *dC; /* Displacement */
static BOOL   traRemoverSetup(PROCESS_INFO *);
static double traRemover(double, double, double (*)(), int, PROCESS_INFO *);
static BOOL   traRemoverCleanUp(PROCESS_INFO *);


/* List of possible transformers together with their associated functions */
/* {Name, Description, 'T', Define, Setup, Initialise, Function, Cleanup */
PROCESS transformers[] = {
    {"Scale by value", "Scale by value",
     TRA_SCALE_BY_VALUE, 'T', nullFunction, traScaleSetup,
     nullFunction, traScale, nullFunction},
    
    {"Rotate", "Rotate",
     TRA_ROTATION, 'T', nullFunction, traRotationSetup,
     traRotationInitialise, traRotation, nullFunction},
    
    {"StirMark", "Geometrical distortion",
     TRA_DISTORTION, 'T', nullFunction, traDistortionSetup,
     nullFunction, traDistortion, traDistortionCleanUp},
    
    {"Rotate scale", "Rotate, crop and rescale",
     TRA_ROTATION_SCALE, 'T', nullFunction, traRotateScaleSetup,
     traRotationInitialise, traRotateScale, nullFunction},
    
    {"Shear", "Shearing in the X/Y direction",
     TRA_SHEAR, 'T', nullFunction, traShearSetup,
     nullFunction, traShear, nullFunction},
    
    {"Flip", "Horizontal flip",
     TRA_FLIP, 'T', nullFunction, traFlipSetup,
     nullFunction, traFlip, nullFunction},
    
    {"Crop", "Crop using some crop factor",
     TRA_CROPPING, 'T', nullFunction, traCroppingSetup,
     nullFunction, traCropping, nullFunction},
    
    {"Invariant", "No geometrical transformation",
     TRA_INVARIANT, 'T', nullFunction, traDefaultSetup,
     nullFunction, traInvariant, nullFunction},
    
    {"Remover", "Row/column removal",
     TRA_ROW_COL_REMOVAL, 'T', nullFunction, traRemoverSetup,
     nullFunction, traRemover, traRemoverCleanUp},
    
    {"Linear", "General linear transform [[a b][c d]]",
     TRA_LINEAR, 'T', nullFunction, traLinearSetup,
     nullFunction, traLinear, nullFunction},
    
    {"", "", TRA_END_OF_LIST, 0, NULL, NULL, NULL, NULL, NULL}
};


/*---------------------------------------------------------------------------
 * Random number generator: returns a random number in the range [0,1)
 * If stream is NULL simply returns a random number.
 * If load is 1 then read a random number from stream.
 * If load is 0 then write random number to stream.
 */
double RandN(FILE *stream, int load)
{
    double dRandom;

    dRandom = (double)rand() / RAND_MAX;
    
    if (stream != NULL)
    {
        if (load == 1)
            fread(&dRandom, sizeof(dRandom), 1, stream);
        else
            fwrite(&dRandom, sizeof(dRandom), 1, stream);
    }

    return (dRandom);   /* there are certainly better ones ... */
}


/*----------------------------------------------------------------------------
 * Default Transformer Setup
 * For transformers without their own setup this sets some default values
 * for the sampler Setup function to use
 */
BOOL traDefaultSetup(PROCESS_INFO *info) 
{
    info->dPrefSize.x = 0;
    info->dPrefSize.y = 0;
    info->offset.x    = 0;
    info->offset.y    = 0;

    ImageNew(&(info->dI), info->sI.size.x, info->sI.size.y,
        info->sI.depth, info->sI.max);

    return (TRUE);
}

double traInvariant(double x, 
                    double y, 
                    double (*reconstructer)(), 
                    int k, 
                    PROCESS_INFO *info)
{
    /* Just an invariant */
    return ((*reconstructer)(x, y, k, info));
}


/*----------------------------------------------------------------------------
 * Cropping stuff
 * This is not really a transformation
 */
BOOL traCroppingSetup(PROCESS_INFO *info)
{
    int pCropPercent;

    pCropPercent = (int)*((int *)(info->param));

    sprintf(info->pszTestName, "Cropping %d%%%%", pCropPercent);
    sprintf(info->pszFileName, "_cropping_%d", pCropPercent);

    info->dPrefSize.x = info->sI.size.x * (100 - pCropPercent) / 100;
    info->dPrefSize.y = info->sI.size.y * (100 - pCropPercent) / 100;
    info->offset.x    = (info->sI.size.x - info->dPrefSize.x) / 2;
    info->offset.y    = (info->sI.size.y - info->dPrefSize.y) / 2;

    ImageNew(&(info->dI), info->dPrefSize.x, info->dPrefSize.y, 
        info->sI.depth, info->sI.max);
    
    return (TRUE);
}

double traCropping(double x, 
                   double y, 
                   double (*reconstructer)(), 
                   int k, 
                   PROCESS_INFO *info)
{
    return (traInvariant(x, y, reconstructer, k, info));
}


/*----------------------------------------------------------------------------
 * Line & column removal
 */
#define CHECK_RANGE(s, a, b) (assert( ( (a) <= (s) ) && ( (s) <= (b) ) ) )
BOOL traRemoverSetup(PROCESS_INFO *info)
{
    IVECTOR nRemove;
    int     i, d = 0;

    nRemove.x = (int)*((int *)(info->param));     /* # of columns to remove */
    nRemove.y = (int)*((int *)(info->param) + 1); /* # of rows to remove    */

    sprintf(info->pszTestName, "Remove %d row(s) and %d column(s) "
            "at regular intervals", nRemove.x, nRemove.y);
    sprintf(info->pszFileName, "_%d_row_%d_col_removed",
        nRemove.x, nRemove.y);

    if (nRemove.x > info->sI.size.x / 2)
    {
        nRemove.x = info->sI.size.x / 2;
        WARNING("RowColRemover: x parameters too large");
    }
    if (nRemove.y > info->sI.size.y / 2)
    {
        nRemove.y = info->sI.size.y / 2;
        WARNING("RowColRemover: y parameters too large");
    }

    info->dPrefSize.x = info->sI.size.x - nRemove.x;
    info->dPrefSize.y = info->sI.size.y - nRemove.y;
    info->offset.x    = 0;
    info->offset.y    = 0;

    dR = (int *) malloc((size_t)info->dPrefSize.y * sizeof(int));
    dC = (int *) malloc((size_t)info->dPrefSize.x * sizeof(int));

    /* Build a table containing the new row indices */
    for(i = d = 0; i < info->sI.size.y; i++)
    {
        if ((i != 0) && (i != info->dPrefSize.y - 1) 
            && ((d + 1) * info->sI.size.y <= (nRemove.y + 1) * i))
            d++;
        CHECK_RANGE(i - d, 0, info->sI.size.y - 1);
        dR[i - d] = d;
    }
    assert(d == nRemove.y);

    /* Build a table containing the new column indices */
    for(i = d = 0; i < info->sI.size.x; i++)
    {
        if ((i != 0) && (i != info->dPrefSize.x - 1) 
            && ((d + 1) * info->sI.size.x <= (nRemove.x + 1) * i))
            d++;
        CHECK_RANGE(i - d, 0, info->sI.size.x - 1);
        dC[i - d] = d;
    }
    assert(d == nRemove.x);

    ImageNew(&(info->dI), info->dPrefSize.x, info->dPrefSize.y, 
        info->sI.depth, info->sI.max);
    
    return (TRUE);
}

double traRemover(double x, 
                  double y, 
                  double (*reconstructer)(), 
                  int k, 
                  PROCESS_INFO *info)
{
    return ((*reconstructer)(x + dC[(int)x], y + dR[(int)y], k, info));
}

BOOL traRemoverCleanUp(PROCESS_INFO *info)
{
    free(dC);
    free(dR);

    return (TRUE);

    UNREFERENCED_PARAMETER(info)
}


/*----------------------------------------------------------------------------
 * General linear transformation:
 *  M'=  [a  b].M = [[a b][c d]] M
 *       [c  d]
 */
BOOL traLinearSetup(PROCESS_INFO *info)
{
    double *mat, mini, maxi;
    IVECTOR new_size;

    mat = (double *)(info->param);

	sprintf(info->pszTestName, "General linear transformation "
            "[[% .3f % .3f][% .3f % .3f]]", mat[0], mat[1], mat[2], mat[3]);
	sprintf(info->pszFileName, "_linear_%.3f_%.3f_%.3f_%.3f",
            mat[0], mat[1], mat[2], mat[3]);

    info->dPrefSize.x = 0;
    info->dPrefSize.y = 0;
    info->offset.x    = 0; /* x-offset in original image */
    info->offset.y    = 0; /* x-offset in original image */
    
    /* Find extreme points and deduce size of output */
    /* image and offset in output image              */
    mini = MIN(mat[0] * info->sI.size.x,
           MIN(mat[1] * info->sI.size.y,
           MIN(mat[0] * info->sI.size.x + mat[1] * info->sI.size.y, 0)));
    maxi = MAX(mat[0] * info->sI.size.x,
           MAX(mat[1] * info->sI.size.y,
           MAX(mat[0] * info->sI.size.x + mat[1] * info->sI.size.y, 0)));
    assert(maxi - mini);
    new_size.x = (int)ceil(maxi - mini);
    linearDOffset.x = mini;

    mini = MIN(mat[2] * info->sI.size.x,
           MIN(mat[3] * info->sI.size.y,
           MIN(mat[2] * info->sI.size.x + mat[3] * info->sI.size.y, 0)));
    maxi = MAX(mat[2] * info->sI.size.x,
           MAX(mat[3] * info->sI.size.y,
           MAX(mat[2] * info->sI.size.x + mat[3] * info->sI.size.y, 0)));
    assert(maxi - mini);
    new_size.y = (int)ceil(maxi - mini);
    linearDOffset.y = mini;
    
    ImageNew(&(info->dI), new_size.x, new_size.y, info->sI.depth, info->sI.max);

    return (TRUE);
}

double traLinear(double x, 
                 double y,
                 double (*reconstructer)(),
                 int k,
                 PROCESS_INFO *info)
{
    DVECTOR newPoint, tmpPoint;
    double  d, *mat;
    
    mat = (double *)(info->param);

    d = mat[0] * mat[3] - mat[1] * mat[2];
    if (d == 0.0)
        ERROR("traLinear - Tranformation non invertible");

    /* x, y are the coordinates of the current pixel in the */
    /* output image. The y axis is downward.                */

    /* Apply offset and reverse y axis */
    tmpPoint.x = linearDOffset.x + x;
    tmpPoint.y = linearDOffset.y + (info->dI.size.y - y);

    /* Compute the coordinates of the point */
    /* in the original image.               */
    newPoint.x = (  mat[3] * tmpPoint.x - mat[1] * tmpPoint.y) / d;
    newPoint.y = info->sI.size.y - (- mat[2] * tmpPoint.x 
                                    + mat[0] * tmpPoint.y) / d;
    return ((*reconstructer)(newPoint.x, newPoint.y, k, info));
}


/*----------------------------------------------------------------------------
 * Scale by value tansformation
 */
/*TODO: Use traLinear for scaling */
BOOL traScaleSetup(PROCESS_INFO *info)
{
    double colScale, rowScale;
 
	colScale = (double)*((double *)(info->param));
	rowScale = (double)*((double *)(info->param) + 1);

	if (colScale == rowScale)
	{
		sprintf(info->pszTestName, "Scaling %.2f", colScale);
		sprintf(info->pszFileName, "_scale_%.2f", colScale);
	}
	else
	{
		sprintf(info->pszTestName, "Change aspect ratio - "
                "scale.x % .2f scale.y % .2f", rowScale, colScale);
		sprintf(info->pszFileName, "_ratio_x_%.2f_y_%.2f", rowScale, colScale);
	}

    if(colScale <= 0 || rowScale <= 0)
        ERROR("scaling factors must be positive (and non-zero).");

    colFac = 1 / colScale;
    rowFac = 1 / rowScale;

    info->offset.x    = 0;
    info->offset.y    = 0;
    info->dPrefSize.x = (int) (info->sI.size.x * colScale + ONE_MINUS_EPSILON);
    info->dPrefSize.y = (int) (info->sI.size.y * rowScale + ONE_MINUS_EPSILON);

    ImageNew(&(info->dI), info->dPrefSize.x, info->dPrefSize.y,
            info->sI.depth, info->sI.max);

    return (TRUE);
}

double traScale(double x,
                double y,
                double (*reconstructer)(),
                int k,
                PROCESS_INFO *info)
{
    double Xscaled, Yscaled;

    Xscaled = x * colFac;
    Yscaled = y * rowFac;

    return ((*reconstructer)(Xscaled, Yscaled, k, info));
}


/*----------------------------------------------------------------------------
 * Rotation + auto-crop
 */
/*TODO: Use traLinear for rotation */
BOOL traRotationSetup(PROCESS_INFO *info)
{
    double scale, rotateAngleDeg;
    int    a, b;

    /* Counter clock wise is + */
    rotateAngleDeg = (double)*(double *)(info->param);
    rotateAngle    = rotateAngleDeg / 180.0 * PI;
    sinAngle       = sin(rotateAngle);
    cosAngle       = cos(rotateAngle);

    sprintf(info->pszTestName, "Rotation % .2f with cropping", rotateAngleDeg);
    sprintf(info->pszFileName, "_rotation_%.2f", rotateAngleDeg);
 
    a = MAX(info->sI.size.x, info->sI.size.y);
    b = MIN(info->sI.size.x, info->sI.size.y);
    scale = MIN(ABS(b / (a * sin(ABS(rotateAngle)) + 
                         b * cos(ABS(rotateAngle)))),
                ABS(b / (a * sin(ABS(rotateAngle)) - 
                         b * cos(ABS(rotateAngle)))));

    info->offset.x    = 0;
    info->offset.y    = 0;

    /* Currently hard-coded cropping */
    /*info->dPrefSize.x = (int) (info->sI.size.y * ABS(cosAngle)
                             + info->sI.size.y * ABS(sinAngle));
    info->dPrefSize.y = (int) (info->sI.size.x * ABS(sinAngle)
                             + info->sI.size.y * ABS(cosAngle));*/
    info->dPrefSize.x = (int) (info->sI.size.x * scale + ONE_MINUS_EPSILON);
    info->dPrefSize.y = (int) (info->sI.size.y * scale + ONE_MINUS_EPSILON);
    ImageNew(&(info->dI), info->dPrefSize.x, info->dPrefSize.y,
        info->sI.depth, info->sI.max);
    return (TRUE);
}

BOOL traRotationInitialise(PROCESS_INFO *info)
{
    rotateXsOffset = info->sI.size.x / 2.0;
    rotateYsOffset = info->sI.size.y / 2.0;
    rotateXdOffset = info->dI.size.x / 2.0;
    rotateYdOffset = info->dI.size.y / 2.0;
    
    return (TRUE);
}

double traRotation(double x,
                   double y,
                   double (*reconstructer)(),
                   int k,
                   PROCESS_INFO *info)
{
    double xPrime, yPrime;
    double newX, newY;

    xPrime = (x - rotateXdOffset);
    yPrime = (y - rotateYdOffset);

    newX = xPrime *   cosAngle  + yPrime * sinAngle + rotateXsOffset;
    newY = xPrime * (-sinAngle) + yPrime * cosAngle + rotateYsOffset;

    return ((*reconstructer)(newX, newY, k, info));
}


/*----------------------------------------------------------------------------
 * Rotation + auto-crop + auto-scale
 */
/*TODO: Use traLinear for rotation */
BOOL traRotateScaleSetup(PROCESS_INFO *info)
{
    double scale, rotateAngleDeg;
    int a, b;

    /* Counter clock wise is + */
    rotateAngleDeg = (double)*(double *)(info->param);
    rotateAngle    = rotateAngleDeg / 180.0 * PI;
    sinAngle       = sin(rotateAngle);
    cosAngle       = cos(rotateAngle);

    sprintf(info->pszTestName, "Rotation % .2f with cropping and scaling",
            rotateAngleDeg);
    sprintf(info->pszFileName, "_rotation_scale_%.2f", rotateAngleDeg);

    a = MAX(info->sI.size.x, info->sI.size.y);
    b = MIN(info->sI.size.x, info->sI.size.y);
    scale = MIN(ABS(b / (a * sin(ABS(rotateAngle)) +
                         b * cos(ABS(rotateAngle)))),
                ABS(b / (a * sin(ABS(rotateAngle)) -
                         b * cos(ABS(rotateAngle)))));

    info->offset.x    = 0;
    info->offset.y    = 0;
    info->dPrefSize.x = info->sI.size.x;
    info->dPrefSize.y = info->sI.size.y;

    colFac = rowFac = scale;

    ImageNew(&(info->dI), info->dPrefSize.x, info->dPrefSize.y, 
             info->sI.depth, info->sI.max);

    return (TRUE);
}

double traRotateScale(double x,
                      double y,
                      double (*reconstructer)(),
                      int k,
                      PROCESS_INFO *info)
{
    double xPrime, yPrime;
    double newX, newY;

    /* Scaling first */
    xPrime = (x - rotateXdOffset) * colFac;
    yPrime = (y - rotateYdOffset) * rowFac;

    /* Now the rotation */
    newX = xPrime *   cosAngle  + yPrime * sinAngle + rotateXsOffset;
    newY = xPrime * (-sinAngle) + yPrime * cosAngle + rotateYsOffset;
    
    return ((*reconstructer)(newX, newY, k, info));
}


/*----------------------------------------------------------------------------
 * Shearing in X and/or Y direction
 */
/*TODO: Use traLinear for shearing */
BOOL traShearSetup(PROCESS_INFO *info)
{
    DVECTOR shift, percentshift;

    percentshift.x = (double)*(double *)(info->param);
    percentshift.y = (double)*((double *)(info->param) + 1);

	sprintf(info->pszTestName, "Shearing - x-direction % .2f%%%%"
        "y-direction % .2f%%%%", percentshift.x, percentshift.y);
	sprintf(info->pszFileName, "_shearing_x_%.2f_y_%.2f",
        percentshift.x, percentshift.y);

    shift.x = -(percentshift.x) * (double) info->sI.size.x / 100.0;
    shift.y = -(percentshift.y) * (double) info->sI.size.y / 100.0;

    slope.x = (double)shift.x / (double)info->sI.size.y;
    slope.y = (double)shift.y / (double)info->sI.size.x;

    info->offset.x    = 0;
    info->offset.y    = 0;
    info->dPrefSize.x = (int) (info->sI.size.x - ABS(shift.x));
    info->dPrefSize.y = (int) (info->sI.size.y - ABS(shift.y));
    ImageNew(&(info->dI), info->dPrefSize.x, info->dPrefSize.y,
        info->sI.depth, info->sI.max);
    return (TRUE);
}

double traShear(double x,
                double y,
                double (*reconstructer)(),
                int k,
                PROCESS_INFO *info)
{
    DVECTOR newPoint;
    
    if (slope.x >= 0)
        newPoint.x = x - slope.x * (y - info->sI.size.y);
    else
        newPoint.x = x - slope.x * y;

    if (slope.y >= 0)
        newPoint.y = y - slope.y * (info->sI.size.x - x);
    else
        newPoint.y = y - slope.y * x;

    return ((*reconstructer)(newPoint.x, newPoint.y, k, info));
}



/*----------------------------------------------------------------------------
 * Horizontal flip
 */
/*TODO: Use traLinear for flip */
BOOL traFlipSetup(PROCESS_INFO *info)
{
    strcpy(info->pszTestName, "Flip");
	strcpy(info->pszFileName, "_flip");

    return traDefaultSetup(info);
}

double traFlip(double x,
               double y,
               double (*reconstructer)(),
               int k,
               PROCESS_INFO *info)
{
    return ((*reconstructer)(info->dI.size.x - 1 - x, y, k, info));
}

/*----------------------------------------------------------------------------
 * Geometrical distortion
 */
BOOL traDistortionSetup(PROCESS_INFO *info)
{
    int     i, k;
    OPTIONS otmp, *o;
    IMAGE   imgtmp;

    o = (OPTIONS *)(info->param);
    paramFile = NULL;

    strcpy(info->pszTestName, "StirMark with randomisation and bending");
    strcpy(info->pszFileName, "_stirmark_random_bend");

    /* Use previously saved parameters */
    if (o->parameters == DIST_OP_LOAD || 
        o->parameters == DIST_OP_LOAD_ALL)
    {
        /* Load existing parameters from file */
        if ((paramFile = fopen(o->pszFileName, "rb")) == NULL)
        {
            WARNING("traDistortionSetup: could open parameters file."
                "Parameters will not be loaded.");
            o->parameters = DIST_OP_NEW;
        }
        else
        {
            /* Info about source image */
            fread(&(imgtmp), sizeof(imgtmp), 1, paramFile);
            if (imgtmp.size.x != info->sI.size.x || 
                imgtmp.size.y != info->sI.size.y || 
                imgtmp.depth != info->sI.depth)
            {
                WARNING("traDistortionSetup: you cannot use the parameters"
                    "file with this image (different size or depth).");
                o->parameters = DIST_OP_NEW;
            }
            else
            {
                /* Load remaining options */

                /* The options */
                fread(&otmp, sizeof(otmp), 1, paramFile);
                if (otmp.parameters == DIST_OP_SAVE && 
                    o->parameters == DIST_OP_LOAD_ALL)
                {
                    WARNING("Not all parameters were saved in the parameter"
                            "file. Only the main parameters will be used.");
                    o->parameters = DIST_OP_LOAD;
                }
                o->bending_factor    = otmp.bending_factor;
                o->deviation         = otmp.deviation;
                o->inwards           = otmp.inwards;
                o->jpeg_quality      = otmp.jpeg_quality;
                o->outwards          = otmp.outwards;
                o->random_factor     = otmp.random_factor;
                o->relative_inwards  = otmp.relative_inwards;
                o->relative_outwards = otmp.relative_outwards;

                /* The distortions */
                fread(&d, sizeof(d), 1, paramFile);
                d.parameters = o->parameters;
            }
        }
    }

    /* Generate new parameters and/or save them */
    if (o->parameters == DIST_OP_SAVE     || 
        o->parameters == DIST_OP_SAVE_ALL ||
        o->parameters == DIST_OP_NEW)
    {
        /* Generate new distortion parameters */
        if (o->relative_inwards)
            o->inwards = (info->sI.size.x < info->sI.size.y ? info->sI.size.x
                                                            : info->sI.size.y) 
            * o->inwards / 100.0;
        if (o->relative_outwards)
            o->outwards = (info->sI.size.y < info->sI.size.y ? info->sI.size.x
                                                             : info->sI.size.y) 
            * o->outwards / 100.0;

        /* better than o.bending_factor *= RandN() + o.outwards; */
        o->bending_factor *=  (info->sI.size.y + info->sI.size.y) / 400.0;
        
        d.ul.x =  RandN(NULL, 0) * (o->inwards + o->outwards) - o->outwards;
        d.ul.y = -RandN(NULL, 0) * (o->inwards + o->outwards) + o->outwards;
        d.ll.x =  RandN(NULL, 0) * (o->inwards + o->outwards) - o->outwards;
        d.ll.y =  RandN(NULL, 0) * (o->inwards + o->outwards) - o->outwards;
        d.ur.x = -RandN(NULL, 0) * (o->inwards + o->outwards) + o->outwards;
        d.ur.y = -RandN(NULL, 0) * (o->inwards + o->outwards) + o->outwards;
        d.lr.x = -RandN(NULL, 0) * (o->inwards + o->outwards) + o->outwards;
        d.lr.y =  RandN(NULL, 0) * (o->inwards + o->outwards) - o->outwards;

        /* use new base: usual base for picture where upper left corner */
        /* is the origin and the y-axis goes down                       */
        d.ul.y = -d.ul.y;
        d.ll.y = -d.ll.y;
        d.ur.y = -d.ur.y;
        d.lr.y = -d.lr.y;

        d.bending_factor = o->bending_factor;
        d.random_factor  = o->random_factor;    
        d.deviation      = o->deviation;
        d.parameters     = o->parameters;

            /* set transfer function error offsets */
        for (i = 0; i <= 256/ERRSTEP; i++)
            for (k = 0; k < info->sI.depth; k++)
                d.err[i][k] = RandN(NULL, 0) * 2 * d.deviation - d.deviation;
        
        if (o->parameters == DIST_OP_SAVE ||
            o->parameters == DIST_OP_SAVE_ALL)
        {
            if ((paramFile = fopen(o->pszFileName, "wb")) == NULL)
            {
                WARNING("traDistortionSetup: could not create parameters file."
                        "Parameters will not be saved.");
                o->parameters = DIST_OP_NEW;
            }
            else
            {
                /* Info about source image */
                fwrite(&(info->sI), sizeof(info->sI), 1, paramFile);

                /* The options */
                fwrite(o, sizeof(*o), 1, paramFile);

                /* The distortions */
                fwrite(&d, sizeof(d), 1, paramFile);
            }
        }
    }

    info->offset.x = 0;
    info->offset.y = 0;
    info->dPrefSize.x = 0;
    info->dPrefSize.y = 0;

    ImageNew(&(info->dI), info->sI.size.x, info->sI.size.y, 
             info->sI.depth, info->sI.max);

    /* Clean up if don't load or save everything */
    if (   (o->parameters == DIST_OP_SAVE || 
            o->parameters == DIST_OP_LOAD)
        && (paramFile != NULL))
    {
        fclose(paramFile);
        paramFile = NULL;
    }

    return (TRUE);
}


/*
 * Distortion function:
 *   M = b.(a.A + (1 - a).D) + (1 - b).(a.B + (1 - a).C)
 * where A is the lower left corner of the image and other letters are given
 * in the clockwise order and where a and b are the coordiantes of M relatively
 * to A, B, C and D.
 */
double traDistortion(double x,
                     double y,
                     double (*reconstructer)(),
                     int k,
                     PROCESS_INFO *info)
{
    DVECTOR A, B, C, D, AB, CB, CD, v, P1, f;
    int        perr;   /* number of error patch */
    double     val;      /* estimated value */
    DVECTOR    M;      /* current pixel */
    double  displacement;

    M.x = x;
    M.y = y;

    /* Coordinates of the corners of the image after      */
    /* distortion expressed in the old base (down Y axis) */
    P(A, d.ll.x, info->sI.size.y + d.ll.y);
    P(B, d.ul.x, d.ul.y);
    P(C, info->sI.size.x + d.ur.x, d.ur.y);
    P(D, info->sI.size.x + d.lr.x, info->sI.size.y + d.lr.y);
    
    V(AB, A, B);
    V(CB, C, B);
    V(CD, C, D);
    ADD(v, AB, CD);
    
    P1.x = 1 - M.x / info->sI.size.x;
    P1.y = M.y / info->sI.size.y;
    
    M.x = - P1.x * P1.y * v.x + P1.x * CB.x + P1.y * CD.x + C.x;
    M.y = - P1.x * P1.y * v.y + P1.x * CB.y + P1.y * CD.y + C.y;

    /* Global bending to the picture */
    if (d.bending_factor != 0)
    {
        M.x += d.bending_factor * sin(M.y * PI / info->sI.size.y);
        M.y += d.bending_factor * sin(M.x * PI / info->sI.size.x);
    }

    /* Randomiser: changes slightly the coordinates ot the pixels */
    if (d.random_factor != 0.0)
    {
        f.x = floor(info->sI.size.x / 20) / info->sI.size.x;
        f.y = floor(info->sI.size.y / 20) / info->sI.size.y;
        displacement   = d.random_factor * (sin(M.x * 2 * PI * f.x) * 
                                            sin(M.y * 2 * PI * f.y));

        assert((paramFile == NULL && (d.parameters == DIST_OP_LOAD || 
                                      d.parameters == DIST_OP_SAVE ||
                                      d.parameters == DIST_OP_NEW))
               ||
               (paramFile != NULL && (d.parameters == DIST_OP_LOAD_ALL ||
                                      d.parameters == DIST_OP_SAVE_ALL)));
        M.x += displacement * (1 + RandN(paramFile, 
                                         d.parameters == DIST_OP_LOAD_ALL));
        M.y += displacement * (1 + RandN(paramFile,
                                         d.parameters == DIST_OP_LOAD_ALL));
    }

    val = (*reconstructer)(M.x, M.y, k, info);
    
    /* interpolation of transfer function offset */
    perr = (int) (val / ERRSTEP);
    if (perr < 0) perr = 0;
    if (perr >= 256/ERRSTEP) perr = 256 / ERRSTEP - 1;
        val += (1 - (val - perr * ERRSTEP) / ERRSTEP) * d.err[perr][k] +
                ((val - perr * ERRSTEP) / ERRSTEP) * d.err[perr+1][k];
                    
    /* dithering to distribute quantization noise */
    val = val + RandN(paramFile, d.parameters == DIST_OP_LOAD_ALL);
                    
    return  (val);
}

BOOL traDistortionCleanUp(PROCESS_INFO *info)
{
    OPTIONS *o;

    o = (OPTIONS *)(info->param);

    if (   (o->parameters == DIST_OP_SAVE_ALL || 
            o->parameters == DIST_OP_LOAD_ALL)
        && (paramFile != NULL))
        fclose(paramFile);

    return (TRUE);

    UNREFERENCED_PARAMETER(info)
}

