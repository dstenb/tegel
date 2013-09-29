BISON_TARGET(Parser parser.y parser.cpp)
FLEX_TARGET(Scanner lexical.l lexical.cpp)

set (SRC_FILES ast.cpp
	bash_backend.cpp
	common.cpp
	constant.cpp
	symbol.cpp
	type.cpp
	py_backend.cpp
	pygtk_backend.cpp
	${BISON_Parser_OUTPUTS}
	${FLEX_Scanner_OUTPUTS})

set (SRC_FILES_TGL main.cpp ${SRC_FILES})
set (SRC_FILES_TGP main-tgp.cpp ${SRC_FILES})

add_executable(tegel ${SRC_FILES_TGL})
add_executable(tegel-tgp ${SRC_FILES_TGP})

install(TARGETS tegel tegel-tgp DESTINATION bin)
