BINARY = star97_server
TARGETDIR =../bin/

XX = g++
TARGET = $(TARGETDIR)$(BINARY)
DEBUG  = -g -D_REENTRANT -DDEBUG -DSQLCA_NONE -Wall -std=c++0x
INCLUDES = -I. -I../../common_lib -I../../aes_lib -I../../lua_lib
LIBS = $(OS_LIB) ../../aes_lib/libaes.a ../../common_lib/libcommon.a ../../lua_lib/liblua.a -lpthread -ldl -lrt -lnsl -lz -lgcc_s
CPPFLAGS = $(DEBUG) $(INCLUDES) 

%.o:%.cpp
	$(XX) $(CPPFLAGS) -c $< -o $@


SOURCE = $(wildcard *.cpp ./Base/*.cpp ./Star97/*.cpp)

OBJS = $(patsubst %.cpp,%.o,$(SOURCE))

$(TARGET):$(OBJS)
	$(XX) $(CPPFLAGS) -o $(TARGET)  $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) $(TARGET) 
