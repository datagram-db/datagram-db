//
// Created by giacomo on 12/04/25.
//

#include <ndp/MainMemoryIndicesForSecondary.h>

void QuickGraph::build() {
    if (!isBuilt) {
        std::sort(arcs.begin(), arcs.end());
        gr.build(n, arcs.begin(), arcs.end());
        arcs.clear();
        isBuilt = true;
    }
}

void init_attributes(MainMemoryIndicesForSecondary& data, const std::filesystem::path& path) {
    std::ifstream file(path);
    std::string line;
    while(!file.eof())
    {
        std::getline(file,line);
        if (line.size() > 0) {
            auto& ref = data.attribute_information[line];
            ref.attribute_id = data.attribute_names.size();
            data.attribute_names.emplace_back(line);
            std::getline(file,line);
            ref.type = magic_enum::enum_cast<gsm2::tables::AttributeTableType>(line).value();
        }
    }
    size_t records_offset_begin_as_chars = sizeof(size_t) * data.attribute_names.size();
    size_t records_offset_end_as_chars = records_offset_begin_as_chars;
    for (size_t idx = 0, N = data.attribute_names.size(); idx < N; idx++) {
        const auto& attribute = data.attribute_names.at(idx);
        auto& info = data.attribute_information[attribute];
        info.table_length= data.database_attributes_table[idx];
        info.secondary_memory_table_records_offset_begin = records_offset_begin_as_chars;
        records_offset_end_as_chars += (info.table_length * (sizeof(gsm2::tables::AttributeTable::record)));
        info.secondary_memory_table_records_offset_begin = records_offset_begin_as_chars;
        info.secondary_memory_table_records_offset_end = records_offset_end_as_chars;
        records_offset_begin_as_chars = records_offset_end_as_chars;
    }
}

void init_activities(MainMemoryIndicesForSecondary& data, const std::filesystem::path& folder) {
    std::ifstream file(folder/ "activities_list.txt");
    std::string line;
    data.activity_name_to_offset[line].activity_id =data.activity_list.size();
    data.activity_list.emplace_back(line);
    while(!file.eof())
    {
        std::getline(file,line);
        if (line.size() > 0) {
            data.activity_name_to_offset[line].activity_id = data.activity_list.size();
            data.activity_list.emplace_back(line);
        }
    }
    std::ifstream primary_index(folder / "activity_table_data_pi.txt");
    size_t offset_reading = 0;
    while (!primary_index.eof()) {
        std::getline(primary_index,line);
        std::stringstream ss(line);
        std::string cell;
        if (std::getline(ss, cell, ',')) {
            offset_reading = std::stoull(cell);
            auto& record = data.activity_name_to_offset[data.activity_list[offset_reading]];
            if (std::getline(ss, cell, ',')) {
                record.offset_begin = std::stoull(cell);
                if (std::getline(ss, cell, ',')) {
                    record.offset_end = std::stoull(cell);
                }
            }
        }

        offset_reading++;
    }
}

void init_containment(MainMemoryIndicesForSecondary& data, const std::filesystem::path& path) {
    std::ifstream file(path);
    std::string line;
    while(!file.eof())
    {
        std::getline(file,line);
        if (line.size() > 0) {
            auto& ref = data.containment_information[line];
            ref.containment_id = data.phi_table_names.size();
            data.phi_table_names.emplace_back(line);
        }
    }
    size_t secondary_index_constant_padding = 0;
    size_t records_offset_begin_as_chars = sizeof(size_t) * data.phi_table_names.size();
    size_t records_offset_end_as_chars = records_offset_begin_as_chars;
    size_t table_padding = data.total_objects * 2;
    for (size_t idx = 0, N = data.phi_table_names.size(); idx < N; idx++) {
        const auto& attribute = data.phi_table_names.at(idx);
        auto& info = data.containment_information[attribute];
        info.table_length= data.database_containment_table[idx];
        info.secondary_memory_table_records_offset_begin = records_offset_begin_as_chars;
        records_offset_end_as_chars += (info.table_length * (sizeof(gsm2::tables::PhiTable::secmem_record)));
        info.secondary_memory_table_records_offset_begin = records_offset_begin_as_chars;
        info.secondary_memory_table_records_offset_end = records_offset_end_as_chars;
        records_offset_begin_as_chars = records_offset_end_as_chars;
        info.secondary_index_offset_header = secondary_index_constant_padding;
        secondary_index_constant_padding += table_padding;
        info.secondary_index_offset_contents = secondary_index_constant_padding;
        secondary_index_constant_padding += (info.table_length);
    }
}

void init_basics_graphs(MainMemoryIndicesForSecondary& data, const std::filesystem::path& path) {
    size_t offset = 0;
    yaucl::data::FixedSizeArrayElements<size_t> obj_file(path / "object_index_with_scores_offset.binary");
    yaucl::data::FixedSizeArrayElements<double> scores_file(path / "actual_scores.binary");
    yaucl::data::FixedSizeArrayElements<size_t> object_offset_in_activity_table(path / "activity_table_secondary_index.bin");
    data.n_graphs = obj_file[0];
    data.graph_information.resize(data.n_graphs);
//        data.graph_to_n_obj_offset.reserve(n_graphs);
//        data.scores.resize(n_graphs);
    data.total_objects = 0;
    for (size_t n_graph = 0; n_graph < data.n_graphs; n_graph++) {
        size_t curr = obj_file[n_graph+1];
        auto& info = data.graph_information[n_graph];
        info.objects.resize(curr);
        info.n_obj = (curr);
        info.n_obj_offset = data.total_objects;
        data.total_objects += curr;
    }

    auto current = (data.n_graphs+1);
    size_t complete_object_offset_list = 0;
    for (size_t n_graph = 0; n_graph < data.n_graphs; n_graph++) {
        auto& info = data.graph_information[n_graph];
        size_t curr = info.n_obj;
        for (size_t n_obj = 0; n_obj<curr; n_obj++) {
            auto& info_obj = info.objects[n_obj];
            size_t n_scores = obj_file[current];
            info_obj.activity_table_entry_id_offset = object_offset_in_activity_table[complete_object_offset_list];
            for (size_t i = 0; i<n_scores; i++) {
                info_obj.scores.emplace_back(scores_file[i+offset]);
            }
            offset += n_scores;
            current++;
            complete_object_offset_list++;
        }
    }
}

void init_ellxi(MainMemoryIndicesForSecondary& data, const std::filesystem::path& path) {
    std::string line, cell;
    {
        bool outcome;
        auto file2 = path/"fuzzyStringMatching"/"object_labels_ells_graphobject_index.txt";
        auto file3 = path/"fuzzyStringMatching"/"object_values_xis_graphobject_index.txt";
        DEBUG_ASSERT(std::filesystem::exists(file2));
        std::ifstream ells(file2);
        std::ifstream xis(file3);
        for (size_t n_graph = 0; n_graph < data.n_graphs; n_graph++) {
            auto& info = data.graph_information[n_graph];
            size_t curr = info.n_obj;
            for (size_t n_obj = 0; n_obj<curr; n_obj++) {
                DEBUG_ASSERT(!ells.eof());
                DEBUG_ASSERT(!xis.eof());
                outcome = (bool)std::getline(ells,line);
                DEBUG_ASSERT(outcome);
                if (line.size()>0) {
                    std::stringstream ss(line);
                    auto& info_obj = info.objects[n_obj];
                    while (std::getline(ss, cell, ',')) {
                        info_obj.ells.emplace_back(std::stoull(cell));
                    }
                }
                outcome = (bool)std::getline(xis,line);
                DEBUG_ASSERT(outcome);
                if (line.size()>0) {
                    std::stringstream ss(line);
                    auto& info_obj = info.objects[n_obj];
                    while (std::getline(ss, cell, ',')) {
                        info_obj.xis.emplace_back(std::stoull(cell));
                    }
                }
            }
        }
    }
}


MainMemoryIndicesForSecondary::MainMemoryIndicesForSecondary(const std::filesystem::path &path): containment_offsets(path / "database_containment_table_si.binary"), inv_containment_offsets(path / "database_containment_table_si_inv.binary"),
                                                                  ells(path / "fuzzyStringMatching"/"object_labels_ells"),
                                                                  xis(path / "fuzzyStringMatching"/"object_values_xis"),
                                                                  attributes_offsets(path / "database_attributes_table_si.binary"),
                                                                  database_attributes_table(path / "database_attributes_table.binary"),
                                                                  database_containment_table(path / "database_containment_table.binary"),
                                                                  string_attributes(path / "fuzzyStringMatching"/ "string_attributes"),
                                                                  activity_table(path / "activity_table_data.bin") {
    init_basics_graphs(*this, path);
    init_ellxi(*this, path);
    init_attributes(*this, path/ "attributes_headers.txt");
    init_containment(*this, path/ "containment_headers.txt");
    init_activities(*this, path);
}