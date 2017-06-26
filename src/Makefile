OBJS = Public.o ChainWalkContext.o ChainWalkSet.o CrackEngine.o HashAlgorithm.o HashSet.o HashRoutine.o MemoryPool.o RainbowCrack.o RainbowTableDump.o RainbowTableGenerate.o RainbowTableSort.o

all: rtgen rtdump rtsort rcrack

rtgen: $(OBJS)
	$(CXX) -O3 $(LDFLAGS) Public.o ChainWalkContext.o HashAlgorithm.o HashRoutine.o RainbowTableGenerate.o -lssl -lcrypto -o rtgen

rtdump: $(OBJS)
	$(CXX) $(LDFLAGS) Public.o ChainWalkContext.o HashAlgorithm.o HashRoutine.o RainbowTableDump.o -lssl -lcrypto -o rtdump

rtsort: $(OBJS)
	$(CXX) $(LDFLAGS) Public.o RainbowTableSort.o -o rtsort

rcrack: $(OBJS)
	$(CXX) $(LDFLAGS) Public.o ChainWalkContext.o HashAlgorithm.o HashRoutine.o HashSet.o MemoryPool.o ChainWalkSet.o CrackEngine.o RainbowCrack.o -lssl -lcrypto -o rcrack

clean:
	rm -f $(OBJS) rtgen rtdump rtsort rcrack
