
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(EXTRA_COMPONENT_DIRS components)

idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ".")