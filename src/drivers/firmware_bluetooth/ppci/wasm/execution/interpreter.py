""" Implement wasm by interpreting bytecodes.
"""
import struct


class Interpreter:

    def load_mem(self):
        """ Initialize memory """
        m.execute(data.offset)
        offset = m.pop()


class Machine:
    """ Wasm stack machine.
    """

    def __init__(self):
        self._stack = []
        self._memory = None

    def load(self, addr):
        return struct.unpack('<d', self._memory[addr:addr+8])[0]

    def store(self, addr, value):
        self._memory[addr:addr+8] = struct.pack('<d', value)

    def pop(self):
        return self._stack.pop()
    
    def push(self, value):
        self._stack.append(value)
    
    def execute(self):
        opcode = 1
        args = 0
        if opcode == 'const':
            self.push(args[0])
        elif opcode == 'add':
            right = self.pop()
            left = self.pop()
            self.push(left + right)
        elif opcode == 'call':
            func = self.functions[args[0]]
            result = self.call(func, *fargs)
            if result:
                self.push(result)
        else:
            raise NotImplementedError(opcode)

