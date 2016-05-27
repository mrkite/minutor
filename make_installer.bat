"c:\Program Files (x86)\WiX Toolset v3.10\bin\candle.exe" MyWixUI_InstallDir.wxs
"c:\Program Files (x86)\WiX Toolset v3.10\bin\candle.exe" minutor.wxs
"c:\Program Files (x86)\WiX Toolset v3.10\bin\light.exe" -ext WixUIExtension -o minutor.msi minutor.wixobj MyWixUI_InstallDir.wixobj
@pause
