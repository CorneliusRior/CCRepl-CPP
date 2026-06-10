add_executable(CCReplCli
    CCRepl.CLI.cpp
)

target_link_libraries(CCReplCli
    PRIVATE
        CCRepl    
)

target_compile_features(CCReplCli
    PRIVATE
        cxx_std_20
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(CCReplCli PRIVATE /EHsc)
elseif(MSVC)
    target_compile_options(CCReplCli PRIVATE /EHsc)
endif()