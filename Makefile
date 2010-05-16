GECKO_SDK_PATH = /src/firefox/xulrunner-sdk

CXX   =  c++
CPPFLAGS += -fPIC -shared -Wl,-h,libsip.so 
 
GECKO_CONFIG_INCLUDE = -include mozilla-config.h 
 
GECKO_DEFINES  = -DXPCOM_GLUE
 
GECKO_INCLUDES = -I $(GECKO_SDK_PATH)/include 
GECKO_LDFLAGS = -L$(GECKO_SDK_PATH)/lib -L$(GECKO_SDK_PATH)/sdk/lib  $(GECKO_SDK_PATH)/sdk/lib/libxpcomglue_s.a -lxpcomglue -lxpcom -lnspr4  
PJSIP_FLAGS = `pkg-config --cflags --libs libpjproject`


FILES = nsSIP.o nsSIPModule.o

TARGET = libsip.so



all: clean prepare build

prepare: header xpt

build:  
	$(CXX) $(CPPFLAGS) -c nsSIP.cpp $(GECKO_INCLUDES) $(GECKO_CONFIG_INCLUDE) $(PJSIP_FLAGS)
	$(CXX) $(CPPFLAGS) -c nsSIPModule.cpp  $(GECKO_INCLUDES) $(GECKO_CONFIG_INCLUDE)

	$(CXX) $(CPPFLAGS) -o $(TARGET) $(FILES) $(GECKO_LDFLAGS) $(GECKO_DEFINES) $(PJSIP_FLAGS)
	chmod +x $(TARGET)
	strip $(TARGET)
 
header:
	$(GECKO_SDK_PATH)/bin/xpidl -m header -w -v -I $(GECKO_SDK_PATH)/idl/ -o nsISIP nsISIP.idl

xpt:
	$(GECKO_SDK_PATH)/bin/xpidl -m typelib -w -v -I $(GECKO_SDK_PATH)/idl/ -o nsISIP nsISIP.idl

clean: 
	rm -rf $(TARGET)
	rm -rf *.xpt
	rm -rf *.o 
