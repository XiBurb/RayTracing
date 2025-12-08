#ifndef IMAGEUTILS_HPP
#define IMAGEUTILS_HPP

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define CREATE_DIR(path) _mkdir(path)
#else
#define CREATE_DIR(path) mkdir(path, 0755)
#endif

class ImageUtils {
private:
    // Создание директории, если она не существует
    static bool createDirectory(const std::string& path) {
        struct stat info;
        if (stat(path.c_str(), &info) != 0) {
            // Директория не существует, создаём
            if (CREATE_DIR(path.c_str()) == 0) {
                std::cout << "Created directory: " << path << std::endl;
                return true;
            } else {
                std::cerr << "Failed to create directory: " << path << std::endl;
                return false;
            }
        } else if (info.st_mode & S_IFDIR) {
            // Директория уже существует
            return true;
        }
        return false;
    }

    // Получение текущего timestamp
    static std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
        ss << "-" << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    // Получение абсолютного пути
    static std::string getAbsolutePath(const std::string& relativePath) {
#ifdef _WIN32
        char fullPath[_MAX_PATH];
        if (_fullpath(fullPath, relativePath.c_str(), _MAX_PATH) != nullptr) {
            return std::string(fullPath);
        }
#else
        char fullPath[PATH_MAX];
            if (realpath(relativePath.c_str(), fullPath) != nullptr) {
                return std::string(fullPath);
            }
#endif
        return relativePath;
    }

public:
    // Сохранение с автоматическим созданием директории и timestamp
    static bool saveImage(const std::vector<unsigned char>& pixels,
                          int width, int height,
                          const std::string& directory = "output",
                          const std::string& prefix = "screenshot") {
        // Создаём директорию
        if (!createDirectory(directory)) {
            return false;
        }

        // Генерируем имена файлов с timestamp
        std::string timestamp = getTimestamp();
        std::string baseFilename = directory + "/" + prefix + "_" + timestamp;
        std::string ppmFile = baseFilename + ".ppm";
        std::string bmpFile = baseFilename + ".bmp";

        // Сохраняем оба формата
        bool ppmSuccess = savePPM(ppmFile, pixels, width, height);
        bool bmpSuccess = saveBMP(bmpFile, pixels, width, height);

        if (ppmSuccess || bmpSuccess) {
            std::cout << "\n=== Screenshot saved ===" << std::endl;
            if (ppmSuccess) {
                std::cout << "PPM: " << getAbsolutePath(ppmFile) << std::endl;
            }
            if (bmpSuccess) {
                std::cout << "BMP: " << getAbsolutePath(bmpFile) << std::endl;
            }
            std::cout << "========================\n" << std::endl;
        }

        return ppmSuccess || bmpSuccess;
    }

    static bool savePPM(const std::string& filename,
                        const std::vector<unsigned char>& pixels,
                        int width, int height) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }

        file << "P6\n" << width << " " << height << "\n255\n";
        file.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());

        file.close();
        return true;
    }

    static bool saveBMP(const std::string& filename,
                        const std::vector<unsigned char>& pixels,
                        int width, int height) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }

        int rowSize = ((width * 3 + 3) / 4) * 4;
        int imageSize = rowSize * height;
        int fileSize = 54 + imageSize;

        unsigned char header[54] = {
                'B', 'M',
                (unsigned char)(fileSize),
                (unsigned char)(fileSize >> 8),
                (unsigned char)(fileSize >> 16),
                (unsigned char)(fileSize >> 24),
                0, 0, 0, 0,
                54, 0, 0, 0,
                40, 0, 0, 0,
                (unsigned char)(width),
                (unsigned char)(width >> 8),
                (unsigned char)(width >> 16),
                (unsigned char)(width >> 24),
                (unsigned char)(height),
                (unsigned char)(height >> 8),
                (unsigned char)(height >> 16),
                (unsigned char)(height >> 24),
                1, 0,
                24, 0,
                0, 0, 0, 0,
                (unsigned char)(imageSize),
                (unsigned char)(imageSize >> 8),
                (unsigned char)(imageSize >> 16),
                (unsigned char)(imageSize >> 24),
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0
        };

        file.write(reinterpret_cast<char*>(header), 54);

        std::vector<unsigned char> padding(rowSize - width * 3, 0);
        for (int y = height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 3;
                file.put(pixels[idx + 2]);
                file.put(pixels[idx + 1]);
                file.put(pixels[idx + 0]);
            }
            file.write(reinterpret_cast<char*>(padding.data()), padding.size());
        }

        file.close();
        return true;
    }
};

#endif