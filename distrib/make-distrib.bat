set POVER=2.5

set ZIP="C:\Program Files\7-Zip\7z.exe" a -tzip -mx=5
set VCBUILD="C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv"

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

%VCBUILD% ../../projects/PngOptimizer/PngOptimizer.sln       /build "Release|x64"   || goto Fail
%VCBUILD% ../../projects/PngOptimizer/PngOptimizer.sln       /build "Release|Win32" || goto Fail
%VCBUILD% ../../projects/PngOptimizerCL/PngOptimizerCL.sln   /build "Release|x64"   || goto Fail
%VCBUILD% ../../projects/PngOptimizerCL/PngOptimizerCL.sln   /build "Release|Win32" || goto Fail

copy "../../projects/PngOptimizer/x64-Release\PngOptimizer.exe"  "x64/PngOptimizer/" || goto Fail
copy "../../projects/PngOptimizer\Readme.txt"                    "x64/PngOptimizer/" || goto Fail
copy "../../projects/PngOptimizer\License.txt"                   "x64/PngOptimizer/" || goto Fail
copy "../../projects/PngOptimizer\Changelog.txt"                 "x64/PngOptimizer/" || goto Fail

copy "../../projects/PngOptimizer/Win32-Release\PngOptimizer.exe"  "x86/PngOptimizer/" || goto Fail
copy "x64/PngOptimizer\*.txt" "x86/PngOptimizer/"  || goto Fail

copy "../../projects/PngOptimizerCL/x64-Release\PngOptimizerCL.exe"  "x64/PngOptimizerCL/" || goto Fail
copy "../../projects/PngOptimizerCL\Readme.txt"                      "x64/PngOptimizerCL/" || goto Fail
copy "../../projects/PngOptimizerCL\License.txt"                     "x64/PngOptimizerCL/" || goto Fail
copy "../../projects/PngOptimizerCL\Changelog.txt"                   "x64/PngOptimizerCL/" || goto Fail

copy "../../projects/PngOptimizerCL/Win32-Release\PngOptimizerCL.exe" "x86/PngOptimizerCL/" || goto Fail
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
