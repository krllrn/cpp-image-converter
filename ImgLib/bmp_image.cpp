#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

//Важно, что между полями отсутствует padding. Таким образом, размер первой части — 14 байт, размер второй части — 40 байт.
#pragma pack(push, 1)
PACKED_STRUCT_BEGIN BitmapFileHeader {
    char b = 'B'; // Подпись — 2 байта. Символы BM.
    char m = 'M';
    uint32_t header_data_size; // Суммарный размер заголовка и данных - 4 байта. Размер данных определяется как отступ, умноженный на высоту изображения.
    uint32_t reserved = {}; // Зарезервированное пространство — 4 байта, заполненные нулями.
    uint32_t stride_from_begin = 54; // Отступ данных от начала файла — 4 байта, беззнаковое целое. Он равен размеру двух частей заголовка.
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t header_size = 40; // Размер заголовка — 4 байта, беззнаковое целое. Учитывается только размер второй части заголовка.
    int32_t width; // Ширина изображения в пикселях — 4 байта, знаковое целое.
    int32_t height; // Высота изображения в пикселях — 4 байта, знаковое целое.
    uint16_t plane_amount = 1; // Количество плоскостей — 2 байта, беззнаковое целое. В нашем случае всегда 1 — одна RGB плоскость.
    uint16_t byte_per_pixel = 24; // Количество бит на пиксель — 2 байта, беззнаковое целое. В нашем случае всегда 24.
    uint32_t compress_size = 0; // Тип сжатия — 4 байта, беззнаковое целое. В нашем случае всегда 0 — отсутствие сжатия.
    uint32_t data_size; // Количество байт в данных — 4 байта, беззнаковое целое. Произведение отступа на высоту.
    int32_t h_resolution = 11811; // Горизонтальное разрешение, пикселей на метр — 4 байта, знаковое целое. 
    int32_t v_resolution = 11811; // Вертикальное разрешение, пикселей на метр — 4 байта, знаковое целое.
    int32_t used_colors = 0; // Количество использованных цветов — 4 байта, знаковое целое. Нужно записать 0 — значение не определено.
    int32_t significant_colors = 0x1000000; // Количество значимых цветов — 4 байта, знаковое целое. Нужно записать 0x1000000.

}
PACKED_STRUCT_END
#pragma pack(pop)

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

void WriteHeaders(BitmapFileHeader& file_header, BitmapInfoHeader& info_header, ofstream& out) {
    out.write((const char*)&file_header, sizeof(file_header));
    out.write((const char*)&info_header, sizeof(info_header));
}

bool SaveBMP(const Path& file, const Image& image) {
    const int w = image.GetWidth();
    const int h = image.GetHeight();
    uint32_t data_size = GetBMPStride(w) * h;
    int stride = GetBMPStride(w);

    BitmapFileHeader file_header;
    file_header.header_data_size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + data_size;

    BitmapInfoHeader info_header;
    info_header.width = w;
    info_header.height = h;
    info_header.data_size = data_size;
    
    ofstream out(file, ios::binary);
    WriteHeaders(file_header, info_header, out);

    std::vector<char> buff(stride);

    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), stride);
    }

    return out.good();
}

Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);

    BitmapFileHeader file_header;
    ifs.read((char*)&file_header, sizeof(file_header));
    if (file_header.b != 'B' && file_header.m != 'M') {
        ifs.close();
        return {};
    }
    
    BitmapInfoHeader info_header;
    ifs.read((char*)&info_header, sizeof(info_header));
    if (info_header.byte_per_pixel != 24) {
        ifs.close();
        return {};
    }

    ifs.seekg(file_header.stride_from_begin);

    img_lib::Image result(info_header.width, info_header.height, Color::Black());

    int32_t row_size = GetBMPStride(info_header.width);
    std::vector<char> buff(row_size);
    for (int y = info_header.height - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), row_size);

        for (int x = 0; x < info_header.width; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib