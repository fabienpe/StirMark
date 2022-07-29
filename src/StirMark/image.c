/*----------------------------------------------------------------------------
 * StirMark -- Graphic file handling and memory routines
 *
 * Copyright (c) 1997, 1999 by Fabien A. P. Petitcolas and Markus G. Kuhn
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
 * $Header: /StirMark/image.c 19    7/04/99 11:25 Fapp2 $
 *----------------------------------------------------------------------------
 */
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>

#include "error.h"
#include "stirmark.h"
#include "../JPEGLib/jpeglib.h"


/*----------------------------------------------------------------------------
 */
static void _ReplaceSpace(char *pszFileName)
{
    char *pc;

    while ((pc = strrchr(pszFileName, ' ')) != NULL)
        *pc = '_';
}


/*----------------------------------------------------------------------------
 */
void *MemoryAlloc(size_t n)
{
    void *p = NULL;

    if ((p = malloc(n)) == NULL)
    {
        fprintf(stderr, "Sorry, not enough memory available!\n");
        exit(1);
    }
    return p;
}    


/*----------------------------------------------------------------------------
 * skip whitespace and comments in PGM/PPM headers
 */
static void _skip_white(FILE *f)
{
    int c;
    
    if (f == NULL) return;
    
    do
    {
        while (isspace(c = getc(f)));
        if (c == '#')
            while ((c = getc(f)) != '\n' && c != EOF);
            else
            {
                ungetc(c, f);
                return;
            }
    } while (c != EOF);
    return;
}


/*----------------------------------------------------------------------------
 */
void ImageReadPPM(IMAGE *I, FILE *fin)
{
    char magic[10];
    int i, v, vmax;
    
    if (I == NULL || fin == NULL) exit(1);
    
    fgets(magic, 10, fin);
    if (magic[0] != 'P' || !isdigit(magic[1]) || magic[2] != '\n')
    {
        fprintf(stderr, "Unsupported input file type!\n");
        exit(1);
    }
    
    _skip_white(fin);
    fscanf(fin, "%d", &(I->size.x));
    
    _skip_white(fin);
    fscanf(fin, "%d", &(I->size.y));
    
    _skip_white(fin);
    fscanf(fin, "%d", &vmax);
    
    I->max = (unsigned char)vmax;

    getc(fin);
    if (I->max > 255 || I->max <= 0 || I->size.x <= 1 || I->size.y <= 1)
    {
        fprintf(stderr, "Unsupported value range!\n");
        exit(1);
    }
  
    switch (magic[1])
    {
    case '2': /* PGM ASCII */
    case '5': /* PGM binary */
        I->depth = 1; /* up to 8 bit/pixel */
        break;
    case '3': /* PPM ASCII */
    case '6': /* PPM binary */
        I->depth = 3; /* up to 24 bit/pixel */
        break;
    default:
        fprintf(stderr, "Unsupported input file type 'P%c'!\n", magic[1]);
        exit(1);
    }
    
    I->img = (unsigned char *) MemoryAlloc(sizeof(unsigned char) *
           I->size.x * I->size.y * I->depth);
    
    switch (magic[1])
    {
    case '2': /* PGM ASCII */
    case '3': /* PPM ASCII */
        for (i = 0; i < I->size.x * I->size.y * I->depth; i++)
        {
            _skip_white(fin);
            fscanf(fin, "%d", &v);
            if (v < 0 || v > I->max)
            {
                fprintf(stderr, "Out of range value!\n");
                exit(1);
            }
            (I->img)[i] = (unsigned char)v;
        }
        break;
    case '5': /* PGM binary */
    case '6': /* PPM binary */
        fread(I->img, I->size.x * I->depth, I->size.y, fin);
        break;
    }
    
    if (ferror(fin))
    {
        perror("Error occured while reading input file");
        exit(1);
    }
    
    if (feof(fin))
    {
        fprintf(stderr, "Unexpected end of input file!\n");
        exit(1);
    }
}


/*----------------------------------------------------------------------------
 */
void ImageNew(IMAGE *I, int width, int height, int depth, unsigned char max)
{
    size_t n;

    if (I == NULL) exit(1);
    
    I->size.x = width;
    I->size.y = height;
    I->depth = depth;
    I->max = max;
    n = sizeof(unsigned char) * I->size.x * I->size.y * I->depth;
    I->img = (unsigned char *) MemoryAlloc(n);
    memset(I->img, 0, n);
}


/*----------------------------------------------------------------------------
 */
void ImageDuplicate(IMAGE *dI, const IMAGE *sI)
{
    size_t n;
    ImageNew(dI, sI->size.x, sI->size.y, sI->depth, sI->max);
    n = sizeof(unsigned char) * sI->size.x * sI->size.y * sI->depth;
    memcpy(dI->img, sI->img, n);
}


/*----------------------------------------------------------------------------
 */
void ImageWritePPM(IMAGE I, FILE *fout)
{
    size_t n;
    
    if (I.img == NULL || fout == NULL) exit(1);

    n = I.size.x * I.size.y * I.depth;
    fprintf(fout, "P%d\n%d %d\n%d\n", I.depth == 1 ? 5 : 6, I.size.x, I.size.y, I.max);
    fwrite(I.img, sizeof(unsigned char), n, fout);
}


/*----------------------------------------------------------------------------
 * JPEG error handler
 * Based on the "example.c" file provided with the JPEG library disptribution
 * package
 */
struct error_mgr {
  struct jpeg_error_mgr pub;  /* "public" fields */
  jmp_buf setjmp_buffer;      /* for return to caller */
};

typedef struct error_mgr * error_ptr;


METHODDEF(void) error_exit (j_common_ptr cinfo)
{
    error_ptr err = (error_ptr) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(err->setjmp_buffer, 1);
}


/*----------------------------------------------------------------------------
 * Read a JPEG image
 * Based on the "example.c" file provided with the JPEG library disptribution
 * package
 */
void ImageReadJPEG(IMAGE *pI, FILE *infile)
{
    struct jpeg_decompress_struct cinfo;  /* decompression parameters and */
                                          /* pointers to working space    */
    struct error_mgr jerr;  /* JPEG error handler   */
                            /* not really used here */

    JSAMPARRAY buffer;    /* Output row buffer */
    int row_stride;  /* physical row width in output buffer */

    assert(infile != NULL);
    
    /* Allocate and initialize JPEG decompression object */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = error_exit;
    
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        return;
    }
    jpeg_create_decompress(&cinfo);
    
    /* Specify data source */
    jpeg_stdio_src(&cinfo, infile);
    
    /* Read file parameters with jpeg_read_header() */
    (void) jpeg_read_header(&cinfo, TRUE);
    
    /* Start decompressor */
    (void) jpeg_start_decompress(&cinfo);

    
    ImageNew(pI, cinfo.output_width, cinfo.output_height, 
             cinfo.output_components, 255); 
    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height)
    {
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(&(pI->img[(cinfo.output_scanline - 1) * row_stride]), 
               buffer[0], row_stride);
    }
    (void) jpeg_finish_decompress(&cinfo);

    /* Release JPEG decompression object */
    jpeg_destroy_decompress(&cinfo);
}


/*----------------------------------------------------------------------------
 * Based on the "example.c" file provided with the JPEG library disptribution
 * package
 */
void ImageWriteJPEG(IMAGE I, FILE *outfile, int quality)
{
    struct jpeg_compress_struct cinfo;  /* JPEG compression parameters and */
                                        /* pointers to working space       */
    struct jpeg_error_mgr jerr;         /* JPEG error handler              */
                                        /* not really used here            */
    JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s]          */
    int row_stride;             /* physical row width in image buffer */
    
    /* Allocate and initialize JPEG compression object */
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    
    /* Specify data destination (eg, a file) */
    jpeg_stdio_dest(&cinfo, outfile);
    
    /* Set parameters for compression */
    cinfo.image_width = I.size.x;      /* image size in pixels */
    cinfo.image_height = I.size.y;
    cinfo.input_components = I.depth;  /* # of color components per pixel */

    /* colorspace of input image */
    if (I.depth == 1)
        cinfo.in_color_space = JCS_GRAYSCALE;
    else if (I.depth == 3)
        cinfo.in_color_space = JCS_RGB;
    else
        ERROR("ImageWriteJPEG: color depth not supported");
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
    
    /* Compression */
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = I.size.x * I.depth;    /* JSAMPLEs per row in image_buffer */
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = & I.img[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    jpeg_finish_compress(&cinfo);

    /* Release compressed object and file */
    jpeg_destroy_compress(&cinfo);
}


/*----------------------------------------------------------------------------
 */
void ImageClear(IMAGE *I)
{
    if (I == NULL) exit(1);

    I->depth = 0;
    free(I->img);
    I->img = NULL;
    I->max = 0;
    I->size.x = 0;
    I->size.y = 0;
}


/*----------------------------------------------------------------------------
 */
void ImageSavePPM(IMAGE I, char *strzFileName)
{
    FILE *stream = NULL;
	char strzFullFileName[MAX_FILENAME_LENGTH + 4]; /* With extension */

    if (I.img == NULL || strzFileName == NULL) exit(1);

	strcat(strncpy(strzFullFileName, strzFileName, MAX_FILENAME_LENGTH), 
		I.depth == 1 ? ".pgm" : ".ppm");
    _ReplaceSpace(strzFullFileName);
    stream = fopen(strzFullFileName, "wb");
    if (stream == NULL)
        ERROR("ImageSavePPM: cannot open output file %s", strzFullFileName);
    ImageWritePPM(I, stream);
    fclose(stream);
}

/*----------------------------------------------------------------------------
 */
/*
void ImageSaveGIF(IMAGE I, char *strzFilename)
{
    FILE *stream = NULL;

    if (I.img == NULL || strzFilename == NULL) exit(1);

    _ReplaceSpace(strzFilename);
    stream = fopen(strzFilename, "wb");
    if (stream == NULL)
        ERROR("ImageSaveGIF: cannot open output file");
    ImageWriteGIF(I, stream);
    fclose(stream);
}
*/


/*----------------------------------------------------------------------------
 */
void ImageSaveJPEG(IMAGE I, char *strzFileName, int quality)
{
    FILE *stream = NULL;
	char strzFullFileName[MAX_FILENAME_LENGTH + 4]; /* With extension */

    if (I.img == NULL || strzFileName == NULL) exit(1);

	strcat(strncpy(strzFullFileName, strzFileName, MAX_FILENAME_LENGTH),
           ".jpg");
    _ReplaceSpace(strzFullFileName);
    stream = fopen(strzFullFileName, "wb");
    if (stream == NULL)
        ERROR("ImageSaveJPEG: cannot open output file %s", strzFullFileName);
    ImageWriteJPEG(I, stream, quality);
    fclose(stream);
}


/*----------------------------------------------------------------------------
 * Very basic autodetect file format based on the extension only
 * (part of the string actually)
 */
void ImageRead(IMAGE *pI, char *strzFilename)
{
    char *p;
    FILE *stream = NULL;

    stream = fopen(strzFilename, "rb");
    if (stream == NULL)
        ERROR("ImageRead: could not read file %s", strzFilename);
    if ((((p = strstr(strzFilename, ".ppm")) != NULL) ||/* Portable pixel map*/
         ((p = strstr(strzFilename, ".pgm")) != NULL))  /* Portable graymap  */
         &&  (*(p + 4) == 0))
        ImageReadPPM(pI, stream);
    else if ((((p = strstr(strzFilename, ".jpg")) != NULL) && /* jpeg */
              (*(p + 4) == 0)) ||
             (((p = strstr(strzFilename, ".jpeg")) != NULL) &&
              (*(p + 5) == 0)))
        ImageReadJPEG(pI, stream);
    else
        ERROR("ImageRead: unrecognised file format");
}
