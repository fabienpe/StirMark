
IF EXIST "PSNR Log.txt" DEL "PSNR Log.txt"

REM Assume that you've kept the same name for
REM original and wartermarked
FOR %%V IN (Samples/*.*) DO stirmark -PSNRSamples/%%V Watermarked/%%V >> "PSNR Log.txt"

REM $Header: /StirMark/Test/PSNR.bat 3     7/04/99 11:49 Fapp2 $