copy "x64\Debug\KSeshKeyboardWindows.dll" "C:\Windows\System32\"
regsvr32 /u "C:\Windows\System32\KSeshKeyboardWindows.dll"
icacls "C:\Windows\System32\KSeshKeyboardWindows.dll" /grant "ALL APPLICATION PACKAGES":(RX)
taskkill /F /IM ctfmon.exe
regsvr32 "C:\Windows\System32\KSeshKeyboardWindows.dll"
