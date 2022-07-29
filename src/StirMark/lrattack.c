/*----------------------------------------------------------------------------
 * LRAttack.c - LR attack copnfigured for StirMark integration.
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
 * Rev 1.0 - 27/11/98
 * $Header: /StirMark/lrattack.c 4     18/01/99 17:44 Fapp2 $
 *
 * This file implements the LRAttack function in a form suitable
 * for integration with StirMark.
 *----------------------------------------------------------------------------
 */

#include    "lrattack.h"
#include    <math.h>
#include    <stdlib.h>

/*----------------------------------------------------------------------------
 * LRAttack() - perform histogram equalised LR attack
 *
 * pSimage = signed image pointer.
 * pDimage = destination image pointer.
 * dStrength = strength of overall attack.
 * dJNDStrength = strength of JND based attack.
 * nScale = scale for attack.
 *
 * Returns true if ok, false if error detected.
 *
 * pSimage is a pointer to the image to be attacked. pDimage
 * is where to put the attacked image. This needs to be
 * supplied to the function. pSImage can be the same as pDImage
 * i.e. an in-place attack is possible.
 *
 * dStrength indicates the strength of the attack and should be
 * betweem 0.0 and 1.0. 0.0 indicates no attack while 1.0 indicates
 * a full strength attack.
 *
 * dJNDStrength is similar except that it controls the JND-weighted
 * version of the attack. Setting this to 0.0 turns off this function
 * which will save a considerable amount of processing time.
 *
 * nScale is the scale for the attack. Support values are 1, 2, 4
 * 8, 16, 32. Anything else will result in an error return.
 *
 * The attack works as follows. First of all, the IMAGE is converted into
 * a YUV array - largely because that is how it works in the program
 * from which this attack code was derived. During this process, if the
 * image size was not a multiple of the scale required, the image is
 * padded with grey pixels to the correct size.
 *
 * This is then histogram equalised and processed with a Laplacian
 * 3 x 3 convolution. The result is then histogram equalised again
 * expanded back to the original size. This is then added to the original
 * image at the selected strengths.
 */
int LRAttack(IMAGE *pSImage, IMAGE *pDImage, double dStrength, double dJNDStrength, int nScale)
{

    YUV *s2Image = NULL, *s4Image = NULL, *s8Image = NULL, *s16Image = NULL;
    YUV *nim, *tim;
    int w, h, nw, nh, sw, sh;
    YUV *pNImFullSize, *pDim, *pSim;

    if ((dStrength == 0) && (dJNDStrength == 0))
        return TRUE; /* nothing needs to be done */

#ifdef VERBOSE
    printf("LRAttack at scale %d, strength = %f, JNDstrength = %f\n",
        nScale, dStrength, dJNDStrength);
#endif

    switch (nScale)
    { /* check scale for legality */
        case 1:
        case 2:
        case 4:
        case 8:
        case 16:
        case 32:
            break;

        default:
            return FALSE;
    }

    w = pSImage->size.x;
    h = pSImage->size.y;

    nw = nScale * ((w + nScale - 1) / nScale); /*increase to multiple of scale */
    nh = nScale * ((h + nScale - 1) / nScale);
    

    /* Create intermediate size images as required */

    switch (nScale)
    {
        case 32    :
            s16Image = (YUV *)malloc((nw/16) * (nh/16) * sizeof(YUV));
        case 16    :    
            s8Image = (YUV *)malloc((nw/8) * (nh/8) * sizeof(YUV));
        case 8    :    
            s4Image = (YUV *)malloc((nw/4) * (nh/4) * sizeof(YUV));
        case 4    :
            s2Image = (YUV *)malloc((nw/2) * (nh/2) * sizeof(YUV));
        case 2    :    break;
        case 1  :    break;
    }    

    sw = nw / nScale;
    sh = nh / nScale;

    pNImFullSize = (YUV *)malloc(nw * nh * sizeof(YUV));
    pDim = (YUV *)malloc(nw * nh * sizeof(YUV));
    pSim = (YUV *)malloc(nw * nh * sizeof(YUV));
    LRImageToYUV(pSImage, pSim, nw, nh);        /* copy into YUV form */
    LRCopyImage(pSim, pNImFullSize, nw, nh);
    LRCopyImage(pSim, pDim, nw, nh);
    nim = (YUV *)malloc(sw * sh * sizeof(YUV)); /* temp shrunk images */
    tim = (YUV *)malloc(sw * sh * sizeof(YUV));
    
    
    /* Shrink image to correct scale */

    switch (nScale)
    {
        case 1 :
            LRCopyImage(pNImFullSize, nim, nw, nh);
            break;

        case 2 :
            LRShrinkImage(pNImFullSize, nim, nw, nh);
            break;

        case 4 :
            LRShrinkImage(pNImFullSize, s2Image, nw, nh);
            LRShrinkImage(s2Image, nim, w/2, h/2);
            break;

        case 8 :
            LRShrinkImage(pNImFullSize, s2Image, nw, nh);
            LRShrinkImage(s2Image, s4Image, nw/2, nh/2);
            LRShrinkImage(s4Image, nim, nw/4, nh/4);
            break;

        case 16 :
            LRShrinkImage(pNImFullSize, s2Image, nw, nh);
            LRShrinkImage(s2Image, s4Image, nw/2, nh/2);
            LRShrinkImage(s4Image, s8Image, nw/4, nh/4);
            LRShrinkImage(s8Image, nim, nw/8, nh/8);
        break;

        case 32 :
            LRShrinkImage(pNImFullSize, s2Image, nw, nh);
            LRShrinkImage(s2Image, s4Image, nw/2, nh/2);
            LRShrinkImage(s4Image, s8Image, nw/4, nh/4);
            LRShrinkImage(s8Image, s16Image, nw/8, nh/8);
            LRShrinkImage(s16Image, nim, nw/16, nh/16);
            break;
    }

    
    /* Compute the negative Laplacian */

    LRHisteq(nim, nim, sw, sh);
    LRSharpenLaplace(nim, tim, sw, sh);
    LRCopyImage(tim, nim, sw, sh);
    LRHisteq(nim, nim, sw, sh);

    
    /* Expand back to full size */

    switch (nScale)
    {
        case 32 :    
            LRExpandImage(nim, s16Image, nw/16, nh/16);
            LRExpandImage(s16Image, s8Image, nw/8, nh/8);
            LRExpandImage(s8Image, s4Image, nw/4, nh/4);
            LRExpandImage(s4Image, s2Image, nw/2, nh/2);
            LRExpandImage(s2Image, pNImFullSize, nw, nh);
            free (s16Image);
            free (s8Image);
            free (s4Image);
            free (s2Image);
            break;

        case 16 :    
            LRExpandImage(nim, s8Image, nw/8, nh/8);
            LRExpandImage(s8Image, s4Image, nw/4, nh/4);
            LRExpandImage(s4Image, s2Image, nw/2, nh/2);
            LRExpandImage(s2Image, pNImFullSize, nw, nh);
            free (s8Image);
            free (s4Image);
            free (s2Image);
            break;

        case 8 :    
            LRExpandImage(nim, s4Image, nw/4, nh/4);
            LRExpandImage(s4Image, s2Image, nw/2, nh/2);
            LRExpandImage(s2Image, pNImFullSize, nw, nh);
            free (s4Image);
            free (s2Image);
            break;

        case 4 :    
            LRExpandImage(nim, s2Image, nw/2, nh/2);
            LRExpandImage(s2Image, pNImFullSize, nw, nh);
            free (s2Image);
            break;

        case 2 :
            LRExpandImage(nim, pNImFullSize, nw, nh);
            break;

        case 1 :
            LRCopyImage(nim, pNImFullSize, nw, nh);
            break;
    }
    free (nim);
    free (tim);
    
    
    /* Now add the computed Laplacian to the original image */

    LRAddImage(pNImFullSize, pDim, nw, nh, dStrength, dJNDStrength);
    LREqualiseLuminance(pSim, pDim, w, h);
    LRYUVToImage(pDim, pDImage, nw);
    free (pNImFullSize);
    free (pDim);
    free (pSim);
    return TRUE;
}



/*----------------------------------------------------------------------------
 * LRAddImage() - adds  the laplacian to an image
 *
 * pLap = pointer to Laplacian image.
 * pDim = pointer to the destination image.
 * w, h = height and width of image.
 * dStrengh = strength of overall attack.
 * dJNDStrength = strength of JND weighted attack.
 */
void LRAddImage(YUV *pLap, YUV *pDim, int w, int h, double dStrength, double dJNDStrength)
{
    double    *jnd = NULL, *jp = NULL;
    YUV        *dp, *lp;
    int        row, col;

    if (dJNDStrength > 0)
    {
        jnd = LRCalculateJND(pDim, w, h);
        jp = jnd;
    }
    dp = pDim;
    lp = pLap;
    for (row = 0; row < h; row++)
        for (col = 0; col < w; col++, dp++, jp++, lp++)
        {
            dp->Y = dp->Y + (lp->Y - IMAGE_GREY) * dStrength;
            if (dJNDStrength > 0)
                dp->Y = dp->Y + (lp->Y - IMAGE_GREY) * (*jp) * dJNDStrength;
        }
    if (dJNDStrength > 0)
        free (jnd);
}



/*----------------------------------------------------------------------------
 * LRSharpenLaplace() - performs a 3 x 3 Laplacian convolution
 *
 * sim = pointer to source YUV image.
 * dim = pointer to destination YUV image
 * w = width
 * h = height
 */
void    LRSharpenLaplace(YUV *sim, YUV *dim, int w, int h)
{
    static    int    pMask[] = {1, 1, 1, 1, -8, 1, 1, 1, 1};
    int    row, col, xoff, yoff, pptr;
    double    tpixel, minval, maxval;
    YUV        *dp;

    minval = 1000000;
    maxval = -minval;
    for (row = 0; row < h; row ++)
    {
        for (col = 0; col < w; col++)
        {                                        /* process a source pixel */
            pptr = row * w + col;                /* points to source pixel */
            tpixel = 0;
            for (xoff = -1; xoff <= 1; xoff++)
                for (yoff = -1; yoff <= 1; yoff++)
                {
                    if ((col + xoff >= 0) && (row + yoff >= 0) && 
                            (col + xoff < w) && (row + yoff < h))
                        tpixel += (pMask[(yoff+1) * 3 + xoff+1] * sim[pptr + xoff + yoff * w].Y);
                    else
                        tpixel += (pMask[(yoff+1) * 3 + xoff+1] * IMAGE_GREY);
                }
            if (tpixel < minval)
                minval = tpixel;
            if (tpixel > maxval)
                maxval = tpixel;
            dim[pptr].Y = tpixel;
            dim[pptr].U = 0;
            dim[pptr].V = 0;
        }
    }
    dp = dim; 
    for (row = 0; row < h; row ++)
        for (col = 0; col < w; col++, dp++)
            dp->Y = ((dp->Y - minval)/(maxval - minval)) * IMAGE_WHITE; 
}


/*----------------------------------------------------------------------------
 * LRShrinkImage() - shrink an image by a factor of two.
 *
 * Works by simply averaging components in a 2 x 2 square to give
 * a pixel at the lower resolution.
 *
 * pSim = pointer to original image.
 * pDim = pointer to shrunk image.
 * w, h = dimensions of original image.
 */
void LRShrinkImage(YUV *pSim, YUV *pDim, int w, int h)
{
    int        i, j, sw, sh;
    YUV        *dp, *tp;

    sw = w/2;
    sh = h/2;

    dp = pDim;
    for (i = 0; i < sh; i++)
        for (j = 0; j < sw; j++, dp++)
        { /* compute average */
            tp = pSim + i * 2 * w + j * 2;
            dp->Y = (tp[0].Y + tp[1].Y + tp[w].Y + tp[w + 1].Y) / 4;
            dp->U = (tp[0].U + tp[1].U + tp[w].U + tp[w + 1].U) / 4;
            dp->V = (tp[0].V + tp[1].V + tp[w].V + tp[w + 1].V) / 4;
        }
}


/*----------------------------------------------------------------------------
 * LRExpandImage() - doubles size of image
 *
 * pSim = pointer to original image.
 * pDim = pointer to expanded image
 * w, h = dimensions of expanded image.
 */
void LRExpandImage(YUV *pSim, YUV *pDim, int w, int h)
{
    int        i, j, sw, sh;
    YUV        *sp, *dp, *tim;

    static    int    pMask[] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
    int    row, col, xoff, yoff, pptr;


    sw = w/2;
    sh = h/2;
    sp = pSim;
    tim = (YUV *)malloc(w * h * sizeof(YUV));

    dp = pDim;
    for (i = 0; i < h * w; i++, dp++)
    {
        dp->Y = dp->U = dp->V = 0;
    }
    for (i = 0; i < sh; i++)
        for (j = 0; j < sw; j++)
            pDim[i * 2 * w + j * 2] = pSim[i * sw + j];

    dp = tim;
    for (row = 0; row < h; row ++)
    {
        for (col = 0; col < w; col++, dp++)
        { /* process a source pixel */
            pptr = row * w + col; /* points to source pixel */
            dp->Y = dp->U = dp->V = 0;
            for (xoff = -1; xoff <= 1; xoff++)
                for (yoff = -1; yoff <= 1; yoff++)
                {
                    if ((col + xoff >= 0) && (row + yoff >= 0) && 
                            (col + xoff < w) && (row + yoff < h))
                    {
                        dp->Y += (pMask[(yoff+1) * 3 + xoff+1] * pDim[pptr + xoff + yoff * w].Y);
                        dp->U += (pMask[(yoff+1) * 3 + xoff+1] * pDim[pptr + xoff + yoff * w].U);
                        dp->V += (pMask[(yoff+1) * 3 + xoff+1] * pDim[pptr + xoff + yoff * w].V);
                    }
                }
            dp->Y /= 4;
            dp->U /= 4;
            dp->V /= 4;
        }
    }
    LRCopyImage(tim, pDim, w, h);
    free (tim);
}


/*----------------------------------------------------------------------------
 * LRCopyImage() - copies an image from one array to another
 * 
 * pSim = pointer to source image.
 * pDim = pointer to destination image.
 * w, h = dimensions of images.
 */
void LRCopyImage(YUV *pSim, YUV *pDim, int w, int h)
{
    int        i;
    YUV        *sp, *dp;

    sp = pSim;
    dp = pDim;
    for (i = w * h; i > 0; i--)
        *dp++ = *sp++;
}


/*----------------------------------------------------------------------------
 * LRHisteq() - performs histogram equalisation of image.
 *
 * pSim = pointer to source image.
 * pDim = pointer to destination image.
 * w, h = dimensions of images.
 *
 * This code was taken from the CVIP toolkit - the original
 * header is below:
 *
 *----------------------------------------------------------------------------
 *
 * File Name: histeq.c
 * Description: Contains routines necessary to perform histogram
 *              equalization on an image.
 * Related Files: 
 * Initial Coding Date: 3/1/96
 * Portability: Standard (ANSI) C
 * References:
 * Author(s): Arve Kjoelen
 *            Southern Illinois University @ Edwardsville
 */
void    LRHisteq(YUV *pSim, YUV *pDim, int w, int h)
{
    YUV      *dp, *sp;  /* handy pointers */
    int       num;
    double    mult, *dist;
    int       i, j;

    mult = 1.0 / (w * h);
    dist =  (double *)calloc((IMAGE_WHITE + 1), sizeof(double));
    num = IMAGE_WHITE + 1;


    /* Calculate the Probability Density Function of each block. */

    sp = pSim;
    dp = pDim;
    for(i = 0; i < h; i++) 
        for(j = 0; j < w; j++, sp++, dp++) 
        {
            *dp = *sp;
            if (dp->Y > IMAGE_WHITE)
                dp->Y = IMAGE_WHITE;
            if (dp->Y < 0)
                dp->Y = IMAGE_BLACK;
            dist[(int)(dp->Y)] += 1.0;
        }


    /* Scale and calculate the Cumulative Distribution Function. */

    mult *= (double)(IMAGE_WHITE);
    dist[0] *= mult;
    for(i = 1; i < num; i++)
    {
        dist[i] *= mult;
        dist[i] += dist[i-1];
    }


    /* Equalize. */

    dp = pDim;
    for(i = 0; i < h; i++) 
        for(j = 0; j < w; j++, dp++) 
             dp->Y = dist[(int)(dp->Y)];
    free (dist);
}


/*----------------------------------------------------------------------------
 * LREqualiseLuminance() - equalise the mean luminance of two images
 *
 * pRef = pointer to reference image.
 * pDim = pointer to image to be processed
 * w, h = dimensions of images.
 */
void LREqualiseLuminance(YUV *pRef, YUV *pDim, int w, int h)
{
    double    rmean, dmean, delta;
    int        i;

    for (rmean = 0, i = 0; i < w * h; i++)
        rmean += pRef[i].Y;

    rmean /= w * h;

    for (dmean = 0, i = 0; i < w * h; i++)
        dmean += pDim[i].Y;

    dmean /= w * h;

    delta = rmean - dmean;

    for (i = 0; i < w * h; i++)
    {
        pDim[i].Y += delta;
        if (pDim[i].Y > IMAGE_WHITE)
            pDim[i].Y = IMAGE_WHITE;
        if (pDim[i].Y < IMAGE_BLACK)
            pDim[i].Y = IMAGE_BLACK;
    }
}


/*----------------------------------------------------------------------------
 * LRImageToYUV - convert an IMAGE format image to a YUV array
 *
 * pSImage = pointer to IMAGE.
 * pDim = pointer to YUV array to receive image data
 * nw, nh = dimensions for YUV array (>= original)
 */
void	LRImageToYUV(IMAGE *pSImage, YUV *pDim, int nw, int nh)
{
	int		w, h, row, col;
	unsigned char	*sp;
	YUV		*dp;

	w = pSImage->size.x;
	h = pSImage->size.y;
	sp = pSImage->img;
	dp = pDim;
	if (pSImage->depth == 1)
	{
		for (row = 0; row < nh; row++)
		{
			sp = pSImage->img + row * w;
			for (col = 0; col < nw; col++, sp++, dp++)
			{
				if ((col < w) && (row < h))
					dp->Y = *sp;
				else
					dp->Y = IMAGE_GREY;
				(*dp).U = 0;
				(*dp).V = 0;
			}
		}
	}
	else
	{
		for (row = 0; row < nh; row++)
		{
			sp = pSImage->img + row * w * 3;
			for (col = 0; col < w; col++, sp += 3, dp++)
			{
				if ((col < w) && (row < h))
				{
					dp->Y = 0.299 * (double)*(sp) + 0.587 * (double)*(sp+1) + 0.114 * (double)*(sp+2);
					dp->U = 0.564 * ((double)*(sp+2) - dp->Y);
					dp->V = 0.713 * ((double)*(sp) - dp->Y);
				}
				else
				{
					dp->Y = IMAGE_GREY;
					dp->U = 0;
					dp->V = 0;
				}
			}
		}
	}
}


/*----------------------------------------------------------------------------
 * LRYUVToImage() - Convert from a YUV array to an image
 *
 * pSim = pointer to YUV array.
 * pDImage = pointer to IMAGE to receive the image.
 * nw = width of YUV array.
 */
void LRYUVToImage(YUV *pSim, IMAGE *pDImage, int nw)
{
    int        w, h, row, col, tpixel;
    unsigned char    *dp;
    YUV        *sp;
    double    eb, eg, er;

    w = pDImage->size.x;
    h = pDImage->size.y;
    sp = pSim;
    dp = pDImage->img;
    if (pDImage->depth == 1)
    {
        for (row = 0; row < h; row++)
        {
            sp = pSim + row * nw;
            for (col = 0; col < w; col++, sp++)
            {
                tpixel = LRRound(sp->Y);
                if (tpixel > IMAGE_WHITE)
                    tpixel = IMAGE_WHITE;
                if (tpixel < IMAGE_BLACK)
                    tpixel = IMAGE_BLACK;
                *dp++ = (unsigned char)tpixel;
            }
        }
    }
    else
    {
        for (row = 0; row < h; row++)
        {
            sp = pSim + row * w;
            for (col = 0; col < w; col++, sp++)
            {
                eb = sp->U / 0.564 + sp->Y;
                er = sp->V / 0.713 + sp->Y;
                eg = (sp->Y - 0.114 * eb - 0.299 * er) / 0.587;
                if (eb < IMAGE_BLACK)
                    eb = IMAGE_BLACK;
                if (eb > IMAGE_WHITE)
                    eb = IMAGE_WHITE;
                if (eg < IMAGE_BLACK)
                    eg = IMAGE_BLACK;
                if (eg > IMAGE_WHITE)
                    eg = IMAGE_WHITE;
                if (er < IMAGE_BLACK)
                    er = IMAGE_BLACK;
                if (er > IMAGE_WHITE)
                    er = IMAGE_WHITE;
                *dp++ = (unsigned char)LRRound(er);
                *dp++ = (unsigned char)LRRound(eg);
                *dp++ = (unsigned char)LRRound(eb);
            }
        }
    }
}


/*----------------------------------------------------------------------------
 * CalculateJND() - calculates the JNDs for the specified image
 *
 * image = pointer to YUV array containing image
 * w = width
 * h = height.
 *
 * Returns pointer to array of same dimensions as image containing
 * the JND values. The caller must delete the jnd array eventually.
 *
 * This function implements the Just Noticeable Distortion technique
 * in the Chou and Li paper. It returns an array of doubles containing
 * the JNDs calculated from the image. Note that this only operates
 * on the luminance values.
 */
double *LRCalculateJND(YUV *image, int w, int h)
{

    double    *jnd;
    int        row, col;

    double    bg;                  /* average background */
    double    mg;                  /* maxiumum weighted average */
    double    alpha;               /* background-luminance function */
    double    beta;                /* background-luminance function */
    double    lambda = 0.5;        /* spatial masking parameter */
    double    gamma = 3.0/128.0;   /* line slope parameter */
    double    thresh = 17;         /* visibility threshold at 0 background */
    double    f1;                  /* spatial masking function */
    double    f2;                  /* background luminance effect */

    jnd = (double *)calloc(w * h, sizeof(double));

    for (row = 0; row < h; row ++)
        for (col = 0; col < w; col++)
        { /* for each pixel in the image */
            bg = LRCalculateJNDbg(image, col, row, w, h);    /* compute average background */
            mg = LRCalculateJNDmg(image, col, row, w, h);    /* compute maximum weighted average */
            alpha = bg * 0.0001 + 0.115; /* compute alpha function */
            beta = lambda - bg * 0.01;   /* compute beta function */
            f1 = mg * alpha + beta;      /* spatial masking effect */
            if (bg <= 127)
                f2 = thresh * (1 - pow(bg / 127.0, 0.5)) + 3;
            else
                f2 = gamma * (bg - 127) + 3;
            if (f1 > f2)
                jnd[row * w + col] = f1;
            else
                jnd[row * w + col] = f2;
        }
    return jnd;
}


/*----------------------------------------------------------------------------
 * LRCalculateJNDbg() - calculates the average background luminance
 *
 * image = pointer to original image
 * col, row = coords pixel under consideration
 * w = width of image
 * h = height of image
 *
 * Returns the value of bg(col, row)
 */
double LRCalculateJNDbg(YUV *image, int col, int row, int w, int h)
{

    static int B[] = {  1, 1, 1, 1, 1,
                        1, 2, 2, 2, 2,
                        1, 2, 0, 2, 1,
                        1, 2, 2, 2, 1,
                        1, 1, 1, 1, 1};

    int        x, y;
    int        i, j;
    double    bg;

    x = col - 2;
    y = row - 2;
    bg = 0;
    for (i = 0; i < 5; i++, y++)
        for (j = 0; j < 5; j++, x++)
            if (LRInRange(x, y, w, h))
                bg += image[y * w + x].Y * B[i * 5 + j];
            else
                bg += IMAGE_GREY * B[i * 5 + j];
    return bg / 32.0;
}


/*----------------------------------------------------------------------------
 * LRCalculateJNDmg() - calculate maximum weighted average
 *
 * image = pointer to original image
 * col, row = coords pixel under consideration
 * w = width of image
 * h = height of image
 *
 * Returns the value of mg(col, row)
 */
double LRCalculateJNDmg(YUV *image, int col, int row, int w, int h)
{

    static int G1[] = { 0, 0, 0, 0, 0,
                        1, 3, 8, 3, 1,
                        0, 0, 0, 0, 0,
                        -1, -3, -8, -3, -1,
                        0, 0, 0, 0, 0};

    static int G2[] = { 0, 0, 1, 0, 0,
                        0, 8, 3, 0, 0,
                        1, 3, 0, -3, -1,
                        0, 0, -3, -8, 0,
                        0, 0, -1, 0, 0};

    static int G3[] = { 0, 0, 1, 0, 0,
                        0, 0, 3, 8, 0,
                       -1, -3, 0, 3, 1,
                        0, -8, -3, 0, 0,
                        0, 0, -1, 0, 0};

    static int G4[] = { 0, 1, 0, -1, 0,
                        0, 3, 0, -3, 0,
                        0, 8, 0, -8, 0,
                        0, 3, 0, -3, 0,
                        0, 1, 0, -1, 0};

    double    mg, tmg;

    mg = -1000000;

    if ((tmg = LRCalculateJNDmggrad(image, col, row, w, h, G1)) > mg)
        mg = tmg;
    if ((tmg = LRCalculateJNDmggrad(image, col, row, w, h, G2)) > mg)
        mg = tmg;
    if ((tmg = LRCalculateJNDmggrad(image, col, row, w, h, G3)) > mg)
        mg = tmg;
    if ((tmg = LRCalculateJNDmggrad(image, col, row, w, h, G4)) > mg)
        mg = tmg;
    return mg;
}


/*----------------------------------------------------------------------------
 * LRCalculateJNDmggrad() - calculate maximum weighted average gradient functions
 *
 * image = pointer to original image
 * col, row = coords pixel under consideration
 * w = width of image
 * h = height of image
 * G = pointer to relevant grad operator
 *
 * Returns the value of grad(col, row)
 */
double LRCalculateJNDmggrad(YUV *image, int col, int row, int w, int h, int *G)
{
    int        x, y;
    int        i, j;
    double    grad;

    y = row - 2;
    x = col - 2;
    grad = 0;
    for (i = 0; i < 5; i++, y++)
        for (j = 0; j < 5; j++, x++)
            if (LRInRange(x, y, w, h))
                grad += image[y * w + x].Y * G[i * 5 + j];
            else
                grad += IMAGE_GREY * G[i * 5 + j];
    return fabs(grad / 16.0);

}


/*----------------------------------------------------------------------------
 * LRRound(double val)
 *
 * Performs rounding of the double value to an int
 */
int LRRound(double val)
{
    return  (val < 0 ) ? (int) ( (val) - 0.5 ) : (int) ( (val) + 0.5 );
}


/*----------------------------------------------------------------------------
 * LRInRange() - checks if (row, col) in range for (h, w)
 */
int LRInRange(int col, int row, int w, int h)
{
    if (col < 0)
        return FALSE;
    if (col >= w)
        return FALSE;
    if (row < 0)
        return FALSE;
    if (row >= h)
        return FALSE;
    return TRUE;
}

