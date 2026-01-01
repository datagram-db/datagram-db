//
// Created by gyankos on 01/01/26.
//

#ifndef GSM2_ZSV_CPP_H
#define GSM2_ZSV_CPP_H

struct local_zsv {
    // void* zsv_opts_data;
    FILE* f;
    void* parser;
};
#ifdef __cplusplus
extern "C" {
#endif

    /* Function declaration */
    unsigned char open_csv_parser(const char* filename, struct local_zsv *zsv, char delimiter);
    unsigned char has_csv_next(struct local_zsv *zsv);
    size_t csv_cells_in_current_row(struct local_zsv *zsv);
    size_t close_csv_parser(struct local_zsv *zsv);
    size_t ith_csv_cell(struct local_zsv *zsv, size_t idx, unsigned char **str,  size_t* len,  char* quoted,
  unsigned char* overwritten);

#ifdef __cplusplus
}
#endif




#endif //GSM2_ZSV_CPP_H