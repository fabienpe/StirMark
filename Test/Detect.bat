SET LOGFILE="Detection log.txt"
SET DETECT =my_detection_prog.exe
SET PARAM  =my_parameters

IF EXIST %LOGFILE% DEL %LOGFILE%
FOR %%V IN (Attacked/*.*) DO %DETECT% %PARAM% Attacked/%%V >> %LOGFILE%

REM $Header: /StirMark/Test/Detect.bat 3     7/04/99 11:49 Fapp2 $