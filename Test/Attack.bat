REM You should not need to modify this file.

IF EXIST "Attack Log.txt" DEL "Attack Log.txt"

REM    First Deleted existing Images directory
IF EXIST Attacked DEL /S /F Attacked

MD Attacked

REM StirMark create the files in current directory...
CD Attacked

REM Apply StirMark tests to all files in Watermarked directory
FOR %%V IN (../Watermarked/*.*) DO ../stirmark -T%%~nV ../%%V >> ../"Attack Log.txt"

CD ..

REM Note about the DOS command prompt
REM %%V stands for name of file
REM %%~nV is name of file without extension
REM For more details about the DOS batch language, see:
REM <http://gearbox.maem.umr.edu/~batch/>

REM $Header: /StirMark/Test/Attack.bat 3     7/04/99 11:48 Fapp2 $