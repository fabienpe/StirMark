/*----------------------------------------------------------------------------
 * StirMark -- Experimental watermark resilience benchmark
 *
 * Markus G. Kuhn <mkuhn@acm.org>, University of Cambridge
 * Fabien A. P. Petitcolas <fapp2@cl.cam.ac.uk>, University of Cambridge
 *
 * History: Version 1.0, 1997-11-10 by Markus G. Kuhn
 *             This is the initial version of StirMark tuned
 *             after various experiments on some existing
 *             steganographic algorithms.
 *
 *          Version 1.1, 1998-01-30 by Fabien A. P. Petitcolas
 *             Inverse StirMark added: anew option allows you
 *             to save the parameters used during the distortion
 *             and to inverse the process later on.
 *
 *          Version 1.2, 1998-03-11 by Fabien A. P. Petitcolas
 *             Add a ramdomise option: the interpolation process
 *             is randomised by changing slightly the coordinates
 *             of the points using the _randomise() fuction.
 *
 *          Version 1.3, 1998-05-19 by Fabien A. P. Petitcolas
 *             Add a global bending (x and y) to the picture.
 *             In fact a slight punch effect.
 *
 *          Version 2.0, 1998-06-15 by Fabien A. P. Petitcolas
 *             Add the simple transformation functions and
 *             filtering routines for the `benchmark'.
 *
 *          Version 2.1, 1998-08-15 by Fabien A. P. Petitcolas
 *             Add JPEG support using the library of the
 *             Independent JPEG Group.
 *
 *          Version 2.2, 1998-09-17 by Fabien A. P. Petitcolas
 *             Minor corrections and bug fixes.
 *
 *          Version 2.3, 1998-19-10 by Fabien A. P. Petitcolas
 *             Improved the speed of StirMark by changing the
 *             resampling method. Now use code by Neil Dodgson,
 *             Graphics Group, University of Cambridge. The
 *             speed has been increased by a factor 8 to 9.
 *             Most of the old StirMark code has been re-organised
 *             and clarified.
 *
 *          Version 3.0, 1999-20-01 by Fabien A. P. Petitcolas
 *             New transformations: row and columns removal,
 *             shrearing in X and Y direction and sharpening.
 *             Color quantisation support based on Jef Poskanzer's
 *             code. Laplacian based removal attack using Richard
 *             Barnett's code. Quality measure functions: SNR and
 *             PSNR. JPEG support for input image. Autodection of
 *             the input format based on file extension (pgm, ppm,
 *             jpg or jpeg).
 *
 *          Version 3.1, 1999-04-07 by Fabien A. P. Petitcolas
 *             Minor improvement: -NOJPEG prevent JPEG compression
 *             after geometrical distortions.
 *             Possibility to select a category of tests only.
 *             New tests: general linear geometric distortions, change
 *             of aspect ration, different sizes of Median filtering,
 *             assymetric row and column removal,
 *             new default values for some existing tests.
 *             Option to save and reuse parameters of geometrical 
 *             distortions.
 *
 *          Version 3.2,
 *             Corrected sizes of Median filters
 *             /? /h -? -h
 *
 * StirMark applies a number of minor almost invisible distortions to
 * an image in order to test whether some watermark detector can still
 * find the embedded signal after this processing. The distortion simulate
 * a resampling process, i.e. they include a minor geometric transform,
 * linear interpolation, slightly non-identical transfer functions as
 * well as highly non-linear and non bi-linear geometrical distortions.
 * The distortions are adjusted to be not recognizeable for typical 
 * photographic source images by human viewers.
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
 * $Header: /StirMark/stirmark.c 22    11/08/99 19:28 Fapp2 $
 *----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "error.h"
#include "bench.h"
#include "stirmark.h"
#include "image.h"
#include "quality.h"

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

extern int cutoff;

/* From bench.c */
extern void StirMark(IMAGE I, OPTIONS o, NYQUIST n, FILE *stream, int use_jpeg);
extern void Benchmark(IMAGE I, NYQUIST n, char *strzBasename, int nTestSet);
extern int  LRAttack(IMAGE *pSImage, 
                     IMAGE *pDImage, 
                     double dStrength, 
                     double dJNDStrength, 
                     int nScale);

/*----------------------------------------------------------------------------
 */
static FILE *StreamOpen(const char *strzFileName, const char *strzMode)
{
    FILE *pStream;

    if ((strzFileName == NULL) || (strzMode == NULL))
        ERROR("StreamOpen: null parameter");

    pStream = fopen(strzFileName, strzMode);
    if (pStream == NULL)
        ERROR("StreamOpen: cannot open file '%s' (mode %s)",
              strzFileName, strzMode);

    return (pStream);
}

/*----------------------------------------------------------------------------
 */
void Usage(char *name)
{
    char usage[] =
        "StirMark %s\nUsage: %s [options] [<input file> [<output file>]]\n"
        "\t-h, -?, /h, /?\tfor this help message\n\n"
        "\t-b<float>\tbending factor (%.2f)\n"
        "\t-d<float>\tmaximum variation of a pixel value (%.2f)\n"
        "\t-i<float>\tmaximum distance a corner can move inwards (%.2f%%)\n"
        "\t-ll<n><float>\tLR strengths for scale 2^n\n"
        "\t-lj<n><float>\tJND LR strengths for scale 2^n\n"
        /*"\t-n<int>\t\tuse Nyquist interpolation up to specified distance (%d)\n"*/
        "\t-o<float>\tmaximum distance a corner can move outwards (%.2f%%)\n"
        "\t-PS<filename>\tsave main distortion parameters into file\n"
        "\t-PL<filename>\tload main distortion parameters from file\n"
        "\t-PS+<filename>\tsave all distortion parameters into file\n"
        "\t-PL+<filename>\tload all distortion parameters from file\n"
        "\t\t\tSame format as input\n"
        "\t-q<int>\t\tJPEG quality factor of output(%d)\n"
        "\t-NOJPEG\t\tno JPEG compression after geometric distortions\n"
        "\t-R<float>\trandomisation factor (%.2f)\n"
        "\t-s<int>\t\trandom number generator seed\n"
        "\t-t<float>\trelative modification of Nyquist filter threshold (%.2f)\n"
        "\n"
        "\t-PSNR<original>\tcompute the PSNR of the input file (original image required)\n"
        "\n"
        "\t-T<name>\tbenchmark: series of pre-defined simple transformations\n"
        "\t\t\tand distortions. All other options ignored expect: \n"
        "\t-S<int>\t\tset of transformations used for test (requires -T option):\n"
        "\t\t\tconvolution filters (%d), median filter (%d), FMLR (%d),\n"
        "\t\t\tJPEG compression (%d), colour quantisation (%d), scaling (%d),\n"
        "\t\t\tshearing (%d), aspect ratio (%d), general linear (%d), \n"
        "\t\t\trotation crop (%d), rotation crop scale (%d), cropping(%d), \n"
        "\t\t\tflip (%d), row and col removal (%d), geometric distortions (%d)\n\n";
    
    if (name == NULL) exit(1);    

    fprintf(stderr, usage, VERSION, name, BENDING_FACTOR, DEVIATION,
        INWARDS, OUTWARDS, JPEG_QUALITY, RANDOM_FACTOR, ROLLOFF,
        TEST_FILTERING, TEST_MEDIAN_FILTERING, TEST_FMLR,
        TEST_COMPRESSION, TEST_QUANTISE, TEST_SCALING,
        TEST_SHEAR, TEST_ASPECT_RATIO, TEST_GENERAL_LINEAR,
        TEST_ROTATION_CROP, TEST_ROTATION_SCALE, TEST_CROPPING,
        TEST_FLIP, TEST_ROW_COL_REMOVAL, TEST_GEOM_DISTORTIONS);

    exit(1);
}


/*----------------------------------------------------------------------------
 */
int main(int argc, char **argv)
{
    char basename[MAX_BASENAME_LENGTH + 1];
    char *pfin = NULL, *pfout = NULL, *pfpsnr = NULL;
    FILE *fin = stdin, *fout = stdout;
    double do_lr[6];    /* LR strengths */
    double do_jndlr[6]; /* JND LR strengths */
    double psnr;        /* Value of PSRN is computed */
    int do_test = 0;    /* Whether -T option is used */
    int use_jpeg = 1;   /* For -NOJPEG option */
    int nTestSet = TEST_ALL; /* For -S option */
    int i = 0, j = 0;
    IMAGE   I, oI;
    OPTIONS o;
    NYQUIST n;
    
    for (i = 0; i < 6; i++)
    {
        do_lr[i] = 0;     /* clear strength arrays */
        do_jndlr[i] = 0;
    }            

    /* Default parameters */
    o.inwards = INWARDS;
    o.outwards = OUTWARDS;
    o.relative_inwards = RELATIVE_INWARDS;
    o.relative_outwards = RELATIVE_OUTWARDS;
    o.random_factor = RANDOM_FACTOR;
    o.bending_factor = BENDING_FACTOR;
    o.deviation = DEVIATION;
    o.jpeg_quality = JPEG_QUALITY_STIR;
    o.parameters = DIST_OP_NEW;

    n.rolloff.x = n.rolloff.y = ROLLOFF;
    n.highpass.x = n.highpass.y = HIGHPASS;
    n.cutoff = CUTOFF;

    /* Seed the random-number generator with current time so that */
    /* the numbers will be different every time we run.           */
    srand((unsigned)time(NULL));

    /* Read command line arguments */
    for (i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '/' || argv[i][0] == '-') && 
            (argv[i][1] == '?' || argv[i][1] == 'h') &&
             argv[i][2] == 0)

            Usage(argv[0]);
        else if (argv[i][0] == '-')
        {
            for (j = 1; j > 0 && argv[i][j] != 0; j++)
            {
                switch (argv[i][j])
                {
                case 'i': /* inward parameter */
                    o.inwards = atof(argv[i] + j + 1);
                    o.relative_inwards = argv[i][strlen(argv[i])-1] == '%';
                    j = -1;
                    break;
                case 'o': /* outward parameter */
                    o.outwards = atof(argv[i] + j + 1);
                    o.relative_outwards = argv[i][strlen(argv[i])-1] == '%';
                    j = -1;
                    break;
                case 'd': /* luminance modulation */
                    o.deviation = atof(argv[i] + j + 1);
                    j = -1;
                    break;
                case 't': /* Nyquist filter threshold */
                    if (argv[i][j+1] == 'x')
                        n.rolloff.x = atof(argv[i] + j + 2);
                    else if (argv[i][j+1] == 'y')
                        n.rolloff.y = atof(argv[i] + j + 2);
                    else
                        n.rolloff.x = n.rolloff.y = atof(argv[i] + j + 1);
                    j = -1;
                    break;
                case 'R': /* randomisation factor */
                    o.random_factor = atof(argv[i] + j + 1);
                    j = -1;
                    break;
                case 'b': /* bending factor */
                    o.bending_factor =  atof(argv[i] + j + 1);
                    j = -1;
                    break;
                case 's': /* Seed */
                    srand(atoi(argv[i] + j + 1));
                    j = -1;
                    break;
                case 'S': /* Test set */
                    nTestSet = atoi(argv[i] + j + 1);
                    j = -1;
                    break;
                /*case 'n': * Nyquist distance *
                    * use true Nyquist sin(x)/x interpolation *
                    if (isdigit(argv[i][j+1]))
                    {
                        n.cutoff = atoi(argv[i] + j + 1);
                        j = -1;
                    }
                    break;*/
                case 'q':
                    o.jpeg_quality = atoi(argv[i] + j + 1);
                    j = -1;
                    break;
                case 'P':
                    /* Compute PSNR */
                    if (strncmp(argv[i] + j, "PSNR", 4) == 0)
                        pfpsnr = argv[i] + j + 4;

                    /* Save or load distortion parameters*/
                    else if (strncmp(argv[i] + j, "PS+", 3) == 0)
                    {
                        o.pszFileName = argv[i] + j + 3;
                        o.parameters = DIST_OP_SAVE_ALL;
                    }
                    else if (strncmp(argv[i] + j, "PL+", 3) == 0)
                    {
                        o.pszFileName = argv[i] + j + 3;
                        o.parameters = DIST_OP_LOAD_ALL;
                    }
                    else if (strncmp(argv[i] + j, "PS", 2) == 0)
                    {
                        o.pszFileName = argv[i] + j + 2;
                        o.parameters = DIST_OP_SAVE;
                    }
                    else if (strncmp(argv[i] + j, "PL", 2) == 0)
                    {
                        o.pszFileName = argv[i] + j + 2;
                        o.parameters = DIST_OP_LOAD;
                    }
                    else
                        ERROR("Unrecognised option -P??.");
                    j = -1;
                    break;
                case 'N':
                    /* PPM ouput when not in -T mode */
                    if (strncmp(argv[i] + j, "NOJPEG", 6) == 0)
                        use_jpeg = 0;
                    else
                        ERROR("Unrecognised option -N??.");
                    j = -1;
                    break;
                case 'l':
                    /* Parameters for the FMLR attack */
                    {
                    switch (argv[i][j+1])
                    {
                    case 'l':
                    {
                        switch (argv[i][j+2])
                        {
                        case '1': /* Scale 1 */
                            do_lr[0] = atof(argv[i] + j + 3);
                            break;
                        case '2': /* Scale 2 */
                            do_lr[1] = atof(argv[i] + j + 3);
                            break;
                        case '3': /* Scale 4 */
                            do_lr[2] = atof(argv[i] + j + 3);
                            break;
                        case '4': /* Scale 8 */
                            do_lr[3] = atof(argv[i] + j + 3);
                            break;
                        case '5': /* Scale 16 */
                            do_lr[4] = atof(argv[i] + j + 3);
                            break;
                        case '6': /* Scale 32 */
                            do_lr[5] = atof(argv[i] + j + 3);
                            break;                
                        default:
                            ERROR("Scale not supported for -ll option.");
                        }
                        j = -1;
                        break;
                    }
                    case 'j':
                    {
                        switch (argv[i][j+2])
                        {
                        case '1': /* Scale 1 */
                            do_jndlr[0] = atof(argv[i] + j + 3);
                            break;
                        case '2': /* Scale 2 */
                            do_jndlr[1] = atof(argv[i] + j + 3);
                            break;
                        case '3': /* Scale 4 */
                            do_jndlr[2] = atof(argv[i] + j + 3);

                            break;
                        case '4': /* Scale 8 */
                            do_jndlr[3] = atof(argv[i] + j + 3);
                            break;
                        case '5': /* Scale 16 */
                            do_jndlr[4] = atof(argv[i] + j + 3);
                            break;
                        case '6': /* Scale 32 */
                            do_jndlr[5] = atof(argv[i] + j + 3);
                            break;
                        default:
                            ERROR("Scale not supported for -lj option.");
                        }
                        j = -1;
                        break;
                    }
                    default:
                        ERROR("Unrecognised option -l??.");
                    }
                    break;
                    }
                case 'T':
                    do_test = 1;
                    strncpy(basename, argv[i] + j + 1,
                            min(strlen(argv[i] + j + 1), MAX_BASENAME_LENGTH));
                    basename[min(strlen(argv[i] + j + 1),
                             MAX_BASENAME_LENGTH)] = '\0';
                    if (strlen(basename) == 0)
                        ERROR("Empty base name.");
                    j = -1;
                    break;
                default:
                    ERROR("One of the options has not been recognised. "
                          "Try -? for help.");
                }
            }
        }
        else if (pfin == NULL)
            pfin = argv[i];
        else if (pfout == NULL)
            pfout = argv[i];
        else
            ERROR("One of the options has not been recognised. "
                  "Try -? for help.");
    }
    if ((nTestSet != TEST_ALL) && (do_test == 0))
        ERROR("The -S option must be used with -T option.");

    if (n.rolloff.x < 0) n.rolloff.x = -n.rolloff.x, n.highpass.x = 1;
    if (n.rolloff.y < 0) n.rolloff.y = -n.rolloff.y, n.highpass.y = 1;

    if (pfin)
        ImageRead(&I, pfin);
    else
        ImageReadPPM(&I, fin);
    if (pfout)
        fout = StreamOpen(pfout, "wb");

    if (pfpsnr)
    {
        /* Just compute sthe PSNR and exit */
        if (pfin == NULL)
            printf("Original: %s - Modified: stdin - ", pfpsnr);
        else
            printf("Original: %s - Modified: %s - ", pfpsnr, pfin);
        ImageRead(&oI, pfpsnr);
        if ((psnr = PSNR(oI, I)) <= MIN_QUALITY)
            printf("PSNR not meaningful\n");
        else
            printf("PSNR = %f\n", psnr);
        ImageClear(&oI);
    }
    else
    {
        if (do_test)
            Benchmark(I, n, basename, nTestSet);
        else /* Backward compatibility */
        {
            /* First the LR attack */
            for (i = 0; i < 6; i++)
                /* Nothing is done if stregths are 0 */
                LRAttack(&I, &I, do_lr[i], do_jndlr[i], (1 << i));

            /* Then the StZirMark geometrical distortions */
            StirMark(I, o, n, fout, use_jpeg);
        }
    }

    if (pfout)
        fclose(fout);

    ImageClear(&I);
    return (0);
}

