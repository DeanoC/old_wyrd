cmake_minimum_required( VERSION 2.8 )
set(CMAKE_CXX_STANDARD 11)

SET( cpp_src_files binify.cpp parser.cxx scanner.cpp scanner.cxx)
file( GLOB header_files *.h *.hpp *.hxx *.hh )

list( APPEND src_files ${cpp_src_files} ${c_src_files} )

add_library( binify ${src_files} ${header_files} )

# make a nice hierachy
set_target_properties( binify PROPERTIES FOLDER "Libs")
