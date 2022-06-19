@echo off

set opts=-FC -GR- -EHa- -nologo -Zi
set code=%cd%
pushd Q:\build\
cl %opts% %code%\Q:\build\ -FeQ:\build\
popd
