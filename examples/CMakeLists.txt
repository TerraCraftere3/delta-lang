# Compiler arguments
set(COMPILER_ARGS --verbose --link)

add_test(NAME OpenGL-Native COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/opengl/example.dltu -i ${CMAKE_CURRENT_SOURCE_DIR}/opengl/fibonacci.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/opengl/example.exe -L ${CMAKE_CURRENT_SOURCE_DIR}/opengl/glew32.lib ${COMPILER_ARGS})
add_test(NAME Web-WASM COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/web/example.dlt -o ${CMAKE_CURRENT_SOURCE_DIR}/web/example.wasm ${COMPILER_ARGS} -T wasm)

# Basic Examples
add_test(NAME Basic-Conditionals COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/basic/conditionals.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/basic/conditionals.exe ${COMPILER_ARGS})
add_test(NAME Basic-HelloWorld COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/basic/hello_world.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/basic/hello_world.exe ${COMPILER_ARGS})
add_test(NAME Basic-Loops COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/basic/loops.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/basic/loops.exe ${COMPILER_ARGS})
add_test(NAME Basic-Variables COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/basic/variables.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/basic/variables.exe ${COMPILER_ARGS})

# Algorithm Examples
add_test(NAME Algorithms-Factorial COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/factorial.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/factorial.exe ${COMPILER_ARGS})
add_test(NAME Algorithms-PrimeNumbers COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/prime_numbers.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/prime_numbers.exe ${COMPILER_ARGS})
add_test(NAME Algorithms-GcdLcm COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/gcd_lcm.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/gcd_lcm.exe ${COMPILER_ARGS})
add_test(NAME Algorithms-Sorting COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/sorting.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/sorting.exe ${COMPILER_ARGS})

# String Examples
add_test(NAME Strings-StringOperations COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/strings/string_operations.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/strings/string_operations.exe ${COMPILER_ARGS})
add_test(NAME Strings-CharacterAnalysis COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/strings/character_analysis.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/strings/character_analysis.exe ${COMPILER_ARGS})

# Pointer Examples
add_test(NAME Pointers-PointerBasics COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/pointers/pointer_basics.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/pointers/pointer_basics.exe ${COMPILER_ARGS})
add_test(NAME Pointers-PointerFunctions COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/pointers/pointer_functions.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/pointers/pointer_functions.exe ${COMPILER_ARGS})

# Struct Examples
add_test(NAME Structs-StructBasics COMMAND $<TARGET_FILE:${PROJECT_NAME}> -i ${CMAKE_CURRENT_SOURCE_DIR}/structs/struct_basics.dltu -o ${CMAKE_CURRENT_SOURCE_DIR}/structs/struct_basics.exe ${COMPILER_ARGS})
