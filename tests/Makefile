CXX=g++
CC=gcc
CPPFLAGS= -O3 -std=c++17 -Wall -Wextra -pedantic -D__LINUX__  -pthread -fPIC -I ../src -DARMA_USE_CXX11 -march=native \
          -I/usr/include/qt5 -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtCore \
          -I/usr/include/qt5/QtGui -I/usr/include/qt5/QtPrintSupport -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets \
          -I/usr/include/x86_64-linux-gnu/qt5/QtCore

LDFLAGS=-fPIC -O3 -L.. -march=native -L/usr/lib/x86_64-linux-gnu
CFLAGS= -O3 -s -DNDEBUG -std=c++17 -pedantic -D__LINUX__ -pthread -Wall -Wextra

LDLIBS=-lDarknetProjectManager -lpthread -ldl -lQt5Gui -lQt5Core -lQt5Widgets -lQt5PrintSupport
OBJDIR=obj

SRCS1=main.cpp

OBJS1=$(subst .cpp,.o,$(SRCS1))
OBJS11=$(addprefix $(OBJDIR)/, $(OBJS1))

OBJS=$(OBJS11) $(OBJS12)
TARGET=test

all: DelBin $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)
	strip -s $(TARGET)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
    
clean:
	$(RM) $(OBJS)
	$(RM) $(TARGET)

DelBin:
	$(RM) $(TARGET)