AUX_SOURCE_DIRECTORY(. SOURCE_UTILITY)
FILE(GLOB UTILITY_HEADERS  *.h *.hpp)
SOURCE_GROUP("Include" FILES ${UTILITY_HEADERS} ) 
SOURCE_GROUP("src" FILES ${SOURCE_UTILITY} ) 
ADD_LIBRARY(utility STATIC ${SOURCE_UTILITY} ${UTILITY_HEADERS})