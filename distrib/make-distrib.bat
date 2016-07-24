set POVER=2.4.3

set ZIP="C:\Program Files\7-Zip\7z.exe" a -tzip -mx=5
set VCBUILD="C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\WDExpress.exe"

cd /d %~dp0

rem Binaries
rmdir /S /Q "PngOptimizer-%POVER%"
mkdir PngOptimizer-%POVER%
chdir PngOptimizer-%POVER%
mkdir x64
chdir x64
mkdir PngOptimizer
mkdir PngOptimizerCL
chdir ..
mkdir x86
chdir x86
mkdir PngOptimizer
mkdir PngOptimizerCL
chdir ..

%VCBUILD% ../../Projects/PngOptimizer/PngOptimizer.sln       /build "Release|x64"   || goto Fail
%VCBUILD% ../../Projects/PngOptimizer/PngOptimizer.sln       /build "Release|Win32" || goto Fail
%VCBUILD% ../../Projects/PngOptimizerCL/PngOptimizerCL.sln   /build "Release|x64"   || goto Fail
%VCBUILD% ../../Projects/PngOptimizerCL/PngOptimizerCL.sln   /build "Release|Win32" || goto Fail

copy "../../Projects/PngOptimizer/x64-Release\PngOptimizer.exe"  "x64/PngOptimizer/" || goto Fail
copy "../../Projects/PngOptimizer\Readme.txt"                    "x64/PngOptimizer/" || goto Fail
copy "../../Projects/PngOptimizer\License.txt"                   "x64/PngOptimizer/" || goto Fail
copy "../../Projects/PngOptimizer\Changelog.txt"                 "x64/PngOptimizer/" || goto Fail

copy "../../Projects/PngOptimizer/Win32-Release\PngOptimizer.exe"  "x86/PngOptimizer/" || goto Fail
copy "x64/PngOptimizer\*.txt" "x86/PngOptimizer/"  || goto Fail

copy "../../Projects/PngOptimizerCL/x64-Release\PngOptimizerCL.exe"  "x64/PngOptimizerCL/" || goto Fail
copy "../../Projects/PngOptimizerCL\Readme.txt"                      "x64/PngOptimizerCL/" || goto Fail
copy "../../Projects/PngOptimizerCL\License.txt"                     "x64/PngOptimizerCL/" || goto Fail
copy "../../Projects/PngOptimizerCL\Changelog.txt"                   "x64/PngOptimizerCL/" || goto Fail

copy "../../Projects/PngOptimizerCL/Win32-Release\PngOptimizerCL.exe" "x86/PngOptimizerCL/" || goto Fail
copy "x64/PngOptimizerCL\*.txt" "x86/PngOptimizerCL/"  || goto Fail

rem Archives
del *.zip

cd x64
%ZIP% "../PngOptimizer-%POVER%-x64.zip"   "PngOptimizer"
%ZIP% "../PngOptimizerCL-%POVER%-x64.zip" "PngOptimizerCL"
cd ..

cd x86
%ZIP% "../PngOptimizer-%POVER%-x86.zip"   "PngOptimizer"
%ZIP% "../PngOptimizerCL-%POVER%-x86.zip" "PngOptimizerCL"
cd ..
cd ..

@color A
@echo =
@echo =   SUCCESS
@echo =
@goto Exit

:Fail
@color C
@echo =
@echo =   FAILURE
@echo =
@goto Exit

:Exit
@pause
@color
