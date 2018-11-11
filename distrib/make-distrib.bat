set POVER=2.6~beta.2

set ZIP="C:\Program Files\7-Zip\7z.exe" a -tzip -mx=5
set VCBUILD="C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv"

:: Save current directory and change it to the directory of this bat
setlocal
cd /d %~dp0

:: Binaries
mkdir download
rmdir /S /Q "zip"
mkdir zip
chdir zip
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

%VCBUILD% ../../projects/pngoptimizer/PngOptimizer.sln       /build "Release|x64"   || goto Fail
%VCBUILD% ../../projects/pngoptimizer/PngOptimizer.sln       /build "Release|Win32" || goto Fail
%VCBUILD% ../../projects/pngoptimizercl/PngOptimizerCL.sln   /build "Release|x64"   || goto Fail
%VCBUILD% ../../projects/pngoptimizercl/PngOptimizerCL.sln   /build "Release|Win32" || goto Fail

copy "../../projects/pngoptimizer/x64-Release\PngOptimizer.exe"  "x64/PngOptimizer/" || goto Fail
copy "../../projects/pngoptimizer\Readme.txt"                    "x64/PngOptimizer/" || goto Fail
copy "../../projects/pngoptimizer\License.txt"                   "x64/PngOptimizer/" || goto Fail
copy "../../projects/pngoptimizer\Changelog.txt"                 "x64/PngOptimizer/" || goto Fail

copy "../../projects/pngoptimizer/Win32-Release\PngOptimizer.exe"  "x86/PngOptimizer/" || goto Fail
copy "x64/PngOptimizer\*.txt" "x86/PngOptimizer/"  || goto Fail

copy "../../projects/pngoptimizercl/x64-Release\PngOptimizerCL.exe"  "x64/PngOptimizerCL/" || goto Fail
copy "../../projects/pngoptimizercl\Readme.txt"                      "x64/PngOptimizerCL/" || goto Fail
copy "../../projects/pngoptimizercl\License.txt"                     "x64/PngOptimizerCL/" || goto Fail
copy "../../projects/pngoptimizercl\Changelog.txt"                   "x64/PngOptimizerCL/" || goto Fail

copy "../../projects/pngoptimizercl/Win32-Release\PngOptimizerCL.exe" "x86/PngOptimizerCL/" || goto Fail
copy "x64/PngOptimizerCL\*.txt" "x86/PngOptimizerCL/"  || goto Fail

:: Archives
del *.zip

cd x64
%ZIP% "../../download/pngoptimizer-%POVER%-win-x64.zip"   "PngOptimizer"
%ZIP% "../../download/pngoptimizercl-%POVER%-win-x64.zip" "PngOptimizerCL"
cd ..

cd x86
%ZIP% "../../download/pngoptimizer-%POVER%-win-x86.zip"   "PngOptimizer"
%ZIP% "../../download/pngoptimizercl-%POVER%-win-x86.zip" "PngOptimizerCL"
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
