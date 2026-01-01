cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(gsm2)
set(BUILD_TESTING OFF)
set(SIMDCSV_SANITIZE ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-fconstexpr-depth=5023 -m64")
cmake_policy(SET CMP0067 NEW)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/submodules/lemon-1.3.1)
include_directories(submodules/lemon-1.3.1)
include_directories(submodules/yaucl/submodules/magic_enum/include)
add_subdirectory(submodules/lemon-1.3.1)
add_subdirectory(submodules/capnproto)
add_subdirectory(submodules/simdjson)
include_directories(submodules/simdjson/include)
include_directories(submodules/capnproto/c++/src/)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
#set(USE_COROUTINES ON CACHE BOOL "" FORCE)
#set(USE_FOLLY OFF CACHE BOOL "" FORCE)
set(ANTLR_BUILD_CPP_TESTS OFF)
find_package(Threads)
set(CMAKE_C_COMPILER clang-19 CACHE INTERNAL "C compiler" )
set(CMAKE_CXX_COMPILER clang++-19 CACHE INTERNAL "C++ compiler")
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES "/usr/local/gcc-14.2.0/include/")
        #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
SET(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,/usr/local/gcc-14.2.0/lib/../lib64")
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(submodules/easylogging/src)
add_subdirectory(submodules/easylogging)
include_directories(submodules/rapidjson/include)
Include(FetchContent)
#add_subdirectory(submodules/stxxl)
#include_directories(submodules/stxxl/include)
#FetchContent_Declare(
#        Catch2
#        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#        GIT_TAG        v3.0.1 # or a later release
#)
#FetchContent_MakeAvailable(Catch2)
#list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
#include(Catch)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message(WARNING "Compiling in debug mode")
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
else()
    message(WARNING "Compiling in optimized mode. Level=2")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

include_directories(submodules/args)
include_directories(include)
include_directories(antlr4)
include_directories(submodules/yaucl/submodules/magic_enum/include)
include_directories(submodules/yaucl/include)
include_directories(submodules/yaucl/submodules/json/include)
include_directories(submodules/yaucl/submodules/CRoaring/cpp)
include_directories(submodules/yaucl/submodules/CRoaring/include)
add_subdirectory(submodules/yaucl/submodules/CRoaring)
add_subdirectory(submodules/yaucl/submodules/antlr4/runtime/Cpp)
include_directories(submodules/yaucl/submodules/antlr4/runtime/Cpp/runtime/src)
include_directories(submodules/yaucl/antlr4/cpp/antlr4)
include_directories(submodules/tabulate/include)
include_directories(submodules/yaucl/submodules/rocksdb/include)
add_subdirectory(submodules/yaucl/submodules/rocksdb)

#include_directories(submodules/sonicsv)
#add_library(sonicsv submodules/sonicsv.cpp)
#include_directories(submodules/zsv_includes)
#add_library(zsvlib submodules/zsv/src/vector_delim.c
#        submodules/zsv/src/zsv.c
#        submodules/zsv/src/zsv_internal.c
#        submodules/zsv/src/zsv_scan_delim.c
#        submodules/zsv/src/zsv_scan_fixed.c
#        submodules/zsv/src/zsv_strencode.c)

#<<<<<<< HEAD
add_library(yaucl_hashing OBJECT
        submodules/yaucl/include/yaucl/hashing/evaluateHashes.h
        submodules/yaucl/src/yaucl/hashing/evaluateHashes.c
        submodules/yaucl/src/yaucl/hashing/hash_combine.cpp
        submodules/yaucl/include/yaucl/hashing/hash_combine.h
        submodules/yaucl/src/yaucl/hashing/hashing.cpp
        submodules/yaucl/include/yaucl/hashing/hashing.h
        submodules/yaucl/include/yaucl/hashing/pair_hash.h
        submodules/yaucl/include/yaucl/hashing/vector_hash.h
        submodules/yaucl/include/yaucl/hashing/uset_hash.h)
target_include_directories(yaucl_hashing PUBLIC ${CMAKE_SOURCE_DIR}/include/yaucl/hashing/ ${CMAKE_CURRENT_BINARY_DIR})

add_library(yaucl_data OBJECT
        submodules/yaucl/src/yaucl/data/csv/csv.cpp
        submodules/yaucl/src/yaucl/data/new_iovec.cpp
        submodules/yaucl/src/yaucl/data/MemoryMappingFile.cpp
        submodules/yaucl/src/yaucl/data/mmapFile.cpp
        submodules/yaucl/src/yaucl/data/VariadicSizeArrayElements.cpp
        submodules/yaucl/src/yaucl/data/VarSizeNDPSorter.cpp
        submodules/yaucl/src/yaucl/data/NDPFuzzyStringMatching.cpp
        submodules/yaucl/src/yaucl/data/SimpleStringSerializer.cpp
        submodules/yaucl/src/yaucl/data/xml.cpp
        submodules/yaucl/include/yaucl/data/json.h
        submodules/yaucl/src/yaucl/data/VarSizeNDPSorter.cpp
        submodules/yaucl/src/yaucl/data/malloc_freelist.c
        submodules/yaucl/src/yaucl/data/SecondaryMemoryMalloc.cpp
        submodules/yaucl/include/yaucl/data/SecondaryMemoryMalloc.h
        submodules/yaucl/include/yaucl/data/smart_malloc.h
        submodules/yaucl/src/yaucl/data/smart_malloc.cpp
        submodules/yaucl/include/yaucl/data/smart_index_pointer.h
        submodules/yaucl/src/yaucl/data/smart_index_pointer.cpp)


add_library(yaucl_structures OBJECT
        submodules/yaucl/include/yaucl/structures/setoids/set_comparators.h
        submodules/yaucl/include/yaucl/structures/setoids/set_partitioning.h
        submodules/yaucl/include/yaucl/structures/setoids/basics.h
        submodules/yaucl/include/yaucl/structures/indexed/BlockOfRecordsIndexedTable.h
        submodules/yaucl/src/yaucl/structures/RoaringBitmapWrapper.cpp
        submodules/yaucl/src/yaucl/structures/NAryTree.cpp
        submodules/yaucl/include/yaucl/structures/NAryTree.h
        submodules/yaucl/include/yaucl/structures/RelationalTables.h)
target_link_libraries(yaucl_structures roaring)


add_library(yaucl_graph OBJECT
        submodules/yaucl/src/yaucl/graphs/adjacency_graph.cpp
        submodules/yaucl/include/yaucl/graphs/adjacency_graph.h
        submodules/yaucl/include/yaucl/graphs/FlexibleGraph.h
        submodules/yaucl/include/yaucl/graphs/NodeLabelBijectionGraph.h
        submodules/yaucl/src/yaucl/graphs/graph_join_pm_algorithms.cpp
        submodules/yaucl/src/yaucl/graphs/graph_join_pm_conversion.cpp
        submodules/yaucl/include/yaucl/graphs/NodeLabelBijectionFA.h
        submodules/yaucl/src/yaucl/graphs/adjacency_entry.cpp
        submodules/yaucl/include/yaucl/graphs/adjacency_entry.h
        submodules/yaucl/src/yaucl/graphs/graph_join_pm.cpp
        submodules/yaucl/include/yaucl/graphs/graph_join_pm.h
        submodules/yaucl/include/yaucl/graphs/graph_join_pm.h
        submodules/yaucl/src/yaucl/graphs/graph_join_pm_conversion.cpp
        submodules/yaucl/src/yaucl/graphs/graph_join_pm_algorithms.cpp
        submodules/yaucl/src/yaucl/graphs/minimizeDFA.cpp
        submodules/yaucl/include/yaucl/graphs/algorithms/minimizeDFA.h
        submodules/yaucl/include/yaucl/graphs/algorithms/all_cycles.h
        submodules/yaucl/include/yaucl/graphs/algorithms/AllDirectedPaths.h
        submodules/yaucl/src/yaucl/graphs/algorithms/prim.cpp
        submodules/yaucl/include/yaucl/graphs/algorithms/prim.h
        submodules/yaucl/src/yaucl/graphs/algorithms/connected_components.cpp
        submodules/yaucl/include/yaucl/graphs/algorithms/connected_components.h
        )
target_link_libraries(yaucl_graph  yaucl_hashing roaring)

add_library(yaucl_ndp OBJECT submodules/yaucl/src/yaucl/data/RocksDBMap.cpp submodules/yaucl/include/yaucl/data/RocksDBMap.h)
target_link_libraries(yaucl_ndp snappy bz2  z rt rocksdb)

add_library(gsm_load OBJECT antlr4/schema_language/schemaBaseListener.cpp
        antlr4/schema_language/schemaBaseVisitor.cpp
        antlr4/schema_language/schemaLexer.cpp
        antlr4/schema_language/schemaListener.cpp
        antlr4/schema_language/schemaParser.cpp
        antlr4/schema_language/schemaVisitor.cpp)

add_library(gsm_script OBJECT ./antlr4/scriptv2/scriptBaseListener.cpp
        ./antlr4/scriptv2/scriptBaseVisitor.cpp
        ./antlr4/scriptv2/scriptLexer.cpp
        ./antlr4/scriptv2/scriptParser.cpp
        ./antlr4/scriptv2/scriptVisitor.cpp
        ./antlr4/scriptv2/scriptListener.cpp
        ./src/scriptv2/ScriptAST.cpp ./include/scriptv2/ScriptAST.h ./src/scriptv2/Funzione.cpp ./include/scriptv2/Funzione.h ./src/scriptv2/Javification.cpp ./include/scriptv2/Javification.h ./src/scriptv2/ScriptVisitor.cpp ./include/scriptv2/ScriptVisitor.h)
target_link_libraries(gsm_script antlr4_static)

add_library(gsm2 OBJECT antlr4/graph_grammar/simple_graph_grammarLexer.cpp antlr4/graph_grammar/simple_graph_grammarParser.cpp antlr4/graph_grammar/simple_graph_grammarListener.cpp  antlr4/graph_grammar/simple_graph_grammarVisitor.cpp src/database/PhiTable.cpp include/database/PhiTable.h src/database/AttributeTable.cpp include/database/AttributeTable.h src/database/SimplifiedFuzzyStringMatching.cpp include/database/SimplifiedFuzzyStringMatching.h src/database/LinearGSM.cpp include/database/LinearGSM.h src/database/ActivityTable.cpp include/database/ActivityTable.h src/result.cpp include/result.h src/database/gsm_indices.cpp include/database/gsm_indices.h src/queries/DataQuery.cpp include/queries/DataQuery.h src/database/gsm_object_xi_content.cpp include/database/gsm_object_xi_content.h include/simple_pair_hash.h src/queries/DataPredicate.cpp include/queries/DataPredicate.h
        #src/database/AtomizingPipeline2.cpp include/database/AtomizingPipeline2.h
        submodules/easylogging/src/easylogging++.cc submodules/easylogging/src/easylogging++.h
        src/database/GSMPatternVisitor.cpp include/database/GSMPatternVisitor.h src/database/GraphEdgeMatchTable.cpp include/database/GraphEdgeMatchTable.h src/database/SimpleTable.cpp include/database/SimpleTable.h include/database/gsm_object.h src/database/gsm_object.cpp include/database/gsm_inmemory_db.h src/database/gsm_inmemory_db.cpp include/database/utility.h src/queries/preserve_results.cpp include/queries/preserve_results.h src/queries/delta_updates.cpp include/queries/delta_updates.h src/queries/closure.cpp include/queries/closure.h src/database/GSMIso.cpp include/database/GSMIso.h include/parsing.h include/configuration/base.h src/configuration/Serialisation.cpp include/configuration/Serialisation.h src/configuration/Configuration.cpp include/configuration/Configuration.h src/configuration/Environment.cpp include/configuration/Environment.h
#        src/database/gsm_object_xi_content.cpp  src/database/gsm_object.cpp
        src/database/ndp/ndp_batch.cpp
        #        include/database/ndp/NDPDatagramDB.h
        #        src/database/ndp/NDPDatagramDB.cpp
        include/parser/schema.h
        src/parser/schema.h
        include/parser/schema/Field.h
        include/parser/schema/Entity.h
        include/parser/xml.h
        src/parser/xml.cpp
        include/parser/parser.h
        src/parser/readers/GSMReader.cpp
        include/parser/readers/GSMReader.h
        include/parser/commons.h
        include/parser/writer.h
        include/parser/writers/GSMWriter.h
        src/parser/writers/GSMWriter.cpp
        include/parser/writers/PrimaryMemoryLoader.h
        src/parser/writers/PrimaryMemoryLoader.cpp
        include/parser/readers/SchemaReader.h
        src/parser/readers/SchemaReader.cpp
        include/parser/schema/NestingState.h
        src/database/SchemaIndexer.cpp include/database/SchemaIndexer.h
        src/parser/schema/NestingState.cpp
        src/parser/schema/DataLoaderFromSchema.cpp
        include/parser/schema/DataLoaderFromSchema.h
        include/parser/readers/BulkReader.h
        src/parser/readers/BulkReader.cpp
        include/parser/writers/BulkWriter.h
        src/parser/writers/BulkWriter.cpp
        src/parser/commons.cpp
        include/parser/readers/PrimaryMemoryReader.h
        src/parser/readers/PrimaryMemoryReader.cpp
        include/parser/writers/CallBackWriter.h
        src/parser/writers/CallBackWriter.cpp
        include/parser/RandomAccessReader.h
        src/parser/ra_readers/RandomAccessBulkreader.cpp
        include/parser/ra_readers/RandomAccessGSMReader.h
        src/parser/ra_readers/RandomAccessGSMReader.cpp
        src/ndp/MainMemoryIndicesForSecondary.cpp
)
target_link_libraries(gsm2 yaucl_data antlr4_static gsm_load gsm_script yaucl_structures roaring  yaucl_hashing yaucl_graph )

add_executable(gsm2_server main.cpp src/queries/NestedResultTable.cpp include/queries/NestedResultTable.h)
target_link_libraries(gsm2_server antlr4_static yaucl_structures roaring  yaucl_hashing yaucl_graph  gsm2  gsm_script antlr4_static yaucl_structures roaring  yaucl_hashing yaucl_graph yaucl_data ${LIBXML2_LIBRARIES} snappy bz2  z rt rocksdb yaucl_ndp gsm_load CapnProto::kj CapnProto::capnp)#  antlr4_static yaucl_structures roaring  yaucl_hashing yaucl_graph  gsm2 gsm_script)

include_directories(submodules/yaucl/submodules/rocksdb/include)
include_directories(submodules/cpp-lru-cache/include)

add_executable(data_loader_test data.cpp
        submodules/yaucl/src/yaucl/data/csv/zsv_cpp.c
)
#sudo apt-get install libsnappy-dev libgflags-dev
#target_link_libraries(data_loader_test snappy bz2  z rt ${ROCKSDB_STATIC_LIB} yaucl_ndp)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STXXL_CXX_FLAGS}")

# add STXXL includes path
#include_directories(${STXXL_INCLUDE_DIRS})
target_link_libraries(data_loader_test zsv simdjson lemon yaucl_data antlr4_static yaucl_structures roaring  yaucl_hashing yaucl_graph  gsm2  gsm_script antlr4_static yaucl_structures roaring  yaucl_hashing yaucl_graph yaucl_data ${LIBXML2_LIBRARIES} snappy bz2  z rt rocksdb yaucl_ndp gsm_load CapnProto::kj CapnProto::capnp)#

add_executable(zsv_wrapper zsv_wrapper.cpp
        submodules/yaucl/src/yaucl/data/csv/zsv_cpp.c)
target_link_libraries(zsv_wrapper zsv)

#enable_testing ()
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib)
#include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})


#add_executable(gsm_gsql_script src/tests/script_runs.cpp)
#target_link_libraries(gsm_gsql_script gtest_main gsm_script gsm2)
#gtest_discover_tests(gsm_gsql_script)
#target_link_libraries(gsm_gsql_script PRIVATE  gsm_script gsm2) #Catch2::Catch2WithMain
#catch_discover_tests(gsm_gsql_script)
#=======
#add_library(gsm_gsql src/gsql_gsm/gsm_object_xi_content.cpp include/gsql_gsm/gsm_object_xi_content.h src/gsql_gsm/gsm_object.cpp include/gsql_gsm/gsm_object.h src/gsql_gsm/gsm_inmemory_db.cpp include/gsql_gsm/gsm_inmemory_db.h include/gsql_gsm/gsql_operators.h src/gsql_gsm/dump_to_xml.cpp include/gsql_gsm/dump_to_xml.h src/gsql_gsm/gsm_db_indices.cpp include/gsql_gsm/gsm_db_indices.h include/gsql_gsm/igc_operators.h include/gsql_gsm/json_operators.h)
#target_link_libraries(gsm_gsql CURL::libcurl yaucl_graph yaucl_hashing roaring antlr4_static tinyxml2)
#
#add_executable(gsm_gsql_test main.cpp include/gsql_gsm/json_operators.h)
#target_link_libraries(gsm_gsql_test CURL::libcurl yaucl_graph yaucl_hashing roaring antlr4_static tinyxml2 gsm_gsql )
#>>>>>>> cb6185cf7859c5b5759ca7d570453d1da49bd026
