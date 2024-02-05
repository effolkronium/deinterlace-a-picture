#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include "jpeglib.h"

namespace fs = std::filesystem;

void deinterlace(const fs::path& input_jpeg, const fs::path& output_jpeg) {
    throw std::logic_error{"TODO"};
}

enum { e_bin_name, e_input_jpeg, e_output_jpeg, e_size_args };

int main(int argc, char* argv[]) try {
    if (argc < e_size_args)
        throw std::invalid_argument{"Usage: input.jpeg output.jpeg"};

    return 0;
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
}
