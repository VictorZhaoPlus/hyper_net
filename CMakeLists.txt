#cmake file for project all
#author:ooeyusea
#created:2015-07-08 09:38:00

cmake_minimum_required(VERSION 2.8)
PROJECT(all)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/kernel)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/core)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/framework)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/logic)
