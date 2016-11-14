
AUX_SOURCE_DIRECTORY(. SUB_SOURCES)
SET(SOURCES ${SOURCES} ${SUB_SOURCES})

IF (WIN32)
   ADD_EXECUTABLE(make-uv WIN32 ${SOURCES})
ELSE (WIN32)
   ADD_EXECUTABLE(make-uv ${SOURCES})
ENDIF (WIN32)


ADD_DEPENDENCIES(make-uv basic math image cells renderer gom gom_basic
    scene_graph gui)

IF (WIN32)
ELSE (WIN32)
    # TARGET_LINK_LIBRARIES(make-uv basic math image cells mpeg2 jpeg-6b numeric_stuff zlib png m dl pthread renderer gom gom_basic scene_graph gui)
TARGET_LINK_LIBRARIES(make-uv basic math image cells mpeg2 jpeg
   numeric_stuff z png m dl pthread renderer gom gom_basic scene_graph gui)
ENDIF (WIN32)


