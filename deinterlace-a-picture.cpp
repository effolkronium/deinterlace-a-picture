#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

enum { e_bin_name, e_input_jpeg, e_output_jpeg, e_size_args };

void deinterlace(const fs::path& input_jpeg, const fs::path& output_jpeg) {
    throw std::logic_error{"TODO"};
}

int main(int argc, char* argv[]) try {
    if (argc < e_size_args)
        throw std::invalid_argument{"Usage: input.jpeg output.jpeg"};

    return 0;
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
}
