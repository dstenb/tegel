BISON_TARGET(Parser parser.y parser.cpp)
FLEX_TARGET(Scanner lexical.l lexical.cpp)

set (SRC_FILES main.cpp ast.cpp
	bash_backend.cpp
	common.cpp
	constant.cpp
	symbol.cpp
	type.cpp
	py_helpers.cpp
	py_backend.cpp
	pygtk_backend.cpp
	${BISON_Parser_OUTPUTS}
	${FLEX_Scanner_OUTPUTS})

add_executable(tegel ${SRC_FILES})

install(TARGETS tegel DESTINATION bin)
