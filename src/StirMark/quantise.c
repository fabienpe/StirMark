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
 * $Header: /StirMark/quantise.c 4     7/04/99 11:26 Fapp2 $
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <search.h>
#include <time.h>

#include "stirmark.h"
#include "error.h"
#include "image.h"
#include "quantise.h"


/* Constants */

#define PGM_MAXMAXVAL (255)
#define PPM_MAXMAXVAL (PGM_MAXMAXVAL)
#define HASH_SIZE     (20023)
#define MAXCOLORS     (32767)
/*
#define LARGE_NORM
*/
#define LARGE_LUM
/*
#define REP_CENTER_BOX
#define REP_AVERAGE_COLORS
*/
#define REP_AVERAGE_PIXELS


/* Macros */

#define PPM_GETR(p) ((p).r)
#define PPM_GETG(p) ((p).g)
#define PPM_GETB(p) ((p).b)
#define PPM_LUMIN(p) (0.299 * PPM_GETR(p) + 0.587 * PPM_GETG(p) + 0.114 * PPM_GETB(p))
#define PPM_EQUAL(p,q) ((p).r == (q).r && (p).g == (q).g && (p).b == (q).b)
#define PPM_HASHPIXEL(p) ((((long) PPM_GETR(p) * 33023 + (long) PPM_GETG(p) * 30013 + (long) PPM_GETB(p) * 27011) & 0x7fffffff) % HASH_SIZE)
#define PPM_ASSIGN(p,red,grn,blu) { (p).r = (pixval)(red); (p).g = (pixval)(grn); (p).b = (pixval)(blu); }
#define PPM_DEPTH(newp,p,oldmaxval,newmaxval) \
            PPM_ASSIGN((newp), \
            ((int) PPM_GETR(p) * (newmaxval) + (oldmaxval) / 2) / (oldmaxval), \
            ((int) PPM_GETG(p) * (newmaxval) + (oldmaxval) / 2) / (oldmaxval), \
            ((int) PPM_GETB(p) * (newmaxval) + (oldmaxval) / 2) / (oldmaxval))


/* Comparison function. Used with qsort */

typedef int(*cmpfunc)(const void *, const void *);


/* Color histogram stuff */

typedef struct colorhist_item* colorhist_vector;
struct colorhist_item
{
    pixel color;
    int   value;
};
typedef struct colorhist_list_item* colorhist_list;
struct colorhist_list_item
{
    struct colorhist_item ch;
    colorhist_list        next;
};
typedef colorhist_list* colorhash_table;
typedef int (* ifunptr)();

/* a code_int must be able to hold 2**BITS values of type int, and also -1 */
typedef int             code_int;
#ifdef SIGNED_COMPARE_SLOW
typedef unsigned long int count_int;
typedef unsigned short int count_short;
#else /*SIGNED_COMPARE_SLOW*/
typedef long int          count_int;
#endif /*SIGNED_COMPARE_SLOW*/

typedef struct box* box_vector; /* For the quantisation itself */
struct box
{
    int ind;
    int colors;
    int sum;
};


/* Local functions */

static colorhist_vector ppm_computecolorhist(pixel**  pixels, int cols, int rows, int maxcolors, int *colorsP);
static colorhist_vector ppm_colorhashtocolorhist(colorhash_table cht, int maxcolors);
static colorhist_vector mediancut (colorhist_vector chv, int colors, int sum, pixval maxval, int newcolors);
static colorhash_table  ppm_computecolorhash(pixel** pixels, int cols, int rows, int maxcolors, int *colorsP);
static colorhash_table  ppm_alloccolorhash();
static int   ppm_lookupcolor(colorhash_table cht, pixel *colorP);
static int   ppm_addtocolorhash(colorhash_table  cht, pixel*  colorP, int value);
static void  ppm_freecolorhash(colorhash_table cht);
static void  ppm_freecolorhist(colorhist_vector  chv);
static char* pm_allocrow(int cols, int size);
static int   redcompare(const colorhist_vector ch1, const colorhist_vector ch2);
static int   greencompare(const colorhist_vector ch1, const colorhist_vector ch2);
static int   bluecompare(const colorhist_vector ch1, const colorhist_vector ch2);
static int   sumcompare(const box_vector b1, const box_vector b2);


/*----------------------------------------------------------------------------
 * Reduce the number of colours of the source image sI.
 * The new image (dI) contains at most nColors.
 * dI should be cleared after usage.
 */
void ColorQuantisation(IMAGE *dI, IMAGE *sI, int nColors)
{
    pixel** mappixels;
    register pixel* pP;
    register int col, limitcol, ind;
    int row;
    pixval newmaxval;
    int colors;
    colorhist_vector chv, colormap;
    colorhash_table cht;
    int usehash, floyd = 1;
    long* thisrerr = NULL;
    long* nextrerr = NULL;
    long* thisgerr = NULL;
    long* nextgerr = NULL;
    long* thisberr = NULL;
    long* nextberr = NULL;
    long* temperr  = NULL;
    register long sr = 0, sg = 0, sb = 0, err = 0;
    int fs_direction = 0;
#define FS_SCALE 1024

    mappixels = (pixel**) 0;

    /* Step 1: Duplicate the source image. From now on,
     * the program will only work on the copy.
     */
    ImageDuplicate(dI, sI);
    
    /* Step 2: attempt to make a histogram of the colors, unclustered.
     * If at first we don't succeed, lower maxval to increase color
     * coherence and try again.  This will eventually terminate, with
     * maxval at worst 15, since 32^3 is approximately MAXCOLORS.
     */
    for (; ;)
    {
        MESSAGE("making histogram...");
        chv = ppm_computecolorhist(
        /*pixels*/(pixel **)&(dI->img), dI->size.x, dI->size.y, MAXCOLORS, &colors);
        if (chv != (colorhist_vector) 0)
            break;
        MESSAGE("too many colors!");
        newmaxval = (pixval)(dI->max / 2);
        MESSAGE("scaling colors from maxval=%d to maxval=%d to improve clustering...", dI->max, newmaxval);
        pP = (pixel *)&(dI->img[0]);
        for (row = 0; row < dI->size.y; ++row)
            for (col = 0; col < dI->size.x; ++col, ++pP)
                PPM_DEPTH(*pP, *pP, dI->max, newmaxval);
        dI->max = newmaxval;
    }
    MESSAGE("%d colors found", colors);


    /* Step 3: apply median-cut to histogram, making the new colormap.
     */
    MESSAGE("choosing %d colors...", nColors);
    colormap = mediancut(chv, colors, dI->size.y * dI->size.x, dI->max, nColors);
    ppm_freecolorhist(chv);


    /* Step 4: map the colors in the image to their closest match in the
     * new colormap, and write 'em out.
     */
    MESSAGE("mapping image to new colors...");
    cht = ppm_alloccolorhash();
    usehash = 1;
    if (floyd)
    {
        /* Initialize Floyd-Steinberg error vectors. */
        thisrerr = (long*) pm_allocrow(dI->size.x + 2, sizeof(long));
        nextrerr = (long*) pm_allocrow(dI->size.x + 2, sizeof(long));
        thisgerr = (long*) pm_allocrow(dI->size.x + 2, sizeof(long));
        nextgerr = (long*) pm_allocrow(dI->size.x + 2, sizeof(long));
        thisberr = (long*) pm_allocrow(dI->size.x + 2, sizeof(long));
        nextberr = (long*) pm_allocrow(dI->size.x + 2, sizeof(long));
        srand((int) (time(0)));
        for (col = 0; col < dI->size.x + 2; ++col)
        {
            thisrerr[col] = rand() % (FS_SCALE * 2) - FS_SCALE;
            thisgerr[col] = rand() % (FS_SCALE * 2) - FS_SCALE;
            thisberr[col] = rand() % (FS_SCALE * 2) - FS_SCALE;
            /* (random errors in [-1 .. 1]) */
        }
        fs_direction = 1;
    }
    for (row = 0; row < dI->size.y; ++row)
    {
        if (floyd)
        {
            for (col = 0; col < dI->size.x + 2; ++col)
                nextrerr[col] = nextgerr[col] = nextberr[col] = 0;
        }

        if ((! floyd) || fs_direction)
        {
            col = 0;
            limitcol = dI->size.x;
            pP = (pixel *)&(dI->img[dI->depth * (dI->size.x * row)]);
        }
        else
        {
            col = dI->size.x - 1;
            limitcol = -1;
            pP = (pixel *)&(dI->img[dI->depth * (dI->size.x * row + col)]);
        }

        do
        {
            if (floyd)
            {
                /* Use Floyd-Steinberg errors to adjust actual color. */
                sr = PPM_GETR(*pP) + thisrerr[col + 1] / FS_SCALE;
                sg = PPM_GETG(*pP) + thisgerr[col + 1] / FS_SCALE;
                sb = PPM_GETB(*pP) + thisberr[col + 1] / FS_SCALE;
                if (sr < 0) sr = 0;
                else if (sr > dI->max) sr = dI->max;
                if (sg < 0) sg = 0;
                else if (sg > dI->max) sg = dI->max;
                if (sb < 0) sb = 0;
                else if (sb > dI->max) sb = dI->max;
                PPM_ASSIGN(*pP, sr, sg, sb);
            }

            /* Check hash table to see if we have already matched this color. */
            ind = ppm_lookupcolor(cht, pP);
            if (ind == -1)
            { /* No; search colormap for closest match. */
                register int i, r1, g1, b1, r2, g2, b2;
                register long dist, newdist;
                r1 = PPM_GETR(*pP);
                g1 = PPM_GETG(*pP);
                b1 = PPM_GETB(*pP);
                dist = 2000000000;
                for (i = 0; i < nColors; ++i)
                {
                    r2 = PPM_GETR(colormap[i].color);
                    g2 = PPM_GETG(colormap[i].color);
                    b2 = PPM_GETB(colormap[i].color);
                    newdist = (r1 - r2) * (r1 - r2) +
                          (g1 - g2) * (g1 - g2) +
                          (b1 - b2) * (b1 - b2);
                    if (newdist < dist)
                    {
                        ind = i;
                        dist = newdist;
                    }
                }
                if (usehash)
                {
                    if (ppm_addtocolorhash(cht, pP, ind) < 0)
                    {
                        MESSAGE("out of memory adding to hash table, proceeding without it");
                        usehash = 0;
                    }
                }
            }

            if (floyd)
            {
                /* Propagate Floyd-Steinberg error terms. */
                if (fs_direction)
                {
                    err = (sr - (long) PPM_GETR(colormap[ind].color)) * FS_SCALE;
                    thisrerr[col + 2] += (err * 7) / 16;
                    nextrerr[col    ] += (err * 3) / 16;
                    nextrerr[col + 1] += (err * 5) / 16;
                    nextrerr[col + 2] += (err    ) / 16;
                    err = (sg - (long) PPM_GETG(colormap[ind].color)) * FS_SCALE;
                    thisgerr[col + 2] += (err * 7) / 16;
                    nextgerr[col    ] += (err * 3) / 16;
                    nextgerr[col + 1] += (err * 5) / 16;
                    nextgerr[col + 2] += (err    ) / 16;
                    err = (sb - (long) PPM_GETB(colormap[ind].color)) * FS_SCALE;
                    thisberr[col + 2] += (err * 7) / 16;
                    nextberr[col    ] += (err * 3) / 16;
                    nextberr[col + 1] += (err * 5) / 16;
                    nextberr[col + 2] += (err    ) / 16;
                }
                else
                {
                    err = (sr - (long) PPM_GETR(colormap[ind].color)) * FS_SCALE;
                    thisrerr[col    ] += (err * 7) / 16;
                    nextrerr[col + 2] += (err * 3) / 16;
                    nextrerr[col + 1] += (err * 5) / 16;
                    nextrerr[col    ] += (err    ) / 16;
                    err = (sg - (long) PPM_GETG(colormap[ind].color)) * FS_SCALE;
                    thisgerr[col    ] += (err * 7) / 16;
                    nextgerr[col + 2] += (err * 3) / 16;
                    nextgerr[col + 1] += (err * 5) / 16;
                    nextgerr[col    ] += (err    ) / 16;
                    err = (sb - (long) PPM_GETB(colormap[ind].color)) * FS_SCALE;
                    thisberr[col    ] += (err * 7) / 16;
                    nextberr[col + 2] += (err * 3) / 16;
                    nextberr[col + 1] += (err * 5) / 16;
                    nextberr[col    ] += (err    ) / 16;
                }
            }

            *pP = colormap[ind].color;

            if ((! floyd) || fs_direction)
            {
                ++col;
                ++pP;
            }
            else
            {
                --col;
                --pP;
            }
        }
        while (col != limitcol);
        
        if (floyd)
        {
            temperr  = thisrerr;
            thisrerr = nextrerr;
            nextrerr = temperr;
            temperr  = thisgerr;
            thisgerr = nextgerr;
            nextgerr = temperr;
            temperr  = thisberr;
            thisberr = nextberr;
            nextberr = temperr;

            fs_direction = ! fs_direction;
        }
    }
}


/*----------------------------------------------------------------------------
 * Here is the fun part, the median-cut colormap generator.  This is based
 * on Paul Heckbert's paper "Color Image Quantization for Frame Buffer
 * Display", SIGGRAPH '82 Proceedings, page 297.
 */
static colorhist_vector
mediancut(colorhist_vector chv, int colors, int sum, pixval maxval, int newcolors)
{
    colorhist_vector colormap;
    box_vector bv;
    register int bi, i;
    int boxes;

    bv = (box_vector) malloc(sizeof(struct box) * newcolors);
    colormap =
    (colorhist_vector) malloc(sizeof(struct colorhist_item) * newcolors);
    if (bv == (box_vector) 0 || colormap == (colorhist_vector) 0)
    ERROR("out of memory");
    for (i = 0; i < newcolors; ++i)
    PPM_ASSIGN(colormap[i].color, 0, 0, 0);

    /* Set up the initial box. */
    bv[0].ind = 0;
    bv[0].colors = colors;
    bv[0].sum = sum;
    boxes = 1;

    /* Main loop: split boxes until we have enough. */
    while (boxes < newcolors)
    {
        register int indx, clrs;
        int sm;
        register int minr, maxr, ming, maxg, minb, maxb, v;
        int halfsum, lowersum;

        /* Find the first splittable box. */
        for (bi = 0; bi < boxes; ++bi)
            if (bv[bi].colors >= 2)
                break;
        if (bi == boxes)
            break;    /* ran out of colors! */
        indx = bv[bi].ind;
        clrs = bv[bi].colors;
        sm = bv[bi].sum;

        /* Go through the box finding the minimum and maximum of each
         * component - the boundaries of the box.
         */
        minr = maxr = PPM_GETR(chv[indx].color);
        ming = maxg = PPM_GETG(chv[indx].color);
        minb = maxb = PPM_GETB(chv[indx].color);
        for (i = 1; i < clrs; ++i)
        {
            v = PPM_GETR(chv[indx + i].color);
            if (v < minr) minr = v;
            if (v > maxr) maxr = v;
            v = PPM_GETG(chv[indx + i].color);
            if (v < ming) ming = v;
            if (v > maxg) maxg = v;
            v = PPM_GETB(chv[indx + i].color);
            if (v < minb) minb = v;
            if (v > maxb) maxb = v;
        }

        /* Find the largest dimension, and sort by that component.  I have
         * included two methods for determining the "largest" dimension;
         * first by simply comparing the range in RGB space, and second
         * by transforming into luminosities before the comparison.  You
         * can switch which method is used by switching the commenting on
         * the LARGE_ defines at the beginning of this source file.
        */
#ifdef LARGE_NORM
        if (maxr - minr >= maxg - ming && maxr - minr >= maxb - minb)
            qsort((char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
                  (cmpfunc)redcompare);
        else if (maxg - ming >= maxb - minb)
            qsort((char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
                  (cmpfunc)greencompare);
        else
            qsort((char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
                  (cmpfunc)bluecompare);
#endif /*LARGE_NORM*/
#ifdef LARGE_LUM
        {
            pixel p;
            float rl, gl, bl;

            PPM_ASSIGN(p, maxr - minr, 0, 0);
            rl = (float)PPM_LUMIN(p);
            PPM_ASSIGN(p, 0, maxg - ming, 0);
            gl = (float)PPM_LUMIN(p);
            PPM_ASSIGN(p, 0, 0, maxb - minb);
            bl = (float)PPM_LUMIN(p);
            
            if (rl >= gl && rl >= bl)
                qsort((char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
                      (cmpfunc)redcompare);
            else if (gl >= bl)
                qsort((char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
                      (cmpfunc)greencompare);
            else
                qsort((char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
                      (cmpfunc)bluecompare);
        }
#endif /*LARGE_LUM*/
    
        /* Now find the median based on the counts, so that about half the
         * pixels (not colors, pixels) are in each subdivision.
         */
        lowersum = chv[indx].value;
        halfsum = sm / 2;
        for (i = 1; i < clrs - 1; ++i)
        {
            if (lowersum >= halfsum)
            break;
            lowersum += chv[indx + i].value;
        }

        /* Split the box, and sort to bring the biggest boxes to the top.
         */
        bv[bi].colors = i;
        bv[bi].sum = lowersum;
        bv[boxes].ind = indx + i;
        bv[boxes].colors = clrs - i;
        bv[boxes].sum = sm - lowersum;
        ++boxes;
        qsort((char*) bv, boxes, sizeof(struct box), (cmpfunc)sumcompare);
    }

    /* Ok, we've got enough boxes.  Now choose a representative color for
     * each box.  There are a number of possible ways to make this choice.
     * One would be to choose the center of the box; this ignores any structure
     * within the boxes.  Another method would be to average all the colors in
     * the box - this is the method specified in Heckbert's paper.  A third
     * method is to average all the pixels in the box.  You can switch which
     * method is used by switching the commenting on the REP_ defines at
     * the beginning of this source file.
     */
    for (bi = 0; bi < boxes; ++bi)
    {
#ifdef REP_CENTER_BOX
        register int indx = bv[bi].ind;
        register int clrs = bv[bi].colors;
        register int minr, maxr, ming, maxg, minb, maxb, v;

        minr = maxr = PPM_GETR(chv[indx].color);
        ming = maxg = PPM_GETG(chv[indx].color);
        minb = maxb = PPM_GETB(chv[indx].color);
        for (i = 1; i < clrs; ++i)
        {
            v = PPM_GETR(chv[indx + i].color);
            minr = min(minr, v);
            maxr = max(maxr, v);
            v = PPM_GETG(chv[indx + i].color);
            ming = min(ming, v);
            maxg = max(maxg, v);
            v = PPM_GETB(chv[indx + i].color);
            minb = min(minb, v);
            maxb = max(maxb, v);
        }
        PPM_ASSIGN(colormap[bi].color, (minr + maxr) / 2, (ming + maxg) / 2,
                   (minb + maxb) / 2);
#endif /*REP_CENTER_BOX*/
#ifdef REP_AVERAGE_COLORS
        register int indx = bv[bi].ind;
        register int clrs = bv[bi].colors;
        register long r = 0, g = 0, b = 0;

        for (i = 0; i < clrs; ++i)
        {
            r += PPM_GETR(chv[indx + i].color);
            g += PPM_GETG(chv[indx + i].color);
            b += PPM_GETB(chv[indx + i].color);
        }
        r = r / clrs;
        g = g / clrs;
        b = b / clrs;
        PPM_ASSIGN(colormap[bi].color, r, g, b);
#endif /*REP_AVERAGE_COLORS*/
#ifdef REP_AVERAGE_PIXELS
        register int indx = bv[bi].ind;
        register int clrs = bv[bi].colors;
        register long r = 0, g = 0, b = 0, sum = 0;

        for (i = 0; i < clrs; ++i)
        {
            r += PPM_GETR(chv[indx + i].color) * chv[indx + i].value;
            g += PPM_GETG(chv[indx + i].color) * chv[indx + i].value;
            b += PPM_GETB(chv[indx + i].color) * chv[indx + i].value;
            sum += chv[indx + i].value;
        }
        r = r / sum;
        if (r > maxval) r = maxval;    /* avoid math errors */
        g = g / sum;
        if (g > maxval) g = maxval;
        b = b / sum;
        if (b > maxval) b = maxval;
        PPM_ASSIGN(colormap[bi].color, r, g, b);
#endif /*REP_AVERAGE_PIXELS*/
    }

    /* All done. */
    return colormap;
}


/*----------------------------------------------------------------------------
 */
static int redcompare(const colorhist_vector ch1, const colorhist_vector ch2)
{
    return (int) PPM_GETR(ch1->color) - (int) PPM_GETR(ch2->color);
}


/*----------------------------------------------------------------------------
 */
static int greencompare(const colorhist_vector ch1, const colorhist_vector ch2)
{
    return (int) PPM_GETG(ch1->color) - (int) PPM_GETG(ch2->color);
}


/*----------------------------------------------------------------------------
 */
static int bluecompare(const colorhist_vector  ch1, const colorhist_vector  ch2)
{
    return (int) PPM_GETB(ch1->color) - (int) PPM_GETB(ch2->color);
}


/*----------------------------------------------------------------------------
 */
static int sumcompare(const box_vector b1, const box_vector b2)
{
    return b2->sum - b1->sum;
}


/*----------------------------------------------------------------------------
 */
static void ppm_freecolorhist(colorhist_vector  chv)
{
    free((char*) chv);
}


/*----------------------------------------------------------------------------
 */
static char* pm_allocrow(int cols, int size)
{
    register char* itrow;

    itrow = (char*) malloc(cols * size);
    if (itrow == (char*) 0)
    ERROR("out of memory allocating a row");
    return itrow;
}


/*----------------------------------------------------------------------------
 */
static int ppm_addtocolorhash(colorhash_table  cht, pixel*  colorP, int value)
{
    register int hash;
    register colorhist_list chl;


    chl = (colorhist_list) malloc(sizeof(struct colorhist_list_item));
    if (chl == 0)
    return -1;
    hash = PPM_HASHPIXEL(*colorP);
    chl->ch.color = *colorP;
    chl->ch.value = value;
    chl->next = cht[hash];
    cht[hash] = chl;
    return 0;
}


/*----------------------------------------------------------------------------
 */
static colorhist_vector ppm_computecolorhist(pixel**  pixels,
                                             int cols,
                                             int rows, 
                                             int maxcolors,
                                             int *colorsP)
{
    colorhash_table  cht;
    colorhist_vector chv;

    cht = ppm_computecolorhash(pixels, cols, rows, maxcolors, colorsP);
    if (cht == (colorhash_table) 0)
        return (colorhist_vector) 0;
    chv = ppm_colorhashtocolorhist(cht, maxcolors);
    ppm_freecolorhash(cht);
    return chv;
}


/*----------------------------------------------------------------------------
 */
static void ppm_freecolorhash(colorhash_table cht)
{
    int i;
    colorhist_list chl, chlnext;

    for (i = 0; i < HASH_SIZE; ++i)
    for (chl = cht[i]; chl != (colorhist_list) 0; chl = chlnext)
    {
        chlnext = chl->next;
        free((char*) chl);
    }
    free((char*) cht);
}


/*----------------------------------------------------------------------------
 */
static colorhash_table ppm_computecolorhash(pixel** pixels,
                                            int cols,
                                            int rows, 
                                            int maxcolors,
                                            int *colorsP)
    {
    colorhash_table cht;
    register pixel* pP = pixels[0];
    colorhist_list  chl;
    int col, row, hash;

    cht = ppm_alloccolorhash();
    *colorsP = 0;

    /* Go through the entire image, building a hash table of colors. */
    for (row = 0; row < rows; ++row)
    {
        for (col = 0; col < cols; ++col, ++pP)
        {
            hash = PPM_HASHPIXEL(*pP);
            for (chl = cht[hash]; chl != (colorhist_list) 0; chl = chl->next)
                if (PPM_EQUAL(chl->ch.color, *pP))
                    break;
            if (chl != (colorhist_list) 0)
                ++(chl->ch.value);
            else
            {
                if (++(*colorsP) > maxcolors)
                {
                    ppm_freecolorhash(cht);
                    return (colorhash_table) 0;
                }
                chl = (colorhist_list) malloc(sizeof(struct colorhist_list_item));
                if (chl == 0)
                    ERROR("out of memory computing hash table");
                chl->ch.color = *pP;
                chl->ch.value = 1;
                chl->next = cht[hash];
                cht[hash] = chl;
            }
        }
    }

    return cht;
}


/*----------------------------------------------------------------------------
 */
static colorhash_table ppm_alloccolorhash()
{
    colorhash_table cht;
    int i;

    cht = (colorhash_table) malloc(HASH_SIZE * sizeof(colorhist_list));
    if (cht == 0)
        ERROR("out of memory allocating hash table");

    for (i = 0; i < HASH_SIZE; ++i)
        cht[i] = (colorhist_list) 0;

    return cht;
}


/*----------------------------------------------------------------------------
 */
static colorhist_vector ppm_colorhashtocolorhist(colorhash_table cht,
                                                 int maxcolors)
{
    colorhist_vector chv;
    colorhist_list   chl;
    int i, j;

    /* Now collate the hash table into a simple colorhist array. */
    chv = (colorhist_vector) malloc(maxcolors * sizeof(struct colorhist_item));
    /* (Leave room for expansion by caller.) */
    if (chv == (colorhist_vector) 0)
        ERROR("out of memory generating histogram");

    /* Loop through the hash table. */
    j = 0;
    for (i = 0; i < HASH_SIZE; ++i)
    for (chl = cht[i]; chl != (colorhist_list) 0; chl = chl->next)
    {
        /* Add the new entry. */
        chv[j] = chl->ch;
        ++j;
    }

    /* All done. */
    return chv;
}


/*----------------------------------------------------------------------------
 */
static int ppm_lookupcolor(colorhash_table cht, pixel *colorP)
{
    int hash;
    colorhist_list chl;

    hash = PPM_HASHPIXEL(*colorP);
    for (chl = cht[hash]; chl != (colorhist_list) 0; chl = chl->next)
        if (PPM_EQUAL(chl->ch.color, *colorP))
            return chl->ch.value;

    return -1;
}

