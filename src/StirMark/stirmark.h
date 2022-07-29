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
 * $Header: /StirMark/stirmark.h 26    7/04/99 11:28 Fapp2 $
 *----------------------------------------------------------------------------
 */

#ifndef _STIRMARK_H_
#define _STIRMARK_H_

#include <stdio.h>
#include <assert.h>

#define VERSION "3.1 (79)"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif

/* Displays more comments during execution
#define VERBOSE
*/

/* Available tests */
#define TEST_END_OF_LIST        (0)

#define TEST_FILTERING          (11)
#define TEST_MEDIAN_FILTERING   (12)
#define TEST_FMLR               (13)

#define TEST_COMPRESSION        (21)
#define TEST_QUANTISE           (22)

#define TEST_SCALING            (31)
#define TEST_SHEAR              (32)
#define TEST_ASPECT_RATIO       (33)
#define TEST_GENERAL_LINEAR     (34)
#define TEST_ROTATION_CROP      (35)
#define TEST_ROTATION_SCALE     (36)

#define TEST_CROPPING           (41)
#define TEST_FLIP               (42)
#define TEST_ROW_COL_REMOVAL    (43)

#define TEST_GEOM_DISTORTIONS   (51)

#define TEST_ALL                (999)


/* The default reconstruction function is defined into reconstructers.h */


/* Global constants */

#ifndef PI
#define PI    (3.14159265358979323846)
#endif

#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef NULL
#define NULL  ((void *)(0))
#endif

#define MAX_BASENAME_LENGTH (32)  /* Without attack name */
#define MAX_FILENAME_LENGTH (250) /* Without extension : .ppm, .pgm, etc. */

#define MAX_DEPTH           (3)   /* maximum number of bytes per pixel */
#define ONE_MINUS_EPSILON   (0.99999999999) /* blocks -- used in resampler */
#define ERRSTEP             (16)  /* Transfer function */
#define TEXT_LINE_LENGTH    (120) /* buffer size */
#define MAX_FILTER_SIZE     (3)   /* Max size fo convolution filters */

#define DIST_OP_NEW      (1) /* Use new random distortion paramters */
#define DIST_OP_SAVE     (2) /* Use new parameters and save them to file */
#define DIST_OP_LOAD     (3) /* Load paramters from file */
#define DIST_OP_SAVE_ALL (4) /* Save ALL the random numbers used in the process */
#define DIST_OP_LOAD_ALL (5) /* Load ALL the random numbers used in the process */

/* Cleanup macro */

#define UNREFERENCED_PARAMETER(P) \
    { \
        (P) = (P); \
    } \


/* New types */

typedef int BOOL;

/* Pixels stuff */

typedef unsigned char bit;
typedef unsigned char gray;
typedef gray          pixval;
typedef struct
{
    pixval r, g, b;
} pixel;


typedef struct DVECTOR_TAG
{
    double x;
    double y;
} DVECTOR;

typedef struct IVECTOR_TAG
{
    int x;
    int y;
} IVECTOR;

typedef struct IMAGE_TAG
{
    unsigned char * img;   /* The image is just a table of char */
    IVECTOR         size;  /* Size of the imge in pixels */
    int             depth; /* Number of color per pixel (1 or 3)*/
    unsigned char   max;   /* Maximum value for pixels */
} IMAGE;


/* Interpolation parameters */

typedef struct NYQUIST_TAG
{
    DVECTOR rolloff;
    IVECTOR highpass;
    int     cutoff;   /* Number of lobes */
} NYQUIST;


/* Command line options for distortion */

typedef struct OPTIONS_TAG
{
    /* Don't forget to modify the consts in bench.h    */
    /* if you modify the order ot the following fileds */
    double inwards;
    double outwards;
    int    relative_inwards;
    int    relative_outwards;
    double random_factor;
    double bending_factor;
    int    jpeg_quality;
    double deviation;   /* Color deviation -- transfer function */
    int    parameters;  /* DIST_OP_USE_NEW, DIST_OP_SAVE or DIST_OP_LOAD */
    char*  pszFileName; /* File used to read from or write to distortion paremeters */

} OPTIONS;


/* `StirMark' distortions parameters */

typedef struct DISTORTION_TAG
{
    double  err[256 / ERRSTEP + 1][MAX_DEPTH]; /* Transfer function */
    double  deviation;  /* Color deviation -- transfer function */
    double  bending_factor;
    double  random_factor;
    DVECTOR ul;  /* upper left corner */
    DVECTOR ur;  /* upper right corner */
    DVECTOR ll;  /* lower left corner */
    DVECTOR lr;  /* lower right corner */
    int     parameters;  /* DIST_OP_USE_NEW, DIST_OP_SAVE or DIST_OP_LOAD */
} DISTORTION;

/* General structure for transformation */

typedef struct PROCESS_INFO_TAG
{
    IMAGE   sI, dI;       /* Source and destination images*/
    IVECTOR dPrefSize;    /* Prefered number of pixels in the destination */
    int     nRec,         /* Define which reconstructor */
            nTra,         /* Transformer and sampler */
            nSam;         /* are used in the process */
    IVECTOR offset;       /* From top left in source image */
    void   *param;        /* *distortion, *rotation, *scaling */
    NYQUIST *resampling;  /* If requires resampling the give parameters */
    int     mirrorBorders;/* If pixel outside original, then mirror inside */
	char    pszFileName[TEXT_LINE_LENGTH];  /* String that can be appended at end of file name */
	char    pszTestName[TEXT_LINE_LENGTH];  /* String displayed while performing the test */
} PROCESS_INFO;

/* Filter */

typedef struct FILTER_TAG
{
	char name[TEXT_LINE_LENGTH];                /* Name of the filter */
	int size;                                   /* Size of the filter <= MAX_FILTER_SIZE */
    int coef[MAX_FILTER_SIZE][MAX_FILTER_SIZE]; /* Convolution filters */
} FILTER;


/* Process struct */

typedef struct PROCESS_TAG {
    char name[TEXT_LINE_LENGTH];        /* Name of the process is fixed */
    char description[TEXT_LINE_LENGTH]; /* Description is variable */
    int  reference;       /* To call processes using this reference number */
    int  type;            /* To distinguish process types */
    BOOL (*define)();     /* Define any global parameters of the process */
    BOOL (*setup)();      /* Setup any info for each image being resampled */
    BOOL (*initialise)(); /* Initialise, ready to resample */
    double (*func)();     /* The function itself */
    BOOL (*cleanup)();    /* Any clean-up that needs doing */
} PROCESS;


/* Test info struct */

/* Used typically to apply a rotation, say, with nParam   */
/* different angles. These angles are stored into pParam. */
typedef struct TEST_TAG
{
    int     nTest;    /* Type of test */
    int     nRec, nTra, nSam;              /* Type of transformation */
    void   *pParam;                        /* Set of parameters for the transform */
    double  nParam;                        /* Number of parameers */
	size_t  sParam;                        /* Size of parameters */
} TEST;

#endif

