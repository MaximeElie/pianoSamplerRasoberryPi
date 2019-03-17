@echo off

git add *

set /P message="commit message : "

git commit -m "%message%"

git push

echo.
pause