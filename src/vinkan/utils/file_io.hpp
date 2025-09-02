#ifndef VINKAN_FILE_IO_HPP
#define VINKAN_FILE_IO_HPP

#include <fstream>
#include <vector>

namespace vinkan {

inline std::vector<char> readTextFile(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file " + filepath);
  }

  // tellg gives size since we're at the end with ios::ate
  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

}  // namespace vinkan

#endif
