ADD_CUSTOM_COMMAND(
    OUTPUT ${PROJECT_SOURCE_DIR}/shaders/frag.spv
    COMMAND touch ${PROJECT_SOURCE_DIR}/shaders/frag.spv && glslc ${PROJECT_SOURCE_DIR}/shaders/shader.frag -o ${PROJECT_SOURCE_DIR}/shaders/frag.spv
    DEPENDS shader.frag
)

ADD_CUSTOM_TARGET(GenerateShaderFrag
    DEPENDS ${PROJECT_SOURCE_DIR}/shaders/frag.spv
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${PROJECT_SOURCE_DIR}/shaders/vert.spv
    COMMAND touch ${PROJECT_SOURCE_DIR}/shaders/vert.spv && glslc ${PROJECT_SOURCE_DIR}/shaders/shader.vert -o ${PROJECT_SOURCE_DIR}/shaders/vert.spv
    DEPENDS shader.frag
)

ADD_CUSTOM_TARGET(GenerateShaderVert
    DEPENDS ${PROJECT_SOURCE_DIR}/shaders/vert.spv
)

ADD_DEPENDENCIES(vulkanApp GenerateShaderFrag GenerateShaderVert)
