curl -O https://d13lb3tujbc8s0.cloudfront.net/onlineinstallers/qt-unified-windows-x86-4.3.0-1-online.exe
mkdir Qt

qt-unified-windows-x86-4.3.0-1-online.exe ^
  --accept-licenses ^
  --default-answer ^
  --accept-obligations ^
  --root "%cd%\Qt" ^
  --confirm-command install ^
  --email mail@gmail.com ^
  --password Password ^
  qt.qt5.5152.win64_msvc2019_64
