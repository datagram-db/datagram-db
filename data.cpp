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
    dfh.load_to_secondary_memory("/home/gyankos/Scaricati/osfstorage-archive/Data/Dataset/Schema.txt",
                                 "/home/gyankos/Scaricati/osfstorage-archive/binaries/benchmark",
                                 DataFormat::Schema);
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout<<"loaded and indexed in (s): "<< duration <<'\n';
}

void minimal_writing(DataFormatHandler& dfh) {
    dfh.load_to_secondary_memory("/home/gyankos/Scaricati/osfstorage-archive/Schema.txt",
                                 "/home/gyankos/Scaricati/osfstorage-archive/binaries/minimal",
                                 DataFormat::Schema);
}

#include <yaucl/data/VariadicSizeArrayElements.h>
#include <ndp/MainMemoryIndicesForSecondary.h>

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
    DataFormatHandler dfh;
    minimal_writing(dfh);
    // bool serialize = false;
    // if (serialize) {
    //     DataFormatHandler dfh;
    //     final_writing(dfh);
    // }
//    Q1("/home/giacomo/projects/gsm2/secondary_tests/final");
//    Q2("/home/giacomo/projects/gsm2/secondary_tests/final");
//    Q4("/home/giacomo/projects/gsm2/secondary_tests/final");
    // Q6("/home/giacomo/projects/gsm2/secondary_tests/final");
//    Q5("/home/giacomo/projects/gsm2/secondary_tests/final");
//    Q3("/home/giacomo/projects/gsm2/secondary_tests/final");


    return EXIT_SUCCESS;
}
