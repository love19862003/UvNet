ADD_EXECUTABLE(demo1 demo1.cpp demo.h main.cpp)
SOURCE_GROUP("Include" FILES demo.h) 
ADD_DEPENDENCIES(demo1 ${LINK_LIBS})
TARGET_LINK_LIBRARIES(demo1 ${LINK_LIBS})

ADD_EXECUTABLE(demo2  demo2.cpp demo.h  main.cpp)
ADD_DEPENDENCIES(demo2 ${LINK_LIBS})
TARGET_LINK_LIBRARIES(demo2 ${LINK_LIBS})


ADD_EXECUTABLE(demo-redis demo-redis.cpp demo.h main.cpp)
ADD_DEPENDENCIES(demo-redis  ${LINK_LIBS})
TARGET_LINK_LIBRARIES(demo-redis  ${LINK_LIBS})

ADD_EXECUTABLE(demo-http demo-http.cpp demo.h main.cpp)
ADD_DEPENDENCIES(demo-http  ${LINK_LIBS})
TARGET_LINK_LIBRARIES(demo-http  ${LINK_LIBS})

