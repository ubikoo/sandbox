SOURCES  += $(filter-out $(wildcard $(ROOTDIR)/_*.cpp), \
                         $(wildcard $(ROOTDIR)/*.cpp)) \
			$(filter-out $(wildcard $(ROOTDIR)/_*.c), \
                         $(wildcard $(ROOTDIR)/*.c))
INCLUDES += $(wildcard $(ROOTDIR)/*.hpp) \
			$(wildcard $(ROOTDIR)/*.h)
CFLAGS   += -I$(ROOTDIR)
