project(morphene_eos)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( morphene_eos morphene_eos morphene_eos.cpp )
target_include_directories( morphene_eos PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( morphene_eos ${CMAKE_SOURCE_DIR}/../ricardian )