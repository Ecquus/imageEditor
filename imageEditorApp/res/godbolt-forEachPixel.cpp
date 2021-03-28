#include <cstdint>
#include <functional>
#include <vector>

using QRgb = std::uint32_t;
using uint = unsigned int;

inline int qRed(QRgb rgb)
{ return ((rgb >> 16) & 0xff); }

inline int qGreen(QRgb rgb)
{ return ((rgb >> 8) & 0xff); }

inline int qBlue(QRgb rgb)
{ return (rgb & 0xff); }

inline int qAlpha(QRgb rgb)
{ return rgb >> 24; }

inline QRgb qRgba(int r, int g, int b, int a)
{ return ((a & 0xffu) << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu); }

const float KR{ 0.2126f };
const float KG{ 0.7152f };
const float KB{ 0.0722f };

inline auto toGray(QRgb pixel) -> QRgb
// void toGray(QRgb& pixel)
{
    const auto gray = int((((float(qRed(pixel))   * KR) +
                            (float(qGreen(pixel)) * KG) +
                            (float(qBlue(pixel))  * KB))));
    return qRgba(gray, gray, gray, qAlpha(pixel));
    // pixel = qRgba(gray, gray, gray, qAlpha(pixel));
}

void forEachPixel(std::vector<unsigned char>& img, std::function<QRgb(QRgb)> func)
// void forEachPixel(std::vector<unsigned char>& img, std::function<void(QRgb&)> func)
{
    const auto data = reinterpret_cast<QRgb*>(img.data());
    const auto size = img.size() / sizeof(QRgb);
    for (uint i = 0; i < img.size() / sizeof(QRgb); ++i)
        data[i] = func(data[i]);
}

void paintGray(std::vector<unsigned char>& img)
{
    const auto data = reinterpret_cast<QRgb*>(img.data());
    const auto size = img.size() / sizeof(QRgb);
    for (uint i = 0; i < size; ++i)
    {
        const auto gray = int((((float(qRed(data[i]))   * KR) +
                                 (float(qGreen(data[i])) * KG) +
                                 (float(qBlue(data[i]))  * KB))));
        data[i] = qRgba(gray, gray, gray, qAlpha(data[i]));
    }
}

int main()
{
    auto vec = std::vector<unsigned char>(2560*1080, 0x00);
    
    forEachPixel(vec, toGray);
    // paintGray(vec);
}
