#include <easylogging++.h>
INITIALIZE_EASYLOGGINGPP
#include <iostream>
#include <parser/schema/DataLoaderFromSchema.h>

#define DEBUG

#include <ndp/main_memory_index.h>

#include <yaucl/data/SimpleStringSerializer.h>
void final_writing(DataFormatHandler& dfh) {
    std::clock_t start;
    double duration;
    start = std::clock();
    dfh.load_to_secondary_memory("/home/giacomo/Scaricati/Unibench-0.2/Dataset/Schema.txt",
                                 "/home/giacomo/projects/gsm2/secondary_tests/final",
                                 DataFormat::Schema);
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout<<"loaded and indexed in (s): "<< duration <<'\n';
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
#include <ndp/MainMemoryIndicesForSecondary.h>

//
//struct MainMemoryIndicesForSecondary {
//    std::vector<GraphInformation> graph_information;
//    size_t total_objects;
//
//
//
//
//    MainMemoryIndicesForSecondary(const std::filesystem::path& path) : containment_offsets(path / "database_containment_table_si.binary"),
//                                                                       ells(path / "fuzzyStringMatching"/"object_labels_ells"),
//                                                                       xis(path / "fuzzyStringMatching"/"object_values_xis"),
//                                                                       attributes_offsets(path / "database_attributes_table_si.binary"),
//                                                                       database_attributes_table(path / "database_attributes_table.binary"),
//                                                                       database_containment_table(path / "database_containment_table.binary"),
//                                                                       string_attributes(path / "fuzzyStringMatching"/ "string_attributes"),
//                                                                       activity_table(path / "activity_table_data.bin") {
//        init_basics_graphs(path);
//        init_ellxi(path);
//        init_attributes(path/ "attributes_headers.txt");
//        init_containment(path/ "containment_headers.txt");
//        init_activities(path);
//    }
//
//    inline size_t getNGraphs() const { return graph_information.size(); }
//    inline size_t getNObjects() const { return total_objects; }
//    inline size_t getNObjects(size_t graphId) const {
//        if (graph_information.size() <= graphId)
//            return 0;
//        DEBUG_ASSERT(graph_information.at(graphId).n_obj == graph_information.at(graphId).objects.size());
//        return graph_information.at(graphId).n_obj;
//    }
//
//    // Activity Table                                                           ~ activities are the first label occurring for each object
//    std::vector<std::string> activity_list;
//    std::unordered_map<std::string, ActivityEntry> activity_name_to_offset;
//    inline const std::pair<const gsm2::tables::ActivityTable::secmem_record*,const gsm2::tables::ActivityTable::secmem_record*> getTable(const std::string& activity) {
//        auto it = activity_name_to_offset.find(activity);
//        if (it == activity_name_to_offset.end())
//            return {nullptr, nullptr};
//       return {activity_table.begin() + it->second.offset_begin, activity_table.begin() + it->second.offset_end};
//    }
//
//    // Containment tables
//    std::vector<std::string> phi_table_names;
//    std::unordered_map<std::string, ContainmentInformation> containment_information;
//
//    inline const std::vector<std::string>& getContainments() const { return phi_table_names; }
//    inline const size_t numberOfContainments(size_t idx) const {
//        if (idx >= attribute_names.size()) return 0;
//        return numberOfContainments(attribute_names.at(idx));
//    }
//    inline const size_t numberOfContainments(const std::string&name) const {
//        auto it = containment_information.find(name);
//        if (it == containment_information.end()) return 0;
//        return it->second.table_length;
//    }
//    inline size_t countContainments(size_t graph_id, size_t object_id, const std::string& name) const {
//        if (graph_information.size() <= graph_id)
//            return 0;
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= object_id)
//            return 0;
//        auto it = containment_information.find(name);
//        if (it == containment_information.end())
//            return 0;
//        size_t computd_offset = (it->second.containment_id * total_objects + ref.n_obj_offset + object_id)*2;
//        DEBUG_ASSERT(computd_offset < containment_offsets.size());
//        size_t begin_offset = containment_offsets[computd_offset];
//        size_t end_offset = containment_offsets[computd_offset+1];
//        return end_offset - begin_offset;
//    }
//    inline gsm2::tables::PhiTable::secmem_record* getIthContainments(size_t graph_id, size_t object_id, const std::string& name, size_t i) const {
//        if (graph_information.size() <= graph_id)
//            return nullptr;
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= object_id)
//            return nullptr;
//        auto it = containment_information.find(name);
//        if (it == containment_information.end())
//            return nullptr;
//        size_t computd_offset = (it->second.containment_id * total_objects + ref.n_obj_offset + object_id)*2;
//        DEBUG_ASSERT(computd_offset < containment_offsets.size());
//        size_t begin_offset = containment_offsets[computd_offset];
//        size_t end_offset = containment_offsets[computd_offset+1];
//        size_t len = end_offset - begin_offset;
//        if (len <= i)
//            return nullptr;
//        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
//        return ptr + begin_offset + i;
//    }
//    inline void setContainmentToObject(size_t graph_id, size_t object_id, const std::string& name, gsm_object& object) const {
//        if (graph_information.size() <= graph_id)
//            return ;
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= object_id)
//            return ;
//        auto it = containment_information.find(name);
//        if (it == containment_information.end())
//            return ;
//        size_t computd_offset = it->second.secondary_index_offset_header + (ref.n_obj_offset + object_id)*2;
//        DEBUG_ASSERT(computd_offset < containment_offsets.size());
//        size_t begin_offset = containment_offsets[computd_offset];
//        size_t end_offset = containment_offsets[computd_offset+1];
//        size_t len = end_offset - begin_offset;
//        if (len == 0)
//            return;
//        gsm_object_xi_content containment;
//        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
//        for (int i = 0; i<len; i++) {
//            auto idx = containment_offsets[it->second.secondary_index_offset_contents + begin_offset + i];
//            const auto& current = ptr[idx];
//            containment.id = current.id_contained;
//            containment.orig_edge_id = current.instance_id;
//            containment.score = current.w_contained;
//            object.phi[name].emplace_back(containment);
//        }
//    }
//
//    // Attribute table
//    std::vector<std::string> attribute_names;
//    std::unordered_map<std::string, AttributeInformation> attribute_information;
//
//    inline const std::vector<std::string>& getAttributes() const { return attribute_names; }
//    inline const size_t attributeTableLength(size_t idx) const {
//        if (idx >= attribute_names.size()) return 0;
//        return attributeTableLength(attribute_names.at(idx));
//    }
//    inline const size_t attributeTableLength(const std::string& name) const {
//        auto it = attribute_information.find(name);
//        if (it == attribute_information.end()) return 0;
//        return it->second.table_length;
//    }
//    std::optional<union_type> getAttribute(size_t graph_id, size_t object_id, const std::string& name) const {
//        auto cp = hasAttribute(graph_id, object_id, name);
//        if (!cp.second)
//            return {};
//        auto it = attribute_information.find(name);
//        gsm2::tables::AttributeTable::record* ptr = (gsm2::tables::AttributeTable::record*)(database_attributes_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
//#ifdef DEBUG
//        size_t offset_to_displacement = cp.first * sizeof(gsm2::tables::AttributeTable::record*);
//        DEBUG_ASSERT(it->second.secondary_memory_table_records_offset_begin + offset_to_displacement < it->second.secondary_memory_table_records_offset_end);
//#endif
//        auto& x = ptr[cp.first];
//        switch (it->second.type) {
//            case gsm2::tables::AttributeTableType::DoubleAtt:
//                return {*(double*)(&x.value)};
//            case gsm2::tables::AttributeTableType::LongAtt:
//                return {*(long long*)(&x.value)};
//            case gsm2::tables::AttributeTableType::StringAtt:
//                return {std::string{string_attributes.str(x.value),string_attributes.strlen(x.value)} };
//            case gsm2::tables::AttributeTableType::BoolAtt:
//                return {((x.value != 0)) && (x.value != 0.0)};
//                //case SizeTAtt:
//            default:
//                // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
//                return {x.value};
//        }
//    }
//
//    inline std::pair<size_t, bool> hasAttribute(size_t graph_id, size_t object_id, const std::string& name) const {
//        std::pair<size_t, bool> result{-1, false};
//        if (graph_information.size() <= graph_id)
//            return result;
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= object_id)
//            return result;
//        auto it = attribute_information.find(name);
//        if (it == attribute_information.end())
//            return result;
//        size_t computd_offset = it->second.attribute_id * total_objects + ref.objects.at(object_id).activity_table_entry_id_offset;
//        DEBUG_ASSERT(computd_offset < attributes_offsets.size());
//        result.first = attributes_offsets[computd_offset];
//        if (result.first == (size_t)-1) {
//            return result;
//        }
//        result.second = true;
//        return result;
//    }
//    inline void setAttributeToObject(size_t graph_id, size_t object_id, const std::string& name, gsm_object&object) const {
//        std::pair<size_t, bool> result{-1, false};
//        if (graph_information.size() <= graph_id)
//            return;// no update
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= object_id)
//            return;// no update
//        auto it = attribute_information.find(name);
//        if (it == attribute_information.end())
//            return;// no update
//        size_t computd_offset = it->second.attribute_id * total_objects + ref.objects.at(object_id).activity_table_entry_id_offset;
//        DEBUG_ASSERT(computd_offset < attributes_offsets.size());
//        result.first = attributes_offsets[computd_offset];
//        if (result.first == (size_t)-1) {
//            return; // no update
//        }
//        result.second = true;
//        gsm2::tables::AttributeTable::record* ptr = (gsm2::tables::AttributeTable::record*)(database_attributes_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
//#ifdef DEBUG
//        size_t offset_to_displacement = result.first * sizeof(gsm2::tables::AttributeTable::record*);
//        DEBUG_ASSERT(it->second.secondary_memory_table_records_offset_begin + offset_to_displacement < it->second.secondary_memory_table_records_offset_end);
//#endif
//        auto& x = ptr[result.first];
//        switch (it->second.type) {
//            case gsm2::tables::AttributeTableType::DoubleAtt:
//                object.content[name] = *(double*)(&x.value);
//                break;
//            case gsm2::tables::AttributeTableType::LongAtt:
//                object.content[name] = (double)*(long long*)(&x.value);
//                break;
//            case gsm2::tables::AttributeTableType::StringAtt:
//                object.content[name] = std::string{string_attributes.str(x.value),string_attributes.strlen(x.value)};
//                break;
//            case gsm2::tables::AttributeTableType::BoolAtt:
//                object.content[name] = (double)(((x.value != 0)) && (x.value != 0.0));
//                break;
//                //case SizeTAtt:
//            default:
//                // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
//                object.content[name] = (double)x.value;
//                break;
//        }
//    }
//
//
//
//    inline size_t getNumberOfLabels(size_t graph_id, size_t objet_id) const {
//        if (graph_information.size() <= graph_id)
//            return 0;
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= objet_id)
//            return 0;
//        return ref.objects[objet_id].ells.size();
//    }
//
//    inline std::string getIthLabel(size_t graph_id, size_t objet_id, size_t i) const {
//        if (graph_information.size() <= graph_id)
//            return "";
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= objet_id)
//            return "";
//        if (ref.objects[objet_id].ells.size() <= i)
//            return "";
//        return ells.get(ref.objects[objet_id].ells.at(i));
//    }
//
//    inline size_t getNumberOfValues(size_t graph_id, size_t objet_id) const {
//        if (graph_information.size() <= graph_id)
//            return 0;
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= objet_id)
//            return 0;
//        return ref.objects[objet_id].xis.size();
//    }
//
//    inline std::string getIthValue(size_t graph_id, size_t objet_id, size_t i) const {
//        if (graph_information.size() <= graph_id)
//            return "";
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= objet_id)
//            return "";
//        if (ref.objects[objet_id].xis.size() <= i)
//            return "";
//        return xis.get(ref.objects[objet_id].xis.at(i));
//    }
//
//    void  reconstruct_object(size_t graph_id, size_t object_id, gsm_object& object) const {
//        object.clear();
//        object.id = object_id;
//        if (getNGraphs() <= graph_id)
//            return;
//        if (getNObjects(graph_id) <= object_id)
//            return;
//        for (size_t n_label = 0, N = getNumberOfLabels(graph_id, object_id); n_label < N; n_label++) {
//            object.ell.emplace_back(getIthLabel(graph_id, object_id, n_label));
//        }
//        for (size_t n_label = 0, N = getNumberOfValues(graph_id, object_id); n_label < N; n_label++) {
//            object.xi.emplace_back(getIthValue(graph_id, object_id, n_label));
//        }
//        for (const auto& attribute : attribute_names) {
//            setAttributeToObject(graph_id, object_id, attribute, object);
//        }
//        for (const auto& containment : phi_table_names) {
//            setContainmentToObject(graph_id, object_id, containment, object);
//        }
//    }
//
//    void print() {
//            for (size_t graph_id = 0, N = getNGraphs(); graph_id < N; graph_id ++) {
//                for (size_t object_id = 0, M = getNObjects(graph_id); object_id < M; object_id++) {
//                    gsm_object object;
//                    reconstruct_object(graph_id, object_id, object);
//                    object.out_json(std::cout);
//                    std::cout << std::endl;
//                }
//            }
//    }
//
//
//private:
//    yaucl::data::FixedSizeArrayElements<size_t> containment_offsets, attributes_offsets, database_attributes_table, database_containment_table;
//    yaucl::data::FixedSizeArrayElements<gsm2::tables::ActivityTable::secmem_record> activity_table;
//    NDPFuzzyStringMatching ells, xis;
//    SimpleStringReader string_attributes;
//    size_t n_graphs;
//};
//
//void load_database_test(const std::filesystem::path& path) {
//    MainMemoryIndicesForSecondary data(path);
//
//    auto cp = data.getTable("Author");
//    for (auto it = cp.first; it != cp.second; it++) {
//        gsm_object object;
//        data.reconstruct_object(it->graph_id, it->event_id, object);
//        object.out_json(std::cout);
//        std::cout << std::endl;
//    }
//
////    data.print();
//}
//
//class Integers
//{
//public:
//    struct Iterator
//    {
//        using iterator_category = std::forward_iterator_tag;
//        using difference_type   = std::ptrdiff_t;
//        using value_type        = gsm2::tables::PhiTable::secmem_record;
//        using pointer           = gsm2::tables::PhiTable::secmem_record*;
//        using reference         = gsm2::tables::PhiTable::secmem_record&;
//
//        Iterator(pointer ptr) : m_ptr(ptr) {}
//
//        reference operator*() const { return *m_ptr; }
//        pointer operator->() { return m_ptr; }
//        Iterator& operator++() { m_ptr++; return *this; }
//        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
//        friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
//        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };
//
//    private:
//        pointer m_ptr;
//    };
//
//    Iterator begin() { return Iterator(&m_data[0]); }
//    Iterator end()   { return Iterator(&m_data[200]); }
//
//private:
//    int m_data[200];
//};

//void Q0(const std::filesystem::path& path) {
//    MainMemoryIndicesForSecondary data(path);
//
//    auto cp = data.getTable("Order");
//    for (auto it = cp.first; it != cp.second; it++) {
//        const auto order_id = it->event_id;
//        gsm_object rec;
//        data.reconstruct_object(0, order_id, rec);
//        auto cp2 = data.getContainments(0, order_id, "PersonId");
//        rec.out_json(std::cout);
//    }
//}

void Q1(const std::filesystem::path& path) {
    MainMemoryIndicesForSecondary data(path);

    std::clock_t start;
    double duration;
    start = std::clock();

    // resolving the tags of interest into ids (for more efficient comparison and edge consideration)
    ssize_t order = data.activityId("Order");
    ssize_t invoice = data.activityId("Invoice");
    ssize_t feedback = data.activityId("Feedback");
    ssize_t post = data.activityId("Post");
    ssize_t hasCreator = data.activityId("hasCreator");
    ssize_t hasTag = data.activityId("hasTag");

    // For i n in Person
    size_t person_count = 0;
    auto cp = data.getTable("Person");
    for (auto it = cp.first; it != cp.second; it++) {
        person_count++;
        const auto i = it->event_id;

        lightweight_gsm_object object; // new o
        data.import_properties(i, object); // o inheritproperties from i
        object.phi["profile"].emplace_back(i); // \phi(o, "profile) << i
        std::unordered_map<size_t, size_t> counter_tc;
        std::unordered_map<std::string, size_t> counter_bc;

        for (const auto& ingoing_edge : data.getInvContainments(0, i, "to")){
            // ... .src
            size_t stream_0_step1 = ingoing_edge->object_id;
            size_t label_step1 = ingoing_edge->src_label;

            // filter (inv pred)
            if (label_step1 != hasCreator) {
                continue;
            }

//            gsm_object tmp_test, tmp_test2;
//            data.reconstruct_object(0, stream_0_step1, tmp_test);
            for (const auto& outgoing_edge : data.getContainments(0, stream_0_step1, "from")) {
                size_t stream_0_step2 = outgoing_edge->id_contained;
                size_t label_step2 = outgoing_edge->src_label;

                for (const auto& ingoing_edge2 : data.getInvContainments(0, stream_0_step2, "from")) {
                    size_t stream_0_step3 = ingoing_edge2->object_id;
                    size_t label_step3 = ingoing_edge2->src_label;

                    // filter (inv pred)
                    if (label_step3 == hasCreator) {
                        continue;
                    }

                    auto tag_ptr = data.getUintAttribute(0, stream_0_step3, "tag");
                    if (tag_ptr != nullptr) {
                        auto itval = counter_tc.emplace(*tag_ptr, 1);
                        if (!itval.second) itval.first->second++;
                    }
                }
            }
        }

        // (i<-PersonId). ...
        for (const auto& ingoing_edge : data.getInvContainments(0, i, "PersonId")) {
            // ... .src
            size_t stream_1 = ingoing_edge->object_id;
            size_t label = ingoing_edge->src_label;

            // multiSink
            if (label == order) {
                object.phi["orders"].emplace_back(stream_1);
                for (const auto& l_edge : data.getContainments(0, stream_1, "Orderline")) {
                    auto brand = data.getStringAttribute(0, l_edge->id_contained, "brand");
                    if (brand != nullptr) {
                        auto itval = counter_bc.emplace(brand, 1);
                        if (!itval.second) itval.first->second++;
                    }
                }
            } else if (label == invoice) {
                object.phi["invoices"].emplace_back(stream_1);
            } else if (label == feedback) {
                object.phi["feedbacks"].emplace_back(stream_1);
            }else if (label == post) {
                object.phi["posts"].emplace_back(stream_1);
            }
        }

        // maxArg
        size_t countMaxTc = 0;
        std::vector<size_t> result_tc;
        for (const auto& [key, value]: counter_tc) {
            if (countMaxTc < value) {
                result_tc.clear();
                countMaxTc = value;
            }
            if (countMaxTc == value) {
                result_tc.emplace_back(key);
            }
        }
        auto s = std::accumulate(result_tc.begin(), result_tc.end(), std::string(),
                                 [](const std::string& x, const size_t& val) {
                                     // TODO: to actual string, not like this
                                     return x + " "+ std::to_string((val));
                                 });
        // Storing it in the properties
        object.pi["bestTag"] = s;

        size_t countMaxBc = 0;
        std::vector<std::string> result_bc;
        for (const auto& [key, value]: counter_bc) {
            if (countMaxBc < value) {
                result_bc.clear();
                countMaxBc = value;
            }
            if (countMaxBc == value) {
                result_bc.emplace_back(key);
            }
        }
        s = std::accumulate(result_bc.begin(), result_bc.end(), std::string(),
                                 [](const std::string& x, const union_type& val) {
                                                     // TODO: to actual string, not like this
                                     return x + " "+ std::get<std::string>(val);
                                 });
        object.pi["bestBrands"] = s;
    }
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout<<"Q1 in (s): "<< duration <<'\n';
    std::cout<<"Q1 time per person (s): "<< duration/((double)person_count) <<'\n';
}


void Q2(const std::filesystem::path& path) {
    MainMemoryIndicesForSecondary data(path);

    std::clock_t start;
    double duration;
    start = std::clock();

    // resolving the tags of interest into ids (for more efficient comparison and edge consideration)
    ssize_t order = data.activityId("Order");
    ssize_t invoice = data.activityId("Invoice");
    ssize_t feedback = data.activityId("Feedback");
    ssize_t post = data.activityId("Post");
    ssize_t hasCreator = data.activityId("hasCreator");
    ssize_t hasTag = data.activityId("hasTag");

    // For i n in Person
    auto cp = data.getTable("Product");
    size_t count = 0;
    for (auto it = cp.first; it != cp.second; it++) {
        const auto p = it->event_id;
        count++;

        lightweight_gsm_object object; // new o
        object.pi["id"] = (double)p;
//        data.import_properties(p, object); // o inheritproperties from i
        const auto title_ptr = data.getStringAttribute(0, it->event_id, "title");
        std::unordered_set<size_t> final_result; // bought and commented people

        for (const auto& ingoing_edge_1 : data.getInvContainments(0, p, "productId")) {
            size_t stream_0_step1 = ingoing_edge_1->object_id; // orderline.
            bool foundUser = false;

            for (const auto& ingoing_edge_2 : data.getInvContainments(0, stream_0_step1, "Orderline")) {
                size_t stream_0_step2 = ingoing_edge_2->object_id;
                if (foundUser) break;

                for (const auto& p_edge : data.getContainments(0, stream_0_step2, "PersonId")) {
                    size_t stream_0_step3 = p_edge->id_contained;
                    foundUser = final_result.contains(stream_0_step3);
                    if (foundUser) break;

                    // person
                    for (const auto& ingoing_edge_3 : data.getInvContainments(0, stream_0_step3, "to")){
                        // ... .src
                        if (foundUser) break;
                        size_t stream_0_step4 = ingoing_edge_3->object_id;
                        size_t label_step4 = ingoing_edge_3->src_label;

                        // filter (inv pred)
                        if (label_step4 != hasCreator) {
                            continue;
                        }

                        for (const auto& outgoing_edge : data.getContainments(0, stream_0_step4, "from")) {
                            size_t stream_0_step5 = outgoing_edge->id_contained;
                            size_t label_step5 = outgoing_edge->src_label;
                            if (foundUser) break;

                            auto content_ptr = data.getStringAttribute(0, stream_0_step5, "content");
                            if (content_ptr != nullptr && strstr(content_ptr, title_ptr)) {
                                final_result.emplace(stream_0_step3);
                                foundUser = true;
                            }
                        }
                    }
                }
            }
        }

    }
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout<<"Q2 in (s): "<< duration <<'\n';
    std::cout<<"Q2 time per product (s): "<< duration/((double)count) <<'\n';
}



void Q3(const std::filesystem::path& path) {
    MainMemoryIndicesForSecondary data(path);

    std::clock_t start;
    double duration;
    start = std::clock();

    // resolving the tags of interest into ids (for more efficient comparison and edge consideration)
    ssize_t order = data.activityId("Order");
    ssize_t invoice = data.activityId("Invoice");
    ssize_t feedback = data.activityId("Feedback");
    ssize_t post = data.activityId("Post");
    ssize_t hasCreator = data.activityId("hasCreator");
    ssize_t hasTag = data.activityId("hasTag");

    // For i n in Person
    auto cp1 = data.getTable("Product");
    size_t count_product = 0;
    size_t produt_len = cp1.second-cp1.first;
    for (auto product = cp1.first; product != cp1.second; product++) {
        count_product++;
        std::cout << count_product << " out of " << produt_len << std::endl;
        const auto title_ptr = data.getStringAttribute(0, product->event_id, "title");
//        const auto& title = std::get<std::string>(data.getAttribute(0, product->event_id, "title").value());

        std::vector<size_t> final_result;
        auto cp2 = data.getTable("Person");
//        std::cout << "person len: " << cp2.second - cp2.first << std::endl;
        for (auto person = cp2.first; person != cp2.second; person++) {
            bool foundUser = false;
            for (const auto& ingoing_edge_3 : data.getInvContainments(0, person->event_id, "to")){
                // ... .src
                if (foundUser) break;
                size_t stream_0_step4 = ingoing_edge_3->object_id;
                size_t label_step4 = ingoing_edge_3->src_label;

                // filter (inv pred)
                if (label_step4 != hasCreator) {
                    continue;
                }

//            gsm_object tmp_test, tmp_test2;
//            data.reconstruct_object(0, stream_0_step1, tmp_test);
                for (const auto& outgoing_edge : data.getContainments(0, stream_0_step4, "from")) {
                    size_t stream_0_step5 = outgoing_edge->id_contained;
                    size_t label_step5 = outgoing_edge->src_label;
                    if (foundUser) break;

                    auto content_ptr = data.getStringAttribute(0, stream_0_step5, "content");
                    if (content_ptr != nullptr && strstr(content_ptr, title_ptr)) {
                        final_result.emplace_back(person->event_id);
                        foundUser = true;
                    }
                }
            }
        }

    }
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout<<"Q3 in (s): "<< duration <<'\n';
    std::cout<<"Q3 time per product (s): "<< (duration)/((double)count_product) <<'\n';

}



void Q4(const std::filesystem::path& path) {
    MainMemoryIndicesForSecondary data(path);

    std::clock_t start;
    double duration;
    start = std::clock();

    // resolving the tags of interest into ids (for more efficient comparison and edge consideration)
    ssize_t order = data.activityId("Order");
    ssize_t invoice = data.activityId("Invoice");
    ssize_t feedback = data.activityId("Feedback");
    ssize_t post = data.activityId("Post");
    ssize_t hasCreator = data.activityId("hasCreator");
    ssize_t hasTag = data.activityId("hasTag");

    // For i n in Person
    auto cp1 = data.getTable("Person");
    size_t count_person = 0;

    // For retrieving the maximum 2
    std::map<double, std::vector<size_t>> aggrPerPerson;
    for (auto person = cp1.first; person != cp1.second; person++) {
        auto i = person->event_id;
        count_person++;
//        std::cout << count_product << " out of " << produt_len << std::endl;
        const auto title_ptr = data.getStringAttribute(0, person->event_id, "title");
//        const auto& title = std::get<std::string>(data.getAttribute(0, product->event_id, "title").value());

        // Sum price per invoice: aggregation
        double totalPrice = 0;
        for (const auto& ingoing_edge : data.getInvContainments(0, i, "PersonId")) {
            size_t stream_0_step2 = ingoing_edge->object_id;
            size_t label_step2 = ingoing_edge->src_label;

            // filter (inv pred)
            if (label_step2 != invoice) {
                continue;
            }

            auto val = data.getDoubleAttribute(0, stream_0_step2, "TotalPrice");
            if (val != nullptr) {
                totalPrice += *val;
            }
        }
        aggrPerPerson[totalPrice].emplace_back(i);
    }

    // Greatest 2 users
    size_t maxOf = 2;
    std::vector<size_t> allUsers;
    auto it = aggrPerPerson.rbegin(), en = aggrPerPerson.rend();
    while ((allUsers.size() < maxOf) && (it != en)) {
        allUsers.insert(allUsers.end(), it->second.begin(), it->second.end());
        it++;
    }
    QuickGraph g;
    data.fillObjectsByEll0AsEdges(0, g, "knows", "from", "to");
    g.build();
    auto kHopSrc = g.kHop(allUsers[0], 3);
    auto kHopDst = g.kHop(allUsers[1], 3);
    std::sort(kHopSrc.begin(), kHopSrc.end());
    std::sort(kHopDst.begin(), kHopDst.end());
    auto end = std::set_intersection(
            kHopSrc.begin(), kHopSrc.end(),
            kHopDst.begin(), kHopDst.end(),
            kHopSrc.begin() // intersection is written in set_1
    );
    kHopSrc.erase(end, kHopSrc.end());
    // Result: kHopSrc
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout<<"Q4 in (s) -- [total]: "<< duration <<'\n';
}

void Q5(const std::filesystem::path& path) {
    MainMemoryIndicesForSecondary data(path);

    std::clock_t start;
    double duration;
    start = std::clock();

    // resolving the tags of interest into ids (for more efficient comparison and edge consideration)
    ssize_t order = data.activityId("Order");
    ssize_t invoice = data.activityId("Invoice");
    ssize_t feedback = data.activityId("Feedback");
    ssize_t post = data.activityId("Post");
    ssize_t hasCreator = data.activityId("hasCreator");
    ssize_t hasTag = data.activityId("hasTag");

    // Prepare knows graph index
    QuickGraph g;
    data.fillObjectsByEll0AsEdges(0, g, "knows", "from", "to");
    g.build();

    // aggregate products into a set
    auto cp0 = data.getTable("BrandByProduct");
    std::unordered_set<std::string> brands;
    for (auto brandToObj = cp0.first; brandToObj != cp0.second; brandToObj++) {
        auto brand = std::string(data.getStringAttribute(brandToObj->graph_id, brandToObj->event_id, "brand"));
        brands.emplace(brand);
    }

    // For i n in Person
    auto cp1 = data.getTable("Person");
    size_t count_person_brand = 0;
    for (auto person = cp1.first; person != cp1.second; person++) {
        auto kHopSrc = g.kHop(person->event_id, 3);

        for (const auto& brand_name : brands) {
            count_person_brand++;
            std::cout << count_person_brand << std::endl;
            std::vector<size_t> friend_result;

            for (const auto& friend_id : kHopSrc) {
                bool friendFound = false;
                for (const auto& ingoing_edge : data.getInvContainments(0, friend_id, "PersonId")) {
                    // ... .src
                    size_t stream_1 = ingoing_edge->object_id;
                    size_t label = ingoing_edge->src_label;

                    if (label != invoice)
                        continue;

                    for (const auto& l_edge : data.getContainments(0, stream_1, "Orderline")) {
                        auto brand = std::string(data.getStringAttribute(0, l_edge->id_contained, "brand"));
                        if (brand == brand_name) {
                            friend_result.emplace_back(friend_id);
                            friendFound = true;
                            break;
                        }
                    }
                    if (friendFound) break;
                }
            }
        }
    }

    // Result: kHopSrc
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout<<"Q5 in (s): "<< duration <<'\n';
    std::cout<<"Q5 time per person*brand (s): "<< (duration)/((double)count_person_brand) <<'\n';
}

#include <lemon/dijkstra.h>

void Q6(const std::filesystem::path& path) {
    MainMemoryIndicesForSecondary data(path);

    std::clock_t start;
    double duration;
    start = std::clock();

    // resolving the tags of interest into ids (for more efficient comparison and edge consideration)
    ssize_t order = data.activityId("Order");
    ssize_t invoice = data.activityId("Invoice");
    ssize_t feedback = data.activityId("Feedback");
    ssize_t post = data.activityId("Post");
    ssize_t hasCreator = data.activityId("hasCreator");
    ssize_t hasTag = data.activityId("hasTag");

    // Prepare knows graph index
    QuickGraph g;
    data.fillObjectsByEll0AsEdges(0, g, "knows", "from", "to");
    g.build();

    // For i n in Person
    auto cp1 = data.getTable("Person");
    size_t count_person_person = 0;
    for (auto person = cp1.first; person != cp1.second; person++) {
        for (auto person2 = cp1.first; person2 != cp1.second; person2++) {
            if (person == person2) continue;
            count_person_person++;
            std::cout << count_person_person << std::endl;

            auto friend_path = g.dijkstra(person->event_id, person2->event_id);
            if (friend_path.size() <= 1)
                continue;
            std::unordered_map<std::string, size_t> counter_bc;
            for (const auto& friend_id : friend_path) {


                for (const auto& ingoing_edge : data.getInvContainments(0, friend_id, "PersonId")) {
                    // ... .src
                    size_t stream_1 = ingoing_edge->object_id;
                    size_t label = ingoing_edge->src_label;

                    if (label != invoice)
                        continue;

                    for (const auto& l_edge : data.getContainments(0, stream_1, "Orderline")) {
                        auto brand = data.getStringAttribute(0, l_edge->id_contained, "brand");
                        if (brand != nullptr) {
                            auto itval = counter_bc.emplace(brand, 1);
                            if (!itval.second) itval.first->second++;
                        }
                    }
                }
            }

            std::map<size_t, std::vector<std::string>> counter_bc_rev;
            for (const auto& [k, v] : counter_bc)
                counter_bc_rev[v].emplace_back(k);

            // Top3 BC
            size_t maxOf = 3;
            std::vector<std::string> allUsers;
            auto it = counter_bc_rev.rbegin(), en = counter_bc_rev.rend();
            while ((allUsers.size() < maxOf) && (it != en)) {
                allUsers.insert(allUsers.end(), it->second.begin(), it->second.end());
                it++;
            }

//            std::cout << "here" << std::endl;
        }
    }


    // Result: kHopSrc
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout<<"Q6 in (s): "<< duration <<'\n';
    std::cout<<"Q6 time per person*brand (s): "<< (duration)/((double)count_person_person) <<'\n';
}

int main(void) {
    bool serialize = false;
    if (serialize) {
        DataFormatHandler dfh;
        final_writing(dfh);
    }
//    Q1("/home/giacomo/projects/gsm2/secondary_tests/final");
//    Q2("/home/giacomo/projects/gsm2/secondary_tests/final");
//    Q4("/home/giacomo/projects/gsm2/secondary_tests/final");
    Q6("/home/giacomo/projects/gsm2/secondary_tests/final");
//    Q5("/home/giacomo/projects/gsm2/secondary_tests/final");
//    Q3("/home/giacomo/projects/gsm2/secondary_tests/final");


    return EXIT_SUCCESS;
}
