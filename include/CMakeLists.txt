# cmake_minimum_required(VERSION 2.8.0)

file( GLOB HDRS "*.h" )

install( FILES ${HDRS} DESTINATION include/$ENV{BRANCH}/SENG )

