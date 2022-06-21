mkdir conan
cd conan
conan install ..
if NOT %errorlevel% == "0" (
conan profile update settings.compiler.version=16  default
conan install ..
)
cd ..\..
