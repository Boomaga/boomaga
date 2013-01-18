cmake_minimum_required( VERSION 2.6 )

project(bokletCups)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/booklet
               ${CMAKE_CURRENT_BINARY_DIR}/booklet
               @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/booklet  
  DESTINATION /usr/lib/cups/backend 
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)

install(FILES booklet.ppd
  DESTINATION /usr/share/ppd/booklet
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
