cmake_minimum_required(VERSION 3.21)

project(CCReplWorkspace LANGUAGES CXX)

option(CCREPL_BUILD_CLI
	"Build the CCRepl development CLI"
	${PROJECT_IS_TOP_LEVEL}
)

add_subdirectory(util)
add_subdirectory(CCRepl)

if(CCREPL_BUILD_CLI)
	add_subdirectory(CCRepl.Cli)
endif()
