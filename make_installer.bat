"c:\Program Files (x86)\Windows Installer XML v3.5\bin\candle.exe" MyWixUI_InstallDir.wxs
"c:\Program Files (x86)\Windows Installer XML v3.5\bin\candle.exe" minutor.wxs
"c:\Program Files (x86)\Windows Installer XML v3.5\bin\light.exe" -ext WixUIExtension -o minutor.msi minutor.wixobj MyWixUI_InstallDir.wixobj
@pause
