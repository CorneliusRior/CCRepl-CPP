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