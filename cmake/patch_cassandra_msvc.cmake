if(WIN32)
    # remove GCC-specific __attribute__((unused)) from sparsehash
    file(READ "${SOURCE_DIR}/src/third_party/sparsehash/src/sparsehash/internal/hashtable-common.h" CONTENT)
    string(REPLACE "__attribute__((unused))" "" CONTENT "${CONTENT}")
    file(WRITE "${SOURCE_DIR}/src/third_party/sparsehash/src/sparsehash/internal/hashtable-common.h" "${CONTENT}")
    message(STATUS "Patched sparsehash: removed __attribute__((unused))")

    # replace unistd.h with io.h on Windows
    file(GLOB_RECURSE ALL_CPP_FILES "${SOURCE_DIR}/src/*.cpp" "${SOURCE_DIR}/src/*.hpp" "${SOURCE_DIR}/src/*.h")
    foreach(FILE_PATH ${ALL_CPP_FILES})
        file(READ "${FILE_PATH}" CONTENT)
        if (CONTENT MATCHES "#include <unistd.h>")
            string(REPLACE "#include <unistd.h>" "#ifdef _WIN32\n#include <io.h>\n#else\n#include <unistd.h>\n#endif" CONTENT "${CONTENT}")
            file(WRITE "${FILE_PATH}" "${CONTENT}")
            message(STATUS "Patched: ${FILE_PATH}")
        endif()
    endforeach()
endif()