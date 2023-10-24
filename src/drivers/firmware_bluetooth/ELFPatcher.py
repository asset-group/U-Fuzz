import shutil
from os import path
from binascii import hexlify
from ppci.format.elf import ElfFile, read_elf
from ppci.format.elf.headers import get_symbol_table_type_name, get_symbol_table_binding_name, SectionHeaderType

sym_types = [SectionHeaderType.DYNSYM, SectionHeaderType.SYMTAB]


class ELFPatcher:
    elf_file = None
    elf_path = None
    last_sym_address = None

    def __init__(self, elf_path, restoreELF=False, always_backup=False):
        with open(elf_path, 'rb') as f:
            has_beckup = path.exists(elf_path + '.backup')
            if not has_beckup or always_backup:
                shutil.copyfile(elf_path, elf_path + '.backup')
                print('Original ELF file saved on ' + elf_path + '.backup')
            elif has_beckup and restoreELF:
                shutil.copyfile(elf_path + '.backup', elf_path)
                print('ELF file restored from ' + elf_path + '.backup')

            self.elf_file = read_elf(f)
            self.elf_path = elf_path

    def find_addr_offset(self, addr):
        for section in self.elf_file.sections:
            s_address = section.header['sh_addr']
            if addr >= s_address and (addr - s_address) < section.header["sh_size"]:
                s_offset = section.header['sh_offset']
                # print("Section:{} at 0x{:08x}, Offset:0x{:08x}".format(self.elf_file.get_str(section.header["sh_name"]),
                #                                                        s_address, s_offset))
                # File offset of symbol
                return int(s_offset + (addr - s_address))
        raise ValueError('Address not found in any section')

    def find_symbols_offset(self, sym_name):
        sym_types = [SectionHeaderType.DYNSYM, SectionHeaderType.SYMTAB]
        for sym_section in self.elf_file.sections:
            if sym_section.header.sh_type in sym_types:
                name_section = self.elf_file.sections[sym_section.header.sh_link]
                table = self.elf_file.read_symbol_table(sym_section)
                for row in table:
                    sym_str = name_section.get_str(row["st_name"])
                    if sym_str == sym_name:
                        bind = get_symbol_table_binding_name(
                            row["st_info"] >> 4)
                        typ = get_symbol_table_type_name(row["st_info"] & 0xF)
                        sym_address = row["st_value"]
                        # Convert address to offset
                        sym_offset = self.find_addr_offset(sym_address)
                        if self.last_sym_address != sym_address:
                            print("[Found] Symbol {} at 0x{:08x}, Offset:0x{:08x}, Size:{}, Type:{}, Bind:{:7}".format(
                                sym_str,
                                sym_address,
                                sym_offset,
                                row["st_size"],
                                typ,
                                bind,
                            ))
                            self.last_sym_address = sym_address
                        return int(sym_offset), sym_address
        raise ValueError('Symbol ' + sym_name + ' not found')

    def patch_all(self, patches):
        """
        Applies all patches
        :param patches: list (or tuple) of patch info dictionaries
        """
        if patches is None or self.elf_file is None:
            return

        with open(self.elf_path, 'r+b') as f:
            for patch in patches:
                addr = patch['address']
                offset = patch['offset']
                payload = patch['opcode']
                f.seek(patch['offset'])
                # change it if patch data is longer than one byte
                data_overwritten = f.read(len(payload))

                if 'original' in patch.keys():  # Check original byte if possible
                    if data_overwritten == patch['original']:
                        print(
                            '[Patch Done] Before:0x{}, After:0x{}, Addr:0x{:08x}, Offset 0x{:08x} OK'.format(
                                hexlify(data_overwritten).decode(),
                                hexlify(payload).decode(),
                                addr,
                                offset))
                    else:
                        print(
                            '[Patch Done] Before:0x{}, After:0x{}, Addr:0x{:08x}, Offset 0x{:08x} FAILED'.format(
                                hexlify(data_overwritten).decode(),
                                hexlify(payload).decode(),
                                addr,
                                offset))
                        return False
                else:
                    print(
                        '[Patch Done] Before:0x{}, After:0x{}, Addr:0x{:08x}, Offset 0x{:08x} OK'.format(
                            hexlify(data_overwritten).decode(),
                            hexlify(payload).decode(),
                            addr,
                            offset))
                f.seek(offset)
                f.write(payload)
                return True

    def get_target_address(self, target):
        if isinstance(target, str):

            # Accept offset notation
            if '+' in target:
                target_s = target.split('+')
                d = int(target_s[1])
                target_offset, target_address = self.find_symbols_offset(
                    target_s[0])
                target_address += d
                target_offset += d
            elif '-' in target:
                target_s = target.split('-')
                d = target_s[1]
                target_offset, target_address = self.find_symbols_offset(
                    target_s[0])
                target_address -= d
                target_offset -= d
            else:
                target_offset, target_address = self.find_symbols_offset(
                    target)
        else:
            target_offset = self.find_addr_offset(target)
            target_address = target
        return target_offset, target_address

    def wrap_negative(self, value, bits):
        """ Make a bitmask of a value, even if it is a negative value ! """
        upper_limit = (1 << (bits)) - 1
        lower_limit = -(1 << (bits - 1))
        if value not in range(lower_limit, upper_limit + 1):
            raise ValueError(
                "Cannot encode {} in {} bits [{},{}]".format(
                    value, bits, lower_limit, upper_limit
                )
            )
        mask = (1 << bits) - 1
        # Performing bitwise and makes it 2 complement.
        bit_value = value & mask
        assert bit_value >= 0
        return bit_value

    def sign_extend(self, x, b):
        if x & (1 << (b - 1)):  # is the highest bit (sign) set? (x>>(b-1)) would be faster
            return x - (1 << b)  # 2s complement
        return x


class XtensaPatcher(ELFPatcher):
    def __init__(self, elf_path, restoreELF=False, always_backup=False):
        super(XtensaPatcher, self).__init__(
            elf_path, restoreELF, always_backup)

    def get_reg_idx(self, reg_name):
        if isinstance(reg_name, str):
            return int(reg_name.split('a')[1])
        elif isinstance(reg_name, int):
            return reg_name

    def nop(self, *args):
        opcode = bytearray([0xf0, 0x20, 0x00])
        return opcode

    def nop_n(self, *args):
        opcode = bytearray([0x3d, 0xf0])
        return opcode

    def rfi(self, *args):
        level = int(args[1])
        # rfi pg. 488
        opcode = bytearray(
            [0b00010000, (0b0011 << 4) | (level & 0xF), 0b00000000])
        return opcode

    def memw(self, *args):
        # memw pg. 409
        opcode = bytearray(
            [0xc0, 0x20, 0x00])
        return opcode

    def rsync(self, *args):
        # rsync pg. 502
        opcode = bytearray(
            [0x10, 0x20, 0x00])
        return opcode

    def ret(self, *args):
        opcode = bytearray([0x80, 0x00, 0x00])
        return opcode

    def ret_n(self, *args):
        opcode = bytearray([0x0d, 0xf0])
        return opcode

    def retw(self, *args):
        opcode = bytearray([0x90, 0x00, 0x00])
        return opcode

    def retw_n(self, *args):
        opcode = bytearray([0x1d, 0xf0])
        return opcode

    def mov_n(self, *args):
        to_reg = self.get_reg_idx(args[1])
        from_reg = self.get_reg_idx(args[2])
        # Narrow Move pg. 413
        opcode = bytearray(
            [((to_reg & 0xFF) << 4) | 0b1101, (from_reg & 0x0F)])
        return opcode

    def call0(self, *args):
        # Non-windowed call
        patch_pos = self.get_target_address(args[0])
        jump_pos = self.get_target_address(args[1])
        diff = jump_pos[1] - patch_pos[1]
        assert diff in range(-524284, 524288), str(diff)
        # 18-Bit offset is divided by four (32 bit aligned)
        offset = (((diff & 0x0FFFFF)) >> 2)
        offset = offset << 6  # Shift offset to the correct opcode field
        opcode = bytearray(
            [offset & 0xFF | 0b000101, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF])
        return opcode, patch_pos

    def call4(self, *args):
        patch_pos = self.get_target_address(args[0])
        jump_pos = self.get_target_address(args[1])
        diff = jump_pos[1] - patch_pos[1]
        assert diff in range(-524284, 524288), str(diff)
        # 18-Bit offset is divided by four (32 bit aligned)
        offset = (((diff & 0x0FFFFF)) >> 2)
        offset = offset << 6  # Shift offset to the correct opcode field
        opcode = bytearray(
            [offset & 0xFF | 0b010101, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF])
        return opcode, patch_pos

    def call8(self, *args):
        patch_pos = self.get_target_address(args[0])
        jump_pos = self.get_target_address(args[1])
        diff = jump_pos[1] - patch_pos[1]
        assert diff in range(-524284, 524288), str(diff)
        # 18-Bit offset is divided by four (32 bit aligned)
        offset = (((diff & 0x0FFFFF)) >> 2)
        offset = offset << 6  # Shift offset to the correct opcode field
        opcode = bytearray(
            [offset & 0xFF | 0b100101, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF])
        return opcode, patch_pos

    def call12(self, *args):
        patch_pos = self.get_target_address(args[0])
        jump_pos = self.get_target_address(args[1])
        diff = jump_pos[1] - patch_pos[1]
        assert diff in range(-524284, 524288), str(diff)
        # 18-Bit offset is divided by four (32 bit aligned)
        offset = (((diff & 0x0FFFFF)) >> 2)
        offset = offset << 6  # Shift offset to the correct opcode field
        opcode = bytearray(
            [offset & 0xFF | 0b110101, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF])
        return opcode, patch_pos

    def decode_call(self, patch_target, opcode):
        patch_pos = self.get_target_address(patch_target)
        print('decode_call8: ' + str(hex(opcode >> 6)))
        addr = (patch_pos[1] & 0xFFFFFFFC) + \
               (self.sign_extend(opcode >> 6, 18) << 2) + 4
        return addr

    def ApplyHook(self, patch_target, jump_target):
        return self.ApplyInst(patch_target, 'call8', jump_target)

    def ApplyInst(self, addr, inst_name, arg1=None, arg2=None, arg3=None, arg4=None):
        print('[>> Inst] ' + str(addr) + ', ' + inst_name)
        path_pos = self.get_target_address(addr)
        inst_name = inst_name.replace('.', '_')

        try:
            inst_fcn = getattr(self, inst_name)
        except AttributeError:
            raise NameError("Instruction " +
                            inst_name.replace('_', '.') + " not found")

        opcode = inst_fcn(addr, arg1, arg2, arg3, arg4)

        if isinstance(opcode, tuple):
            opcode = opcode[0]

        patch = [{
            "offset": path_pos[0],
            "address": path_pos[1],
            "opcode": opcode
        }]

        self.patch_all(patch)
        return opcode
