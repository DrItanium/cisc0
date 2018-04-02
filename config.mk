LIBS = -lc -lm 

CC := gcc-7.3.0
CXX := g++-7.3.0
GENFLAGS = -Wall -g3 
CXXFLAGS = -std=c++17 ${GENFLAGS}
LDFLAGS = ${LIBS}
