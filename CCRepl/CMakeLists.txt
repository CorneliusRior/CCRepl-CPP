add_library(CCRepl STATIC
    pch.cpp
    CCRepl.cpp
    BaseCommands.cpp
    CommandArgs.cpp
    CommandBuilder.cpp
    IArgSpec.cpp
    ReplCommand.cpp
    ReplContext.cpp
    ScptCommands.cpp
    Script.cpp
    ScriptService.cpp
    TestCommands.cpp
    Tokenizers.cpp
)

target_include_directories(CCRepl
    PUBLIC
		${CMAKE_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(CCRepl
    PUBLIC
        util
)

target_compile_features(CCRepl
    PUBLIC
        cxx_std_20
)