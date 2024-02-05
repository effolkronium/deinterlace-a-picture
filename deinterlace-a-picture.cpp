#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <memory>
#include "jpeglib.h"

namespace fs = std::filesystem;
using namespace std::string_literals;

class Jpeg {
   public:
    struct YCbCr {
        unsigned char Y;
        unsigned char Cb;
        unsigned char Cr;
    };

    using callback_t = std::function<void(std::vector<YCbCr>&& line)>;

    explicit Jpeg(const std::vector<std::uint8_t>& jpeg_data)
        : m_jpeg_data{jpeg_data} {
        m_cinfo.err = jpeg_std_error(&m_jerr_mgr);
        m_cinfo_compress.err = jpeg_std_error(&m_jerr_mgr);
        m_jerr_mgr.error_exit = [](j_common_ptr cinfo) {
            char pszErr[1024]{};
            cinfo->err->format_message(cinfo, pszErr);
            throw std::runtime_error{"Jpeg error: "s + pszErr};
        };
    }

    void readLinesYCbCr(const callback_t& callback) {
        jpeg_create_decompress(&m_cinfo);
        jpeg_mem_src(&m_cinfo, m_jpeg_data.data(), m_jpeg_data.size());
        int rc = jpeg_read_header(&m_cinfo, TRUE);

        if (rc != 1) throw std::invalid_argument{"Cannot read jpeg header"};

        m_cinfo.out_color_space = J_COLOR_SPACE::JCS_YCbCr;

        jpeg_start_decompress(&m_cinfo);

        size_t jpeg_size = m_cinfo.output_width * m_cinfo.output_height *
                           m_cinfo.output_components;

        size_t line_size = m_cinfo.output_width;

        if (m_cinfo.output_components != sizeof(YCbCr))
            throw std::runtime_error{"Invalid output color space"};

        if (m_cinfo.out_color_space != J_COLOR_SPACE::JCS_YCbCr)
            throw std::runtime_error{"Invalid output color space"};

        while (m_cinfo.output_scanline < m_cinfo.output_height) {
            std::vector<YCbCr> line{line_size};
            auto* line_bufp = reinterpret_cast<std::uint8_t*>(line.data());
            jpeg_read_scanlines(&m_cinfo, &line_bufp, 1);
            callback(std::move(line));
        }

        jpeg_finish_decompress(&m_cinfo);
        jpeg_destroy_decompress(&m_cinfo);
    }

    std::vector<std::uint8_t> writeYCbCrToJpeg(
        std::vector<std::vector<Jpeg::YCbCr>> lines) {
        std::vector<std::uint8_t> output;
        jpeg_create_compress(&m_cinfo_compress);

        uint8_t* outbuffer = nullptr;
        size_t outlen = 0;
        jpeg_mem_dest(&m_cinfo_compress, &outbuffer, &outlen);
        
        m_cinfo_compress.image_width = lines.back().size();
        m_cinfo_compress.image_height = lines.size();
        m_cinfo_compress.input_components = 3;
        m_cinfo_compress.in_color_space = J_COLOR_SPACE::JCS_YCbCr;

        jpeg_set_defaults(&m_cinfo_compress);
        jpeg_set_quality(&m_cinfo_compress, 100, TRUE);
        jpeg_start_compress(&m_cinfo_compress, TRUE);

        while (m_cinfo_compress.next_scanline < m_cinfo_compress.image_height) {
            auto row_ptr = reinterpret_cast<std::uint8_t*>(
                lines[m_cinfo_compress.next_scanline].data());
            jpeg_write_scanlines(&m_cinfo_compress, &row_ptr, 1);
        }

        jpeg_finish_compress(&m_cinfo_compress);
        jpeg_destroy_compress(&m_cinfo_compress);

        
        return {outbuffer, outbuffer + outlen};
    }

   private:
    const std::vector<std::uint8_t>& m_jpeg_data;
    jpeg_decompress_struct m_cinfo{};
    jpeg_compress_struct m_cinfo_compress{};
    jpeg_error_mgr m_jerr_mgr{};
};

std::vector<std::uint8_t> deinterlace(
    const std::vector<std::uint8_t>& jpeg_data) {
    Jpeg jpeg{jpeg_data};

    std::vector<std::vector<Jpeg::YCbCr>> lines, orig;
    jpeg.readLinesYCbCr([&lines, &orig](std::vector<Jpeg::YCbCr>&& line) {
        if (lines.empty()) {
            orig.push_back(line);
            lines.emplace_back(std::move(line));
            
            return;
        }

        const auto& last_line = orig.back();
        for (size_t i = 0; i < line.size(); ++i) {

            line[i].Y = (last_line[i].Y + line[i].Y) / 2;
            line[i].Cb = (last_line[i].Cb + line[i].Cb) / 2;
            line[i].Cr = (last_line[i].Cr + line[i].Cr) / 2;

        }

        orig.push_back(line);
        lines.emplace_back(std::move(line));
    });

    return jpeg.writeYCbCrToJpeg(lines);
}

void deinterlace(const fs::path& input_jpeg_path,
                 const fs::path& output_jpeg_path) {
    if (!fs::exists(input_jpeg_path))
        throw std::invalid_argument{"Input file not found"};

    std::ifstream input_stream(input_jpeg_path,
                               std::ios::in | std::ios::binary);
    std::vector<std::uint8_t> input_jpeg_data{
        std::istreambuf_iterator<char>{input_stream},
        std::istreambuf_iterator<char>{}};

    auto output_jpeg_data = deinterlace(input_jpeg_data);

    std::ofstream output_stream(output_jpeg_path,
                                std::ios::out | std::ios::binary);
    output_stream.write(reinterpret_cast<char*>(output_jpeg_data.data()),
                        static_cast<std::streamsize>(output_jpeg_data.size()));
}

enum { e_bin_name, e_input_jpeg, e_output_jpeg, e_size_args };

int main(int argc, char* argv[]) try {
    if (argc < e_size_args)
        throw std::invalid_argument{"Usage: input.jpeg output.jpeg"};

    deinterlace(argv[e_input_jpeg], argv[e_output_jpeg]);

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
