#include <easylogging++.h>
INITIALIZE_EASYLOGGINGPP
#include <iostream>
#include <parser/schema/DataLoaderFromSchema.h>

void final_writing(DataFormatHandler& dfh) {
    dfh.load_to_primary_memory("/home/giacomo/Scaricati/Unibench-0.2/Dataset/Schema.txt", DataFormat::Schema);
    std::cout << dfh.count_databases() << std::endl;
}

void minimal_writing(DataFormatHandler& dfh) {
    dfh.load_to_secondary_memory("/home/giacomo/projects/DATA_IDEAS2021/Schema.txt",
                                 "/home/giacomo/projects/gsm2/secondary_tests/minimal",
                                 DataFormat::Schema);
    std::cout << dfh.count_databases() << std::endl;
    //    dfh.data_converter("/home/giacomo/projects/DATA_IDEAS2021/Schema.txt",
//                       DataFormat::Schema,
//                       "/home/giacomo/projects/DATA_IDEAS2021/gsm.txt",
//                       DataFormat::GSM);

}

#include <yaucl/data/VariadicSizeArrayElements.h>

struct ContainmentEntry {
    size_t offset_begin;
    size_t offset_end;
    yaucl::data::FixedSizeArrayElements<gsm2::tables::PhiTable::secmem_record>* view;
    yaucl::data::FixedSizeArrayElements<gsm2::tables::PhiTable::primary_index_2m>* view_primary;

    ~ContainmentEntry() {
        delete view;
        delete view_primary;
    }
};

struct ActivityEntry {
    size_t offset_begin;
    size_t offset_end;
    size_t activity_id;
};

struct GraphToObjInformation {                      // Defines the pointer towards the information being stored for each object
    size_t activity_table_entry_id_offset;          // Offset to the secondary memory activity table
    std::vector<double> scores;                     // Trustworthyness scores associated with each object
};

struct GraphInformation {                           // Defines the information stored and indexed for each graph
    size_t n_obj;                                   // Number of contained objects
    size_t n_obj_offset;                            // Pre-computed object offsets
    std::vector<GraphToObjInformation> objects;     // Pointer towards the collected object information
};

struct MainMemoryIndicesForSecondary {
    std::vector<GraphInformation> graph_information;
    size_t total_objects;

    // Activity Table                                                           ~ activities are the first label occurring for each object
    std::vector<std::string> activity_list;
    std::unordered_map<std::string, ActivityEntry> activity_name_to_offset;

    // Containment tables
    std::vector<std::string> phi_table_names;
    std::vector<size_t> phi_table_name_to_offset;
    std::vector<ContainmentEntry> entries;
    std::unordered_map<std::string, size_t> phi_table_name_to_position;

    // Attribute table
    std::vector<std::string> attribute_names;
    std::unordered_map<std::string,gsm2::tables::AttributeTableType> attribute_types;

    MainMemoryIndicesForSecondary(const std::filesystem::path& path) : containment_offsets(path / "database_containment_table_pi.binary"),
                                                                       containment_secondary_indices(path / "database_containment_table_si.binary"),
                                                                       containments(path / "database_containment_table.binary"){

    }

    std::pair<size_t, size_t> containment_secondary_index(const std::string& name, size_t graph_id, size_t obj_id) {
        if (graph_id >= graph_information.size())
            return {-1,-1};
        if (graph_information.at(graph_id).n_obj <= obj_id)
            return {-1,-1};
        auto it = phi_table_name_to_position.find(name);
        if (it == phi_table_name_to_position.end())
            return {-1,-1};

    }

    inline void init_containment() {
        size_t n_containment_tables = phi_table_names.size();
        phi_table_name_to_offset.reserve(n_containment_tables);
        entries.reserve(n_containment_tables);
        size_t c = /*containments.raw_data() +*/ ((sizeof (size_t))* n_containment_tables);
        size_t cumulative_containment = 0, cumulative_primary_containment = 0, cumulative_secondary_offset = 0;
        for (size_t idx = 0; idx < n_containment_tables; idx++) {
            phi_table_name_to_position[phi_table_names[idx]] = idx;
            phi_table_name_to_offset.emplace_back(containment_offsets[idx]);

            // Primary Index offsets
            auto primary_len = phi_table_name_to_offset[idx];
            auto primary_offset = c + ((cumulative_primary_containment * sizeof(gsm2::tables::PhiTable::secmem_record)));
            cumulative_primary_containment += primary_len;
            entries[idx].view_primary = containment_offsets.open_view_at_raw_offset<gsm2::tables::PhiTable::primary_index_2m>(primary_offset, primary_len);

            // Secondary Indx offsets
            // TODO

            // Main Table offsets
            auto contained_edges = (containments[idx]);
            auto local_offset = c + ((cumulative_containment * sizeof(gsm2::tables::PhiTable::secmem_record)));
            cumulative_containment += contained_edges;
            entries[idx].view = containments.open_view_at_raw_offset<gsm2::tables::PhiTable::secmem_record>(local_offset, contained_edges);
        }
    }

private:
    yaucl::data::FixedSizeArrayElements<size_t> containment_offsets, containment_secondary_indices, containments;
};

void load_database_test(const std::filesystem::path& path) {
    MainMemoryIndicesForSecondary data(path);
    {
        size_t offset = 0;
        yaucl::data::FixedSizeArrayElements<size_t> obj_file(path / "object_index_with_scores_offset.binary");
        yaucl::data::FixedSizeArrayElements<double> scores_file(path / "actual_scores.binary");
        yaucl::data::FixedSizeArrayElements<size_t> object_offset_in_activity_table(path / "activity_table_secondary_index.bin");
        size_t n_graphs = obj_file[0];
        data.graph_information.resize(n_graphs);
//        data.graph_to_n_obj_offset.reserve(n_graphs);
//        data.scores.resize(n_graphs);
        data.total_objects = 0;
        for (size_t n_graph = 0; n_graph < n_graphs; n_graph++) {
            size_t curr = obj_file[n_graph+1];
            auto& info = data.graph_information[n_graph];
            info.objects.resize(curr);
            info.n_obj = (curr);
            info.n_obj_offset = (data.total_objects);
            data.total_objects += curr;
        }

        auto current = (n_graphs+1);
        size_t complete_object_offset_list = 0;
        for (size_t n_graph = 0; n_graph < n_graphs; n_graph++) {
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

    std::string line;
    {
        std::ifstream file(path/ "activities_list.txt");
        while(!file.eof())
        {
            std::getline(file,line);
            if (line.size() > 0) {
                data.activity_name_to_offset[line].activity_id = data.activity_list.size();
                data.activity_list.emplace_back(line);
            }
        }
        std::ifstream primary_index(path / "activity_table_data_pi.txt");
        size_t offset_reading = 0;
        while (!file.eof()) {
            std::stringstream ss(line);
            std::string cell;
            auto& record = data.activity_name_to_offset[data.activity_list[offset_reading]];
            if (std::getline(ss, cell, ',')) {
                record.offset_begin = std::stoull(cell);
                if (std::getline(ss, cell, ',')) {
                    record.offset_end = std::stoull(cell);
                }
            }
            offset_reading++;
        }
    }
    {
        std::ifstream file(path/ "containment_headers.txt");
        while(!file.eof())
        {
            std::getline(file,line);
            if (line.size() > 0)
                data.phi_table_names.emplace_back(line);
        }
        data.init_containment();
    }
    {
        std::ifstream file(path/ "attributes_headers.txt");
        while(!file.eof())
        {
            std::getline(file,line);
            if (line.size() > 0) {
                data.attribute_names.emplace_back(line);
                std::getline(file,line);
                data.attribute_types[*data.attribute_names.rbegin()] = magic_enum::enum_cast<gsm2::tables::AttributeTableType>(line).value();
            }
        }
    }


}


int main(void) {
    DataFormatHandler dfh;
    minimal_writing(dfh);
    load_database_test("/home/giacomo/projects/gsm2/secondary_tests/minimal");

    return EXIT_SUCCESS;
}
