/*----------------------------------------------------------------------------
 * StirMark -- Errors and messages notification functions
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
 * $Header: /StirMark/error.c 7     7/04/99 11:47 Fapp2 $
 *----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include "stirmark.h"
#include "error.h"


#ifdef __STDC__
void ERROR(char *format, ...)
{
    va_list args;
    va_start(args, format);
#else /*__STDC__*/
void ERROR(va_alist) va_dcl
{
    va_list args;
    char* format;

    va_start(args);
    format = va_arg(args, char*);
#endif /*__STDC__*/

    fprintf(stderr, "\n[ERROR] ");
    (void) vfprintf(stderr, format, args);
    fputc('\n', stderr);
    va_end(args);
    exit(-1);
}

#ifdef __STDC__
void MESSAGE(char* format, ...)
{
    va_list args;
    va_start(args, format);
#else /*__STDC__*/
void MESSAGE(va_alist)
    va_dcl
{
    va_list args;
    char* format;
    va_start(args);
    format = va_arg(args, char*);
#endif /*__STDC__*/

#ifdef VERBOSE
    (void) vfprintf(stderr, format, args);
    fputc('\n', stderr);
#endif
    va_end(args);
}
