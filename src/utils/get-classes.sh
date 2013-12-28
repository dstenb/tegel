grep "class *[A-Z][a-zA-Z_]*;" ../ast.hpp | cut -d " " -f 6 | cut -d ";" -f 1
