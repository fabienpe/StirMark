
------------
StirMark 3.1
------------

Fabien A. P. Petitcolas and Markus G. Kuhn
University of Cambridge, Computer Laboratory


Broad claims have been made about the `robustness' of various digital
watermarking method. Unfortunately the criteria as well as the
pictures used to demonstrate these claims vary from one system to the
other and recent attacks show that the robustness criteria used until
now are inadequate: JPEG compression, additive Gaussian noise, low
pass filtering rescaling and cropping have been addressed in most the
literature but specific distortions such as rotation have been rarely
addressed. In some cases the watermark is simply said to be ``robust
against common signal processing algorithms and geometric distortions
when used on some standard images.''

Most of the potential attacks detailed in:

   Martin Kutter and Fabien A. P. Petitcolas. A Fair Benchmark for
   Image Watermarking Systems. To be presented at the 11th Annual
   Symposium on Electronic Imaging, IS&T/SPIE, San Jose, USA, 23-29
   January 1999.

   <http://www.cl.cam.ac.uk/~fapp2/papers/ei99-benchmark/>

are actually implemented into this version of StirMark: given a
watermarked image, StirMark will apply these transformations with
various parameters. Then the output images can be tested with watermark
detection or extraction programs. The full process can be atomised
using a simple batch file.

For this benchmark we suggest to use pictures freely available for
research purpose and downloadable from:

    <http://www.cl.cam.ac.uk/~fapp2/watermarking/benchmark/
    image_database.html>

A result sheet for this benchmark applied to various copyright marking
algorithms can be download from:

    <http://www.cl.cam.ac.uk/~fapp2/watermarking/benchmark/>

---

The original StirMark (version 1.0) is still available from

   <http://www.cl.cam.ac.uk/~mgk25/stirmark/>

---

StirMark reads and writes arbitrary images in the PGM, PPM and JPEG
file formats. The selection is based on the extension of the file
only: .ppm, .pgm, .jpg or .jpeg



----------------------------
What's new with version 3.1?
----------------------------

New features in version 3.1:
- Possibility to select a category of tests only (e.g. rotations only).
- New tests: general linear geometric distortions, change of aspect
             ration, different sizes for median filtering, asymmetric
             row and column removal, new default values for some
             existing tests (especially. JPEG compression).
- Option to save and re-use parameters of geometrical distortions.

---

Version 3.0 adds:

- New transformations including row and columns removal, shearing in
  X and Y direction and sharpening.
- Color quantisation (used by GIF) support based on Jef Poskanzer's
  code.
- The Laplacian based removal attack using Richard Barnett's is now
  applied before the geometrical distortions.
- Basic quality measure functions: SNR and PSNR.
- Better JPEG support for input image including simple autodection of
  the input format based on file extension (pgm, ppm, jpg or jpeg).

---

Version 2.3 now uses the bi-quadratic resampling algorithm instead of
the Nyquist interpolation algorithm used in previous version. The
speed improvement is roughly a factor 8 to 9 for a 700x500 image. The
bi-quadratic algorithm gives a result similar to the bi-cubic in about
60% of the CPU time. It is described into:

   Neil Dodgson, "Quadratic Interpolation for Image Resampling," IEEE
   Transactions on Image Processing, vol. 6, no. 9, pp. 1322--1326,
   Sept. 1997.

---

Version 2.2 adds the following command line option to version 1.0
  -T<name> to perform a standard series of tests on a single image.
  -R<float> and -b<float> are used in the enhanced distortion process.
These options are described in details below.

Version 2.2 now uses the JPEG library of the Independent JPEG Group's
free JPEG.  For convenience this library is provided in the StirMark
package but the latest version can be downloaded from:

   <ftp://192.48.96.9/graphics/jpeg/>

So the default output now uses the JPEG format:

   stirmark test.ppm output.jpg

just applies the latest StirMark distortions to the image, including
JPEG compression at the end.



--------------------
Command line options
--------------------

You can specify an input and an output file name in the command
line. If you omit any of those, stdin and stdout are used instead
respectively. The output uses the JPEG format.

  -? To get help.

  -b<float> The number of pixel displacement allowed for the center of
                 the image. All the pixel are moved using a smooth
                 function. Pixels at the corners are not moved while
                 the pixel in the center of the image is moved the
                 most.  This is similar to a `punch/pinch' effect.

  -d<float> The maximum byte value by which any of the RGB values of
                  any pixel is allowed to deviate from the original
                  value (default 1.5).

  -i<float> The number of pixel distances that the corner of the
                 target image is allowed to be inside the original
                 image. If the number is followed by %, then the
                 maximum corner shift is defined as a percentage of
                 the image width or height, whichever is
                 smaller. Default is -i2.0%.

  -ll<n><float> This option is specific to the frequency mode
                 Laplacian removal attack which is applied before the
                 random geometric distortions. It is used to specify
                 the LR strengths for scale 2^n. For more details,
                 see: <http://www.cl.cam.ac.uk/~fapp2/steganography/
                 bibliography/074104.html>

  -lj<n><float> This option is also specific to the FMLR attack. It is
                 used to specify the JND LR strengths for scale
                 2^n. For more details, see:
                 <http://www.cl.cam.ac.uk/~fapp2/steganography/
                 bibliography/074104.html>

  -o<float> The number of pixel distances that the corner of the
                 target image is allowed to be outside the original
                 image (default 0.7). Sample values taken from outside
                 the original image are extrapolated. You can also
                 specify a percentage as with -i, but this is normally
                 not useful as -o values much higher than 1 cause
                 mostly useless extrapolations.

  -PS<filename> Save the main distortion parameters into a file. These
                 parameters include those specified from the command
                 line (or default if not specified) as well as other
                 random parameters. Useful is you want to apply the
                 same transformation to a sequence of frames in a
                 video. This avoid getting shaking effect.

  -PS+<filename> Save the main distortion parameters and all the
                 random numbers used the in the geometrical distortion
                 process. This creates very big files.

  -PL<filename> Load previously saved main random parameters using -PS
                 or -PS+ options.

  -PL+<filename> Load previously all previously saved parameters (using
                 -PS+ option). If -PL+ is used after -PS, StirMark
                 switches it back automatically to -PL.

  -q<int> JPEG quality factor used after geometric distortions.

  -NOJPEG Prevent StirMark from applying JPEG compression after the
                 geometrical distortions.

  -R<float> The fraction of pixel displacement allowed for any pixel.
                 During the distortion process, any pixel can be moved
                 randomly along both axes.

  -s<int> Seed value for the random number generator. If you do
                 systematic measurements with StirMark, then repeat
                 the runs with different -s values (e.g. from -s1 to
                 -s100), to get significant results regarding the
                 probability with which StirMark breaks your
                 steganographic technique.


  -PSNR<name> All options given above ignored in this case. Gives the
                 PSNR of the input file compare to the file whose name
                 is provided after the PSNR option:
                                          2
                                         N  max X
                                          i   i
                 PSNR = 10 log_10   ------------------
                                      N           2
                                     Sum (X  - X')
                                     i=1   i    i
           

  -T<name>    All options given above ignored in this case.
              The tests implemented by default into StirMark include:
              - rotation by a small angle and cropping: -2, -1, -0.75,
                -0.5, -0.25, 0.25, 0.5, 0.75, 1 and 2 degrees. This
                simulates in part what is obtain when scanning an
                image: it is impossible to align it perfectly.
              - rotation by a small angle followed by cropping and
                rescaling to keep the original size of the image.
              - rotation by large angle: 5, 10, 15, 30, 45, and 90
                degrees.
              - scaling by factors 0.5, 0.75, 0.9, 1.1, 1.5 and 2.
              - centered cropping: 1%, 2%, 5%, 10%, 15%, 20%, 25%, 50%
                and 75%
              - median filtering
              - Gaussian filtering  1  2  1
                                    2  4  2
                                    1  2  1
              - sharpening   0 -1  0
                            -1  5 -1
                             0 -1  0
              - horizontal flip
              - Frequency mode Laplacian removal attack
              - Colour quantisation (similar to GIF compression)
              - JPEG compression with quality factors: 90, 80, 70, 60,
                50, 40, 35, 30, 25, 20, 15 and 10. Most existing
                algorithms break down towards 50, but certain images
                (e.g., baboon) still look fine at 30. Most pictures
                have a very poor quality with factor 10 but some
                algorithm survive up to this point.
              - Symmetric and asymmetric shearing (X and/or Y
                direction): (0, 1), (0, 5) (1, 0), (5, 0), (1, 1) and
                (5, 5). The first component is the shift in X
                direction (% of width) and the second is the shift in
                the Y direction (% of height).
              - Symmetric and asymmetric line and column removal: (1,
                1), (1, 5), (5, 1), (5, 17), (17, 5). First component
                is the number of columns removed and the second
                component the number or rows.
              - General linear geometric transformation:
                |x'|   |a b| |x|
                |y'| = |c d| |y|
                with the following parameters:
                (a, b, c, d) = (1.010, 0.013, 0.009, 1.011)
                               (1.007, 0.010, 0.010, 1.012)
                               (1.013, 0.008, 0.011, 1.008)
              - `StirMark' random geometric distortions

              Output files are in JPEG and PPM formats, depending on
              the test.

              Other information about these tests, how to use them can
              be found on:

              <http://www.cl.cam.ac.uk/~fapp2/watermarking/benchmark/>

  - S<int>    To be use with -T option only. This option can be used to
              select a category of transformation (e.g. rotation
              only). Please use ``stirmark -?'' to get the list of
              category numbers available. These numbers are subject to
              change.



--------
Examples
--------

- stirmark -Tbase test.ppm

  will apply all the test describe above and create one output file
  per test with base name "base" and different suffixes and extensions
  depending on the transformation applied.

- stirmark -S34 -Tbase test.ppm

  will only apply category 34 of the tests. See ``stirmark -?'' for
  the list of available codes.

- stirmark -PSNRoriginal.ppm modified.jpg

  asks StirMark to compute the PSNR of modified relatively to the
  original.

- stirmark -NOJPEG -PS+sample.param sample.ppm sample_S+.ppm
  stirmark -NOJPEG -PL+sample.param sample.ppm sample_S+L+.ppm

  The first command applies geometrical distortions and save the
  parameters into sample.param. The second command loads these
  parameters and use them to apply the same geometric distortions. At
  the end sample_S+L+.ppm and sample_S+.ppm will be exactly the same.

- stirmark -i0 -o0 -d0 test.ppm output.jpg

  Does not apply any geometric distortions. Just JPEG compression.



--------------------------
About StirMark distortions
--------------------------

Although many watermarking systems survive basic manipulations -- that
is, manipulations that can be done easily with standard tools, such as
rotation, shearing, resampling, resizing and lossy compression -- they
do not cope with combinations of them and especially minor random
geometric distortions.

StirMark is a testing tool for watermarking systems that applies such
distortions to images. The image is slightly stretched, sheared,
shifted and/or rotated by an unnoticeable random amount. If A, B, C
and D are the corners of the image, a point M of the said image can be
expressed as M = a(b A + (1 - b)D) + (1 - a)(b B + (1 - b)C) where 0
<= a, b <= 1 are the coordinates of M relatively to the corners. The
distortion is done by moving the corners by a small random amount in
both directions. The new coordinates of M are given by the previous
formula, keeping (a, b) constant.

More distortions -- still unnoticeable -- can be applied to a
picture. A global `bending' is applied to the image: in addition to
the general bi-linear property explained previously a slight deviation
is applied to each pixel, which is greatest at the center of the
picture and almost null at the borders. On top of this a higher
frequency displacement of the form l.sin(Ox.x)sin(Oy.y) + n(x,y) --
where n is a random number -- is added.

All these distortions combined together with a mild JPEG compression
do not remove the watermark per se. They prevent the detector from
finding it. This suggests that the real problem is not so much
inserting the marks as recognizing them afterwards.

StirMark introduces a practically unnoticeable quality loss in the
image if it is applied only once. However after a few iterated
applications of the algorithm, the image degradation becomes soon very
noticeable. This makes StirMark (especially the -d option) also a
model for algorithms that can be applied to artificially degrade
images on replay to get rid of the lossless-copying property of
digital multimedia objects.

---

Suggestions for improving this software are always welcome. We are
also interested in robustness test results obtained by processing with
StirMark images that were marked with various available watermarking
tools.

Surprisingly, first experiences with various watermarking tools show
that the watermarks of *all* tools we have been able to locate until
today can be removed or made undetectable by StirMark. For more
information, please see:

   <http://www.cl.cam.ac.uk/~fapp2/watermarking/>

Still more information is provided in:

   Fabien A. P. Petitcolas, Ross J. Anderson and Markus
   G. Kuhn. Attacks on copyright marking systems. In David Aucsmith,
   Ed., second workshop on information hiding, in vol. 1525 of Lecture
   Notes in Computer Science Portland, Oregon, USA, 14--17 April,
   1998, pp. 218--238. ISBN 3-540-65386-4.
   
   <http://www.cl.cam.ac.uk/~fapp2/papers/ih98-attacks/>



--------------------------
Copyright and legal notice
--------------------------

StirMark is freely available under the GNU General Public Licence in
full portable ISO C source code from

   <http://www.cl.cam.ac.uk/~fapp2/watermarking/stirmark/>

Users should read the "Copyright" file provided in this package for
copyright information about code and libraries used in StirMark.

If you use StirMark for your research, please cite:

    Fabien A. P. Petitcolas, Ross J. Anderson, Markus G. Kuhn. Attacks
    on copyright marking systems, in David Aucsmith (Ed), Information
    Hiding, Second International Workshop, IH'98, Portland, Oregon,
    USA, April 15--17, 1998, Proceedings, LNCS 1525, Springer-Verlag,
    ISBN 3-540-65386-4, pp. 219--239.

and 

    Martin Kutter and Fabien A. P. Petitcolas. A fair benchmark for
    image watermarking systems, To in E. Delp et al. (Eds), in
    vol. 3657, proceedings of Electronic Imaging '99, Security and
    Watermarking of Multimedia Contents, San Jose, CA, USA, 25--27
    January 1999. The International Society for Optical
    Engineering. To appear.

Using StirMark for any other purpose than research or evaluation of
copyright marking systems is prohibited, at least in Europe: "Member
States shall provide adequate legal protection against the
circumvention without authority of any effective technological
measures designed to protect any copyrights or any rights related to
copyright as provided by law or the sui generis right provided for in
chapter III of European Parliament and Council Directive 96/9/EC".



---------------
Acknowledgments
---------------

- Peter Meerwald <pmeerw@cosy.sbg.ac.at> for the `man' page.
- Martin Kutter <Martin.Kutter@lts.epfl.ch>, École Polytechnique
  Fédérale de Lausanne, Switzerland, for providing some sample images
  for version 2.2.
- This software is based in part on the work of the Independent JPEG
  Group
- Alan Day <day@ms.chttl.com.tw> for pointing out bugs into version
  2.2, build 24.
- Frank Hartung <hartung@nt.e-technik.uni-erlangen.de>,
  Friedrich-Alexander- Universität Erlangen-Nürnberg, Germany, who
  pointed out a bug into image.c
- Neil Dodgson <nad@cl.cam.ac.uk>, University of Cambridge, for
  providing the new resampling algorithms.
- Richard Barnett <rbarne@essex.ac.uk>, University of Essex, for the
  code of his Laplacian Removal attack.

- Gabriela O. Csurka <Gabriela.Csurka@cui.unige.ch> (Université de
  Genève), Thomas Fiebig <fiebig@hrzpub.tu-darmstadt.de> (Technische
  Universität Darmstadt), and Shelby Pereira
  <Shelby.Pereira@cui.unige.ch> for their valuable comments.


Fabien.
Cambridge, 7 April 1999.

-- 
Fabien A. P. Petitcolas, The Computer Laboratory,
University of Cambridge, New Museum Site, CB2 3QG, UK.

<http://www.cl.cam.ac.uk/~fapp2/>


$Header: /StirMark/README.txt 21    7/04/99 12:21 Fapp2 $