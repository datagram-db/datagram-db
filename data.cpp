#include <easylogging++.h>
INITIALIZE_EASYLOGGINGPP
#include <iostream>
#include <parser/schema/DataLoaderFromSchema.h>

#define DEBUG



#include <yaucl/data/SimpleStringSerializer.h>
void final_writing(DataFormatHandler& dfh) {
    dfh.load_to_secondary_memory("/home/giacomo/Scaricati/Unibench-0.2/Dataset/Schema.txt",
                                 "/home/giacomo/projects/gsm2/secondary_tests/final",
                                 DataFormat::Schema);
}

void minimal_writing(DataFormatHandler& dfh) {
    dfh.load_to_secondary_memory("/home/giacomo/projects/DATA_IDEAS2021/Schema.txt",
                                 "/home/giacomo/projects/gsm2/secondary_tests/minimal",
                                 DataFormat::Schema);
//    std::cout << dfh.count_databases() << std::endl;
    //    dfh.data_converter("/home/giacomo/projects/DATA_IDEAS2021/Schema.txt",
//                       DataFormat::Schema,
//                       "/home/giacomo/projects/DATA_IDEAS2021/gsm.txt",
//                       DataFormat::GSM);

}

#include <yaucl/data/VariadicSizeArrayElements.h>


struct ActivityEntry {
    size_t offset_begin;
    size_t offset_end;
    size_t activity_id;
};

struct GraphToObjInformation {                               // Defines the pointer towards the information being stored for each object
    size_t activity_table_entry_id_offset;                   // Offset to the secondary memory activity table
    std::vector<double> scores;                              // Trustworthyness scores associated with each object
    std::vector<size_t> ells;                                // storing just the id for the labels
    std::vector<size_t> xis;                                 // storing just the id for the values
};

struct GraphInformation {                           // Defines the information stored and indexed for each graph
    size_t n_obj;                                   // Number of contained objects
    size_t n_obj_offset;                            // Pre-computed object offsets
    std::vector<GraphToObjInformation> objects;     // Pointer towards the collected object information
};

struct AttributeInformation {
    size_t attribute_id;
    size_t secondary_memory_table_records_offset_begin;
    size_t secondary_memory_table_records_offset_end;
    size_t table_length;
    gsm2::tables::AttributeTableType type;
};

struct ContainmentInformation {
    size_t containment_id;
    size_t secondary_memory_table_records_offset_begin;
    size_t secondary_memory_table_records_offset_end;
    size_t table_length;
    size_t secondary_index_offset_header;
    size_t secondary_index_offset_contents;
};

struct MainMemoryIndicesForSecondary {
    std::vector<GraphInformation> graph_information;
    size_t total_objects;




    MainMemoryIndicesForSecondary(const std::filesystem::path& path) : containment_offsets(path / "database_containment_table_si.binary"),
                                                                       ells(path / "fuzzyStringMatching"/"object_labels_ells"),
                                                                       xis(path / "fuzzyStringMatching"/"object_values_xis"),
                                                                       attributes_offsets(path / "database_attributes_table_si.binary"),
                                                                       database_attributes_table(path / "database_attributes_table.binary"),
                                                                       database_containment_table(path / "database_containment_table.binary"),
                                                                       string_attributes(path / "fuzzyStringMatching"/ "string_attributes"),
                                                                       activity_table(path / "activity_table_data.bin") {

    }

    inline size_t getNGraphs() const { return graph_information.size(); }
    inline size_t getNObjects() const { return total_objects; }
    inline size_t getNObjects(size_t graphId) const {
        if (graph_information.size() <= graphId)
            return 0;
        DEBUG_ASSERT(graph_information.at(graphId).n_obj == graph_information.at(graphId).objects.size());
        return graph_information.at(graphId).n_obj;
    }

    // Activity Table                                                           ~ activities are the first label occurring for each object
    std::vector<std::string> activity_list;
    std::unordered_map<std::string, ActivityEntry> activity_name_to_offset;
    inline const std::pair<const gsm2::tables::ActivityTable::secmem_record*,const gsm2::tables::ActivityTable::secmem_record*> getTable(const std::string& activity) {
        auto it = activity_name_to_offset.find(activity);
        if (it == activity_name_to_offset.end())
            return {nullptr, nullptr};
       return {activity_table.begin() + it->second.offset_begin, activity_table.begin() + it->second.offset_end};
    }

    // Containment tables
    std::vector<std::string> phi_table_names;
    std::unordered_map<std::string, ContainmentInformation> containment_information;

    inline const std::vector<std::string>& getContainments() const { return phi_table_names; }
    inline const size_t numberOfContainments(size_t idx) const {
        if (idx >= attribute_names.size()) return 0;
        return numberOfContainments(attribute_names.at(idx));
    }
    inline const size_t numberOfContainments(const std::string&name) const {
        auto it = containment_information.find(name);
        if (it == containment_information.end()) return 0;
        return it->second.table_length;
    }
    inline size_t countContainments(size_t graph_id, size_t object_id, const std::string& name) const {
        if (graph_information.size() <= graph_id)
            return 0;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return 0;
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return 0;
        size_t computd_offset = (it->second.containment_id * total_objects + ref.n_obj_offset + object_id)*2;
        DEBUG_ASSERT(computd_offset < containment_offsets.size());
        size_t begin_offset = containment_offsets[computd_offset];
        size_t end_offset = containment_offsets[computd_offset+1];
        return end_offset - begin_offset;
    }
    inline gsm2::tables::PhiTable::secmem_record* getIthContainments(size_t graph_id, size_t object_id, const std::string& name, size_t i) const {
        if (graph_information.size() <= graph_id)
            return nullptr;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return nullptr;
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return nullptr;
        size_t computd_offset = (it->second.containment_id * total_objects + ref.n_obj_offset + object_id)*2;
        DEBUG_ASSERT(computd_offset < containment_offsets.size());
        size_t begin_offset = containment_offsets[computd_offset];
        size_t end_offset = containment_offsets[computd_offset+1];
        size_t len = end_offset - begin_offset;
        if (len <= i)
            return nullptr;
        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
        return ptr + begin_offset + i;
    }
    inline void setContainmentToObject(size_t graph_id, size_t object_id, const std::string& name, gsm_object& object) const {
        if (graph_information.size() <= graph_id)
            return ;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return ;
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return ;
        size_t computd_offset = it->second.secondary_index_offset_header + (ref.n_obj_offset + object_id)*2;
        DEBUG_ASSERT(computd_offset < containment_offsets.size());
        size_t begin_offset = containment_offsets[computd_offset];
        size_t end_offset = containment_offsets[computd_offset+1];
        size_t len = end_offset - begin_offset;
        if (len == 0)
            return;
        gsm_object_xi_content containment;
        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
        std::ptrdiff_t diff = (ptr + begin_offset) - ptr;
        for (int i = 0; i<len; i++) {
            auto idx = containment_offsets[it->second.secondary_index_offset_contents + begin_offset + i];
            const auto& current = ptr[idx];
            containment.id = current.id_contained;
            containment.orig_edge_id = current.instance_id;
            containment.score = current.w_contained;
            object.phi[name].emplace_back(containment);
        }
    }

    // Attribute table
    std::vector<std::string> attribute_names;
    std::unordered_map<std::string, AttributeInformation> attribute_information;

    inline const std::vector<std::string>& getAttributes() const { return attribute_names; }
    inline const size_t attributeTableLength(size_t idx) const {
        if (idx >= attribute_names.size()) return 0;
        return attributeTableLength(attribute_names.at(idx));
    }
    inline const size_t attributeTableLength(const std::string& name) const {
        auto it = attribute_information.find(name);
        if (it == attribute_information.end()) return 0;
        return it->second.table_length;
    }
    std::optional<union_type> getAttribute(size_t graph_id, size_t object_id, const std::string& name) const {
        auto cp = hasAttribute(graph_id, object_id, name);
        if (!cp.second)
            return {};
        auto it = attribute_information.find(name);
        gsm2::tables::AttributeTable::record* ptr = (gsm2::tables::AttributeTable::record*)(database_attributes_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
#ifdef DEBUG
        size_t offset_to_displacement = cp.first * sizeof(gsm2::tables::AttributeTable::record*);
        DEBUG_ASSERT(it->second.secondary_memory_table_records_offset_begin + offset_to_displacement < it->second.secondary_memory_table_records_offset_end);
#endif
        auto& x = ptr[cp.first];
        switch (it->second.type) {
            case gsm2::tables::AttributeTableType::DoubleAtt:
                return {*(double*)(&x.value)};
            case gsm2::tables::AttributeTableType::LongAtt:
                return {*(long long*)(&x.value)};
            case gsm2::tables::AttributeTableType::StringAtt:
                return {std::string{string_attributes.str(x.value),string_attributes.strlen(x.value)} };
            case gsm2::tables::AttributeTableType::BoolAtt:
                return {((x.value != 0)) && (x.value != 0.0)};
                //case SizeTAtt:
            default:
                // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
                return {x.value};
        }
    }

    inline std::pair<size_t, bool> hasAttribute(size_t graph_id, size_t object_id, const std::string& name) const {
        std::pair<size_t, bool> result{-1, false};
        if (graph_information.size() <= graph_id)
            return result;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return result;
        auto it = attribute_information.find(name);
        if (it == attribute_information.end())
            return result;
        size_t computd_offset = it->second.attribute_id * total_objects + ref.objects.at(object_id).activity_table_entry_id_offset;
        DEBUG_ASSERT(computd_offset < attributes_offsets.size());
        result.first = attributes_offsets[computd_offset];
        if (result.first == (size_t)-1) {
            return result;
        }
        result.second = true;
        return result;
    }
    inline void setAttributeToObject(size_t graph_id, size_t object_id, const std::string& name, gsm_object&object) const {
        std::pair<size_t, bool> result{-1, false};
        if (graph_information.size() <= graph_id)
            return;// no update
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return;// no update
        auto it = attribute_information.find(name);
        if (it == attribute_information.end())
            return;// no update
        size_t computd_offset = it->second.attribute_id * total_objects + ref.objects.at(object_id).activity_table_entry_id_offset;
        DEBUG_ASSERT(computd_offset < attributes_offsets.size());
        result.first = attributes_offsets[computd_offset];
        if (result.first == (size_t)-1) {
            return; // no update
        }
        result.second = true;
        gsm2::tables::AttributeTable::record* ptr = (gsm2::tables::AttributeTable::record*)(database_attributes_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
#ifdef DEBUG
        size_t offset_to_displacement = result.first * sizeof(gsm2::tables::AttributeTable::record*);
        DEBUG_ASSERT(it->second.secondary_memory_table_records_offset_begin + offset_to_displacement < it->second.secondary_memory_table_records_offset_end);
#endif
        auto& x = ptr[result.first];
        switch (it->second.type) {
            case gsm2::tables::AttributeTableType::DoubleAtt:
                object.content[name] = *(double*)(&x.value);
                break;
            case gsm2::tables::AttributeTableType::LongAtt:
                object.content[name] = (double)*(long long*)(&x.value);
                break;
            case gsm2::tables::AttributeTableType::StringAtt:
                object.content[name] = std::string{string_attributes.str(x.value),string_attributes.strlen(x.value)};
                break;
            case gsm2::tables::AttributeTableType::BoolAtt:
                object.content[name] = (double)(((x.value != 0)) && (x.value != 0.0));
                break;
                //case SizeTAtt:
            default:
                // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
                object.content[name] = (double)x.value;
                break;
        }
    }



    inline size_t getNumberOfLabels(size_t graph_id, size_t objet_id) const {
        if (graph_information.size() <= graph_id)
            return 0;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= objet_id)
            return 0;
        return ref.objects[objet_id].ells.size();
    }

    inline std::string getIthLabel(size_t graph_id, size_t objet_id, size_t i) const {
        if (graph_information.size() <= graph_id)
            return "";
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= objet_id)
            return "";
        if (ref.objects[objet_id].ells.size() <= i)
            return "";
        return ells.get(ref.objects[objet_id].ells.at(i));
    }

    inline size_t getNumberOfValues(size_t graph_id, size_t objet_id) const {
        if (graph_information.size() <= graph_id)
            return 0;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= objet_id)
            return 0;
        return ref.objects[objet_id].xis.size();
    }

    inline std::string getIthValue(size_t graph_id, size_t objet_id, size_t i) const {
        if (graph_information.size() <= graph_id)
            return "";
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= objet_id)
            return "";
        if (ref.objects[objet_id].xis.size() <= i)
            return "";
        return xis.get(ref.objects[objet_id].xis.at(i));
    }

    void  reconstruct_object(size_t graph_id, size_t object_id, gsm_object& object) const {
        object.clear();
        object.id = object_id;
        if (getNGraphs() <= graph_id)
            return;
        if (getNObjects(graph_id) <= object_id)
            return;
        for (size_t n_label = 0, N = getNumberOfLabels(graph_id, object_id); n_label < N; n_label++) {
            object.ell.emplace_back(getIthLabel(graph_id, object_id, n_label));
        }
        for (size_t n_label = 0, N = getNumberOfValues(graph_id, object_id); n_label < N; n_label++) {
            object.xi.emplace_back(getIthValue(graph_id, object_id, n_label));
        }
        for (const auto& attribute : attribute_names) {
            setAttributeToObject(graph_id, object_id, attribute, object);
        }
        for (const auto& containment : phi_table_names) {
            setContainmentToObject(graph_id, object_id, containment, object);
        }
    }

    void print() {
            for (size_t graph_id = 0, N = getNGraphs(); graph_id < N; graph_id ++) {
                for (size_t object_id = 0, M = getNObjects(graph_id); object_id < M; object_id++) {
                    gsm_object object;
                    reconstruct_object(graph_id, object_id, object);
                    object.out_json(std::cout);
                    std::cout << std::endl;
                }
            }
    }

    inline void init_attributes(const std::filesystem::path& path) {
        std::ifstream file(path);
        std::string line;
        while(!file.eof())
        {
            std::getline(file,line);
            if (line.size() > 0) {
                auto& ref = attribute_information[line];
                ref.attribute_id = attribute_names.size();
                attribute_names.emplace_back(line);
                std::getline(file,line);
                ref.type = magic_enum::enum_cast<gsm2::tables::AttributeTableType>(line).value();
            }
        }
        size_t records_offset_begin_as_chars = sizeof(size_t) * attribute_names.size();
        size_t records_offset_end_as_chars = records_offset_begin_as_chars;
        for (size_t idx = 0, N = attribute_names.size(); idx < N; idx++) {
            const auto& attribute = attribute_names.at(idx);
            auto& info = attribute_information[attribute];
            info.table_length= database_attributes_table[idx];
            info.secondary_memory_table_records_offset_begin = records_offset_begin_as_chars;
            records_offset_end_as_chars += (info.table_length * (sizeof(gsm2::tables::AttributeTable::record)));
            info.secondary_memory_table_records_offset_begin = records_offset_begin_as_chars;
            info.secondary_memory_table_records_offset_end = records_offset_end_as_chars;
            records_offset_begin_as_chars = records_offset_end_as_chars;
        }
    }

    inline void init_activities(const std::filesystem::path& folder) {
        std::ifstream file(folder/ "activities_list.txt");
        std::string line;
        activity_name_to_offset[line].activity_id = activity_list.size();
        activity_list.emplace_back(line);
        while(!file.eof())
        {
            std::getline(file,line);
            if (line.size() > 0) {
                activity_name_to_offset[line].activity_id = activity_list.size();
                activity_list.emplace_back(line);
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
                auto& record = activity_name_to_offset[activity_list[offset_reading]];
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

    inline void init_containment(const std::filesystem::path& path) {
        std::ifstream file(path);
        std::string line;
        while(!file.eof())
        {
            std::getline(file,line);
            if (line.size() > 0) {
                auto& ref = containment_information[line];
                ref.containment_id = phi_table_names.size();
                phi_table_names.emplace_back(line);
            }
        }
        size_t secondary_index_constant_padding = 0;
        size_t records_offset_begin_as_chars = sizeof(size_t) * phi_table_names.size();
        size_t records_offset_end_as_chars = records_offset_begin_as_chars;
        size_t table_padding = total_objects * 2;
        for (size_t idx = 0, N = phi_table_names.size(); idx < N; idx++) {
            const auto& attribute = phi_table_names.at(idx);
            auto& info = containment_information[attribute];
            info.table_length= database_containment_table[idx];
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

private:
    yaucl::data::FixedSizeArrayElements<size_t> containment_offsets, attributes_offsets, database_attributes_table, database_containment_table;
    yaucl::data::FixedSizeArrayElements<gsm2::tables::ActivityTable::secmem_record> activity_table;
    NDPFuzzyStringMatching ells, xis;
    SimpleStringReader string_attributes;
};

void load_database_test(const std::filesystem::path& path) {
    MainMemoryIndicesForSecondary data(path);
    size_t n_graphs;
    {
        size_t offset = 0;
        yaucl::data::FixedSizeArrayElements<size_t> obj_file(path / "object_index_with_scores_offset.binary");
        yaucl::data::FixedSizeArrayElements<double> scores_file(path / "actual_scores.binary");
        yaucl::data::FixedSizeArrayElements<size_t> object_offset_in_activity_table(path / "activity_table_secondary_index.bin");
        n_graphs = obj_file[0];
        data.graph_information.resize(n_graphs);
//        data.graph_to_n_obj_offset.reserve(n_graphs);
//        data.scores.resize(n_graphs);
        data.total_objects = 0;
        for (size_t n_graph = 0; n_graph < n_graphs; n_graph++) {
            size_t curr = obj_file[n_graph+1];
            auto& info = data.graph_information[n_graph];
            info.objects.resize(curr);
            info.n_obj = (curr);
            info.n_obj_offset = data.total_objects;
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

    std::string line, cell;
    {
        bool outcome;
        auto file2 = path/"fuzzyStringMatching"/"object_labels_ells_graphobject_index.txt";
        auto file3 = path/"fuzzyStringMatching"/"object_values_xis_graphobject_index.txt";
        DEBUG_ASSERT(std::filesystem::exists(file2));
        std::ifstream ells(file2);
        std::ifstream xis(file3);
        for (size_t n_graph = 0; n_graph < n_graphs; n_graph++) {
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
    data.init_attributes(path/ "attributes_headers.txt");
    data.init_containment(path/ "containment_headers.txt");
    data.init_activities(path);


    // TODO:

    auto cp = data.getTable("Author");
    for (auto it = cp.first; it != cp.second; it++) {
        gsm_object object;
        data.reconstruct_object(it->graph_id, it->event_id, object);
        object.out_json(std::cout);
        std::cout << std::endl;
    }

//    data.print();
}


int main(void) {
    bool serialize = true;
    if (serialize) {
        DataFormatHandler dfh;
        final_writing(dfh);
//        SimpleStringReader kdl("/home/giacomo/projects/gsm2/secondary_tests/minimal/fuzzyStringMatching/string_attributes");
//        for (size_t i = 0, N = kdl.size(); i<N; i++) {
//            size_t len = kdl.strlen(i);
//            const char* val = kdl.str(i);
//            std::string copy(val, len);
//            std::cout << copy << std::endl;
//        }
//        std::cout << kdl.query_equals("Dan") << std::endl;
    }
    load_database_test("/home/giacomo/projects/gsm2/secondary_tests/minimal");

    return EXIT_SUCCESS;
}
