#pragma once

class FileLogger {
public:
  FileLogger() {
    int64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    fFilePath = std::format("C:\\Users\\User\\AppData\\Local\\Temp\\SampleIME_{}.log", timestamp);
  }

  void println(std::string const& s) {
#if defined(_DEBUG)
    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, fFilePath.c_str(), "a+b");
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
  std::string fFilePath;
};
