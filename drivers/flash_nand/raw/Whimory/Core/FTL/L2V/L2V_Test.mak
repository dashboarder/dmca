OBJS=L2V_Test.o L2V_Funcs.o L2V_Init.o L2V_Mem.o L2V_Search.o L2V_Update.o L2V_Valid.o L2V_Types.o L2V_Print.o L2V_Repack.o L2V_Forget.o L2V_Free.o WMROAM.o
LINK_TARGET=L2V_Test
REBUILDABLES=$(OBJS) $(LINK_TARGET)

.PHONY: all
all: copythem $(LINK_TARGET) cleanthem
	#

.PHONY: copythem
copythem:
	cp WMROAM-test.h WMROAM.h
	cp WMROAM-test.c WMROAM.c
	cp WMRFeatures-test.h WMRFeatures.h

.PHONY: cleanthem
cleanthem:
	rm WMROAM.c WMROAM.h WMRFeatures.h

$(LINK_TARGET): $(OBJS)
	gcc -g -o $@ $^ -Wall

.PHONY: clean
clean:
	rm -f $(REBUILDABLES)
	rm WMROAM.c WMROAM.h WMRFeatures.h

%.o: %.c
	gcc -g -o $@ -c $< -Wall -I.
