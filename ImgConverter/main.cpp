#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>
#include <bmp_image.h>

#include <filesystem>
#include <string_view>
#include <iostream>

using namespace std;

enum class Format {
    PPM,
    JPEG ,
    BMP ,
    UNKNOWN
};

namespace FormatInterfaces {

class ImageFormatInterface {
public:
    virtual bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const = 0;
    virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
};

class PPM : public ImageFormatInterface {
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
        return img_lib::SavePPM(file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadPPM(file);
    }
};

class JPEG : public ImageFormatInterface {
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
        return img_lib::SaveJPEG(file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadJPEG(file);
    }
};

class BMP : public ImageFormatInterface {
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
        return img_lib::SaveBMP(file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadBMP(file);
    }
};
} // namespace FormatInterfaces

Format GetFormatByExtension(const img_lib::Path& input_file) {
    const string ext = input_file.extension().string();
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }

    if (ext == ".bmp"sv) {
        return Format::BMP;
    }

    return Format::UNKNOWN;
}

const FormatInterfaces::ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
    auto format = GetFormatByExtension(path);

    static const FormatInterfaces::PPM ppmInterface;
    static const FormatInterfaces::JPEG jpegInterface;
    static const FormatInterfaces::BMP bmpInterface;

    switch (format)
    {
    case Format::PPM:
    {
        return &ppmInterface;
    }
    case Format::JPEG:
    {
        return &jpegInterface;
    }
    case Format::BMP:
    {
        return &bmpInterface;
    }
    case Format::UNKNOWN:
    {
        return nullptr;
    }
    }
}

int main(int argc, const char** argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    Format in_p = GetFormatByExtension(in_path);
    Format out_p = GetFormatByExtension(out_path);
    if (in_p == Format::UNKNOWN) {
        cerr << "Unknown INPUT format"sv << '\n';
        return 2;
    } else if (out_p == Format::UNKNOWN) {
        cerr << "Unknown OUTPUT format"sv << '\n';
        return 3;
    } else {
        const FormatInterfaces::ImageFormatInterface* input_format = GetFormatInterface(in_path);
        const FormatInterfaces::ImageFormatInterface* output_format = GetFormatInterface(out_path);

        img_lib::Image image = input_format->LoadImage(in_path);
        if (!image) {
            cerr << "Loading failed"sv << endl;
            return 4;
        }

        if (!output_format->SaveImage(out_path, image)) {
            cerr << "Saving failed"sv << endl;
            return 5;
        }

        cout << "Successfully converted"sv << endl;
    }
}