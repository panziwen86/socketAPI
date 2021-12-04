SRCDIR := .
INCLUDEDIR := . ./include
INCLUDELIST := $(addprefix -I,$(INCLUDEDIR))
#LIBLIST := -L../installed/lib -lboost_system -lboost_filesystem -lboost_date_time -lboost_thread -pthread -lrt

$(warning  $(INCLUDELIST)) 
CXXFLAGS := -g -O2 $(INCLUDELIST)

C_SRC := $(shell echo $(SRCDIR)/*.c)
C_FILTER_SRC := $(SRCDIR)/*.c
C_SRC_USE := $(filter-out $(C_FILTER_SRC), $(C_SRC))
C_OBJ := $(patsubst %c, %o, $(C_SRC_USE))

CPP_SRC := $(shell echo $(SRCDIR)/*.cpp)  
CPP_FILTER_SRC := $(SRCDIR)/*.cpp
CPP_SRC_USE := $(filter-out $(CPP_FILTER_SRC), $(CPP_SRC))
CPP_OBJ := $(patsubst %cpp, %o, $(CPP_SRC_USE)) 

CC := gcc
CXX := g++
AR := ar

TARGET := $(patsubst %.o, %, ${C_OBJ} ${CPP_OBJ})

.PHONY:all clean

all:$(TARGET)

%:%.cpp  
	$(CXX) -o $@ $^ ${CXXFLAGS} ${LIBLIST}

%:%.c  
	$(CC) -o $@ $^ ${CXXFLAGS} ${LIBLIST}

clean:
	rm -f $(TARGET)
