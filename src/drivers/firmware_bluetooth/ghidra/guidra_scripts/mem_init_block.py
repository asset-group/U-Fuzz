# Initialise a block of memory by splitting the block of memory of interest, set 
# them to initialised and set bytes to them
#@author Henry Fung
#@category CustomScript

#TODO Add User Code Here
def mySplit(start, end, value=""):
        mem = currentProgram.getMemory()
        aStart = currentAddress.getAddress(start)
        aEnd = currentAddress.getAddress(hex(int(end, base=16) + 1)[2:])
        
        # Split upper bound
        mem.split(mem.getBlock(aStart), aStart)
        # Split lower bound
        mem.split(mem.getBlock(aEnd), aEnd)
        print("Split done (from {} to {})".format(start, end))
        
        newBlock = mem.getBlock(aStart)
        
	# Initialise the block to 0
        mem.convertToInitialized(newBlock, 0)
        
	# Insert bytes
        data = [int(value[i:i+2], base=16) for i in range(0, len(value), 2)]
        addr = start
        i = 0
        for d in data:
                mem.setByte(currentAddress.getAddress(hex(int(addr, base=16) + i)[2:]), d)
                i += 1

startAddr = askString("Start address in hex", "Enter start address (1234abcd)")
endAddr = askString("End address in hex", "Enter end address (12350000)")
data = askString("Bytes to insert", "Enter bytes to insert (deadbeef)")

mySplit(startAddr, endAddr, data) 
