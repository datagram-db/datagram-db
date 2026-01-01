//
// Created by gyankos on 01/01/26.
//

#include <filesystem>
#include <iostream>

#include "zsv_cpp.h"

class SIMDParser {
    struct local_zsv c_struct;
    bool error;

public:
    SIMDParser(const std::filesystem::path& file, unsigned char delimiter) {
        if (open_csv_parser(file.c_str(), &c_struct, delimiter) == EXIT_FAILURE)
            error = true;
        else
            error = false;
    }

    ~SIMDParser() {
        close_csv_parser(&c_struct);
    }

    bool getNextRow() {
        return (has_csv_next(&c_struct) == EXIT_SUCCESS);
    }

    uint64_t nCells() {
        return csv_cells_in_current_row(&c_struct);
    }

    template<bool perform_check = true>
    std::pair<std::string_view, bool> getCell(uint64_t cell) {
        if ((!perform_check) || (cell < nCells())) {
            unsigned char *str = nullptr;
            size_t len = 0;
            char quoted = 0;
            unsigned char overwritten = 0;
            ith_csv_cell(&c_struct, cell, &str, &len, &quoted, &overwritten);
            if (str && len) {
                return {std::string_view{(const char*)str, len}, quoted ? true : false};
            }
        }
        return {{nullptr, 0}, false};
    }

};

int main() {
    SIMDParser file("/home/gyankos/Scaricati/osfstorage-archive/Data/Dataset/Customer/person_0_0.csv", '|');
    while (file.getNextRow()) {
        uint64_t N = file.nCells();
        for (uint64_t i = 0; i < N; i++) {
            std::cout << file.getCell(i).first ;
            if ((i)!= N-1)  std::cout << ",";
        }
        std::cout << std::endl;
    }
}

