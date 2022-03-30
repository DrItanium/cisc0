LIBS = -lc -lm 

CC := gcc
CXX := g++
GENFLAGS = -Wall -g3 
CXXFLAGS = -std=c++17 ${GENFLAGS}
LDFLAGS = ${LIBS}
