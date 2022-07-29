REM Watermark each sample image with different parameters
REM The parameters can be tuned such that PSNR(watermarked_image)
REM is greater than 38dB.

SET LOGFILE   ="Watermarking log.txt"
SET WATERMARK =my_watermarking_prog.exe

%WATERMARK% param_baboon Samples/baboon.ppm Watermarked/baboon.ppm >> %LOGFILE%
%WATERMARK% param_bear Samples/bear.ppm Watermarked/bear.ppm >> %LOGFILE%
%WATERMARK% param_lena Samples/lean.ppm Watermarked/lena.ppm >> %LOGFILE%
%WATERMARK% param_skyline_arch Samples/skyline_arch.ppm Watermarked/skyline_arch.ppm >> %LOGFILE%
%WATERMARK% param_watch Samples/watch.ppm Watermarked/watch_w.ppm >> %LOGFILE%
%WATERMARK% param_fishin_boat Samples/fishing_boat.pgm Watermarked/fishing_boat.pgm >> %LOGFILE%

REM Convert the watermarked images to PPM or PGM if necessary