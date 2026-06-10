add_library(util STATIC
	pch.cpp
	util.cpp
	fmt.cpp
	ObjTbl.cpp
	parsers.cpp
	str.cpp
)

target_include_directories(util
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/../include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(util
	PUBLIC
		cxx_std_20
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(util PRIVATE /EHsc)
elseif(MSVC)
    target_compile_options(util PRIVATE /EHsc)
endif()