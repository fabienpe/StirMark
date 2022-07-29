/*----------------------------------------------------------------------------
 * StirMark -- Benchmark
 *
 * Main routines of the benchmark. These routines mainly prepare the
 * parameters passed to the transformation / distortion functions
 * defined in `transformations.c'
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
 * $Header: /StirMark/bench.c 23    7/04/99 11:24 Fapp2 $
 *----------------------------------------------------------------------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stirmark.h"
#include "bench.h"
#include "error.h"
#include "image.h"
#include "resamplers.h"
#include "transformations.h"
#include "reconstructers.h"
#include "quality.h"
#include "lrattack.h"
#include "quantise.h"



/* Each table contains different values for the same parameter */
/* See StirMark.h for more details about these parameters      */
static double pScaleFactors[][2]    = SCALE_FACTORS;
static double pRotationAngles[]     = ROTATION_ANGLES;
static int    pQualityFactors[]     = JPEG_QUALITY_FACTORS;
static int    pCropFactors[]        = CROP_FACTORS;
static int    pRemove[][2]          = NUMBER_ROW_COL_REMOVED;
static double pShifts[][2]          = SHIFTS;
static double pAspectRatios[][2]    = ASPECT_RATIOS;
static double pLR[]                 = LR_STRENGTHS;
static double pJNDLR[]              = JNDLR_STRENGTHS;
static double pLinearTrans[][2][2]  = LINEAR_TRANSFORMATIONS;
static FILTER pFilters[]            = FILTERS;
static IVECTOR pMedianFilterSizes[] = MEDIAN_FILTER_SIZES;
static OPTIONS pGeomDistortions[]   = DISTORTIONS;


/* Parameters for each attack will be passed through          */
/* void* so need to keep track and number of these parameters */
#define N_ELEMENTS(a) (sizeof((a)) / sizeof((a)[0]))
#define EXPAND_PARAMETER_LIST(a) (a), N_ELEMENTS((a)), sizeof((a)[0])

/* List of possible tests with parameters:
 *  - test number
 *  - character string used to display progress information
 *  - index of the reconstructer used by the transformation (reconstructers.h)
 *  - index of the transformation (transformations.h)
 *  - index of the sampler (resampler.h)
 *  - array of parameters
 *  - number of elements in arrays of parameters
 *  - character string used to format the out file name
 */
static TEST pTestsInfo[] = {
    {TEST_CROPPING,
        REC_NEAREST_NEIGHBOUR, TRA_CROPPING, SAM_POINT,
        EXPAND_PARAMETER_LIST(pCropFactors)},

    {TEST_ROW_COL_REMOVAL,
        REC_NEAREST_NEIGHBOUR, TRA_ROW_COL_REMOVAL, SAM_POINT,
        EXPAND_PARAMETER_LIST(pRemove)},

    {TEST_FLIP,
        REC_NEAREST_NEIGHBOUR, TRA_FLIP, SAM_POINT,
        NULL, 1, 0},

    {TEST_SCALING,
        DEFAULT_RECONSTRUCTER, TRA_SCALE_BY_VALUE, SAM_POINT,
        EXPAND_PARAMETER_LIST(pScaleFactors)},

    {TEST_ASPECT_RATIO,
        DEFAULT_RECONSTRUCTER, TRA_SCALE_BY_VALUE, SAM_POINT,
        EXPAND_PARAMETER_LIST(pAspectRatios)},

    {TEST_ROTATION_CROP,
        DEFAULT_RECONSTRUCTER, TRA_ROTATION, SAM_POINT,
        EXPAND_PARAMETER_LIST(pRotationAngles)},

    {TEST_ROTATION_SCALE,
        DEFAULT_RECONSTRUCTER, TRA_ROTATION_SCALE, SAM_POINT,
        EXPAND_PARAMETER_LIST(pRotationAngles)},

    {TEST_SHEAR,
        DEFAULT_RECONSTRUCTER, TRA_SHEAR, SAM_POINT,
        EXPAND_PARAMETER_LIST(pShifts)},

    {TEST_GENERAL_LINEAR,
        DEFAULT_RECONSTRUCTER, TRA_LINEAR, SAM_POINT,
        EXPAND_PARAMETER_LIST(pLinearTrans)},

    {TEST_GEOM_DISTORTIONS,
        DEFAULT_RECONSTRUCTER, TRA_DISTORTION, SAM_POINT,
        EXPAND_PARAMETER_LIST(pGeomDistortions)},

    {TEST_FILTERING,
        REC_CONVOLUTION_FILTER, TRA_INVARIANT, SAM_POINT,
        EXPAND_PARAMETER_LIST(pFilters)},

    {TEST_MEDIAN_FILTERING,
        REC_MEDIAN_FILTER, TRA_INVARIANT, SAM_POINT,
        EXPAND_PARAMETER_LIST(pMedianFilterSizes)},

    {TEST_END_OF_LIST, 0, 0, 0, NULL, 0, 0}
};


/*----------------------------------------------------------------------------
 * Main routine: applies resampling, reconstruction and transformation
 */
void ApplyTransformation(PROCESS_INFO *info)
{
    static bCalledBefore = FALSE;
    double result; /* the result of the sample[samNum].func() process */
    int    recNum = 0, traNum = 0, samNum = 0;

    assert(info != NULL && info->sI.img != NULL);

    /* Look for the appropriate functions */
    for(recNum = 0; (reconstructers[recNum].name[0] != 0) &&
        (reconstructers[recNum].reference != info->nRec); recNum++);
    
    for(traNum = 0; (transformers[traNum].name[0] != 0) &&
        (transformers[traNum].reference != info->nTra); traNum++);
    
    for(samNum = 0; (samplers[samNum].name[0] != 0) &&
        (samplers[samNum].reference != info->nSam); samNum++);

    /* Definitions function are called only once */
    if (bCalledBefore == FALSE)
    {
        if (reconstructers[recNum].define(&(reconstructers[recNum])))
        {
            if (transformers[traNum].define(&(transformers[traNum])))
            {
                if (samplers[samNum].define(&(samplers[samNum])))
                {
#ifdef VERBOSE
                    printf("General settings done.\n");
#endif
                }
                else WARNING("Sampler definition failed.")
            }
            else WARNING("Transformer definition failed.")
        }
        else WARNING("Reconstructer definition failed.");
        bCalledBefore = TRUE;
    }

#ifdef VERBOSE
    printf("   reconstructer - %s\n", reconstructers[recNum].description);
    printf("   transformer   - %s\n", transformers[traNum].description);
    printf("   sampler       - %s\n", samplers[samNum].description);
#endif


    /* Setup and initialise the reconstructer, transformer and sampler */
    if (reconstructers[recNum].setup(info))
        if (transformers[traNum].setup(info))
            if (samplers[samNum].setup(info))
                if (reconstructers[recNum].initialise(info))
                    if (transformers[traNum].initialise(info))
                        if (samplers[samNum].initialise(info))
                        {
                            
                            /* Apply the transformation */
                            result = (*samplers[samNum].func)(
                                reconstructers[recNum].func,
                                transformers[traNum].func,
                                info);
                            
                            /* Clean up */
                            if (!reconstructers[recNum].cleanup(info))
                                WARNING("reconstructer cleanup failed.");

                            if (!transformers[traNum].cleanup(info))
                                WARNING("transformer cleanup failed.");

                            if (!samplers[samNum].cleanup(info))
                                WARNING("sampler cleanup failed.");

                        }
                        else
                            WARNING("sampler initialisation failed.")
                    else
                        WARNING("transformer initialisation failed.")
                else
                    WARNING("reconstructer initialisation failed.")
            else
                WARNING("sampler setup failed.")
        else
            WARNING("transformer setup failed.")
    else
        WARNING("reconstructer setup failed.")
#ifdef VERBOSE
    printf("   Leaving transformation sub-system.\n");
#endif
}


/*---------------------------------------------------------------------------
 * Apply a series of test.
 */
static int test(IMAGE I, char *strzBasename, NYQUIST n, int nTest)
{
    PROCESS_INFO info;
    int i;
    double psnr;
    char strzFileName[MAX_FILENAME_LENGTH];
    
    info.param = info.resampling = NULL;

    if (I.img == NULL || strzBasename == NULL)
        ERROR("test(): NULL parameter");

    info.resampling = &n;
    info.sI   = I;
    info.nRec = pTestsInfo[nTest].nRec;
    info.nTra = pTestsInfo[nTest].nTra;
    info.nSam = pTestsInfo[nTest].nSam;
    
    /* Mirror borders on ly for geometric distortions */
    info.mirrorBorders = (pTestsInfo[nTest].nTest == TEST_GEOM_DISTORTIONS);

    /* For each value of the parameter */
    for (i = 0; i < pTestsInfo[nTest].nParam; i++)
    {
        if (pTestsInfo[nTest].pParam == NULL)
            info.param = NULL;
        else
			info.param = ((char *)pTestsInfo[nTest].pParam + 
                                  i * pTestsInfo[nTest].sParam);
        ApplyTransformation(&info);

        /* Display which test was completed */
		printf(strzBasename);printf(" - ");
        printf(info.pszTestName);
        if ((info.dI.size.x == info.sI.size.x) &&
            (info.dI.size.y == info.sI.size.y))
        {
            if ((psnr = PSNR(info.dI, info.sI)) <= MIN_QUALITY)
			    printf(" - PSNR not meaningful");
		    else
    			printf(" - PSNR = %f", psnr);
        }
        printf("\n");

        /* Format the name of the output file and save it */
		strcpy(strzFileName, strzBasename);
		strcat(strzFileName, info.pszFileName);
        if (pTestsInfo[nTest].nTest != TEST_GEOM_DISTORTIONS)
            ImageSavePPM(info.dI, strzFileName);
        if (pTestsInfo[nTest].nTest != TEST_FLIP)
            ImageSaveJPEG(info.dI, strzFileName, JPEG_QUALITY);
        ImageClear(&(info.dI));
    }
    return (0);
}


/*---------------------------------------------------------------------------
 * Test various centred cropping.
 */
static int _test_compression(IMAGE I,
                             char *strzBasename,
                             int  *pQualityFactors,
                             int   nQualityFactors)
{
    char   strzName[MAX_FILENAME_LENGTH], strzBuffer[32];
    int    i;
    double psnr;  /* PSNR of the compressed image */
    IMAGE  compI; /* Temporary image */

    if (I.img == NULL || strzBasename == NULL || pQualityFactors == NULL)
        ERROR("_test_compression: NULL parameter");

    /* For all JPEG quality factor in pQualityFactors */
    for (i = 0; i < nQualityFactors; i++)
    {
        printf(strzBasename);printf(" - ");
        sprintf(strzBuffer, "_JPEG_%d", pQualityFactors[i]);
        printf("JPEG compression %d", pQualityFactors[i]);

        /* Save image as JPEG. ".jpg" extension will */
        /* be added to filename by ImageSaveJPEG     */
        ImageSaveJPEG(I, strcat(strncpy(strzName, strzBasename, 
            MAX_BASENAME_LENGTH), strzBuffer), pQualityFactors[i]);
        
        /* Load JPEG image and compute PSNR */
        ImageRead(&compI, strcat(strzName, ".jpg"));
        if ((psnr = PSNR(compI, I)) <= MIN_QUALITY)
            printf(" - PSNR not meaningful\n");
        else
            printf(" - PSNR = %f\n", psnr);

        ImageClear(&compI);
    }
    return (0);
}


/*---------------------------------------------------------------------------
 * Apply FMLR. R. Barnett's attack
 */
static int _test_fmlr(IMAGE I, char *strzBasename)
{
    int    i;
    char   strzName[MAX_FILENAME_LENGTH];
    double psnr;
    IMAGE  dI; /* FMRLR-attacked image*/
    IMAGE  compI; /* Temporary image */

    printf(strzBasename);printf(" - LRAttack - ");
    ImageDuplicate(&dI, &I);
    for (i = 0; i < 6; i++)
        LRAttack(&dI, &dI, pLR[i], pJNDLR[i], (1 << i));

    /* Light JPEG compression applied after FMLR */
    ImageSaveJPEG(dI, strcat(strncpy(strzName, strzBasename,
        MAX_BASENAME_LENGTH), "_FMLR"), LR_QUALITY);
    ImageClear(&dI);

    /* Load attack&compressed image and computer its PSNR */
    ImageRead(&compI, strcat(strzName, ".jpg"));
    if ((psnr = PSNR(compI, I)) <= MIN_QUALITY)
        printf("PSNR not meaningful\n");
    else
        printf("PSNR = %f\n", psnr);
    ImageClear(&compI);

    return (0);
}


/*----------------------------------------------------------------------------
 * The main benchmark function
 */
void Benchmark(IMAGE I, NYQUIST n, char *strzBasename, int nTestSet)
{
    PROCESS_INFO info;
    int    nTest = 0;
    double psnr  = 0.0;

    IMAGE dI;
	char strzFileName[MAX_FILENAME_LENGTH];

    info.param = info.resampling = NULL;
    strzFileName[0] = 0;

    if (I.img == NULL || strzBasename == NULL) exit(1);

    printf("\nStirmark %s\n\n", VERSION);

    /* Apply all the tests given in the table                */
	/* or just the test set selected by the user (-S option) */
    while(pTestsInfo[nTest].nTest !=0)
    {
		if ((pTestsInfo[nTest].nTest == nTestSet) ||
			(nTestSet == TEST_ALL))
			test(I, strzBasename, n, nTest);
        nTest++;
    }


    /* Apply the remaining tests */


    /* Frequency Mode Laplacian Removal */
	if ((nTestSet == TEST_FMLR) || (nTestSet == TEST_ALL))
	    _test_fmlr(I, strzBasename);
    
    
    /* JPEG compression test */
	if ((nTestSet == TEST_COMPRESSION) || (nTestSet == TEST_ALL))
	{
		_test_compression(I, strzBasename, pQualityFactors, 
			N_ELEMENTS(pQualityFactors));
	}
    

    /* Test color quantisation -- conversion to the GIF format */
	if ((nTestSet == TEST_QUANTISE) || (nTestSet == TEST_ALL))
	{
		printf(strzBasename);
		printf(" - Color reduction");
		if (I.depth == 3)
		{
			ColorQuantisation(&dI, &I, 256);
			ImageSavePPM(dI, strcat(strncpy(strzFileName, strzBasename,
				MAX_BASENAME_LENGTH), "_reduce_colour"));
			if ((psnr = PSNR(dI, I)) <= MIN_QUALITY)
			{
				printf(" - PSNR not meaningful\n");
			}
			else
			{
				printf(" - PSNR = %f\n", psnr);
			}
			ImageClear(&dI);
		}
		else
			WARNING("GIF conversion not supported with greyscale images.");
	}
}


/*---------------------------------------------------------------------------
 * For backward compatibility (see stirmark.c)
 */
void StirMark(IMAGE I, OPTIONS o, NYQUIST n, FILE *stream, int use_jpeg)
{
    PROCESS_INFO info;
    
    if (I.img == NULL || stream == NULL) exit(1);

    info.resampling = &n;
    info.param = &o;
    info.sI = I;
    info.nRec = DEFAULT_RECONSTRUCTER;
    info.nTra = TRA_DISTORTION;
    info.nSam = SAM_POINT;
    info.mirrorBorders = 1;

    ApplyTransformation(&info);
	if (use_jpeg)
	{
		ImageWriteJPEG(info.dI, stream, o.jpeg_quality);
	}
	else
	{
		ImageWritePPM(info.dI, stream);
	}

    ImageClear(&(info.dI));
}
