regsvr32 /u "C:\Windows\System32\KSeshIME.dll"
copy "x64\Debug\KSeshKeyboardWindows.dll" "C:\Windows\System32\"
icacls "C:\Windows\System32\KSeshIME.dll" /grant "ALL APPLICATION PACKAGES":(RX)
regsvr32 "C:\Windows\System32\KSeshIME.dll"
taskkill /F /IM ctfmon.exe
