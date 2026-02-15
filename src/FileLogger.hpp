#pragma once

class FileLogger {
public:
  FileLogger() {
    fTimestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  }

  void println(std::string const& s) {
#if defined(_DEBUG)
    WCHAR tempPath[MAX_PATH];
    DWORD length = GetTempPathW(MAX_PATH, tempPath);
    if (length <= 0) {
      return;
    }
    auto filePath = std::format(L"{}\\KSeshIME_{}.log", tempPath, fTimestamp);
    FILE* fp = nullptr;
    errno_t err = _wfopen_s(&fp, filePath.c_str(), L"a+b");
    if (!fp) {
      return;
    }
    defer{
      fclose(fp);
    };
    fprintf(fp, "%s\n", s.c_str());
#endif
  }

  static void Println(std::string const& s) {
    static std::unique_ptr<FileLogger> const sFile = std::make_unique<FileLogger>();
    sFile->println(s);
  }

private:
  int64_t fTimestamp;
};
