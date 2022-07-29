REM Some sample command line showing the use of the PS/PL
REM options. The use of MD5 at the end it to check that
REM sample_S+.ppm and sample_S+L+.ppm aer indeed the same
REM and all the others are different.
REM I used MD5 because I don't have 'diff' on NT ;-)
REM Use anything you like.
REM
REM $Header: /StirMark/Test/test.bat 3     7/04/99 11:49 Fapp2 $

stirmark -NOJPEG sample.ppm sample_stir.ppm

stirmark -NOJPEG -PSsample.param_S sample.ppm sample_S.ppm
stirmark -NOJPEG -PS+sample.param_S+ sample.ppm sample_S+.ppm

stirmark -NOJPEG -PLsample.param_S sample.ppm sample_SL.ppm
stirmark -NOJPEG -PLsample.param_S+ sample.ppm sample_S+L.ppm

stirmark -NOJPEG -PL+sample.param_S sample.ppm sample_SL+.ppm
stirmark -NOJPEG -PL+sample.param_S+ sample.ppm sample_S+L+.ppm

md5 sample_stir.ppm
md5 sample_S.ppm
md5 sample_S+.ppm
md5 sample_SL.ppm
md5 sample_S+L.ppm
md5 sample_S+L+.ppm