#!/usr/bin/env python
# coding: utf-8

import sys
import pprint
import utils
from disassembler import Disassembler
from fpu_module import FpuModule as fpu
import traceback
import pickle

disas = Disassembler()

class Simulator:
	"""
	A class representing for Carina ISA simulator.
	"""

	def __init__(self, filename, stdin = None):
		"""
		Parameters
		----------
		filename: str
			the name of file which is to be simulated
		"""
		self.data_size = 0
		self.text_size = 0
		self.entry_point = 0
		self.pc = 0
		self.reg = {}
		self.freg = {}
		self.inst_mem = []
		self.mem = {}
		self.fpcond = 0
		self.dic = 0  # dynamic instruction count
		self.stdin = None
		if stdin is not None:
			self.stdin = open(stdin)

		with open(filename, "rb") as file_in:
			self.binary = file_in.read()
			self.load_header()
			self.load_instruction()
			self.load_data()

		for i in range(32):
			# init registers
			self.reg[format(i, "05b")] = "0"*32 
			self.freg[format(i, "05b")] = "0"*32

		# heap pointer
		self.reg["11100"] = format(self.text_size + self.data_size, "032b")
		# stack pointer 
		self.reg["11101"] = format(0xfffff, "032b")
		self.pc = self.entry_point

	def __del__(self):
		if self.stdin is not None:
			self.stdin.close()
	
	def simulate(self, verbose = False):
		"""
		Execute simulation until hlt instruction. The target file is given in __init__.

		Parameters
		----------
		verbose: bool, option
			use verbose output in simulation

		Returns
		----------
		rv: int
			the content of return value register %v0
		"""
		if verbose:
			sys.stderr.write("(dyn_inst_cnt, pc, %gp, %sp, disas)\n")

		try:
			while(True):
				self.dic += 1
				inst = self.inst_mem[self.pc]
				if verbose:
					sys.stderr.write(str(
							(self.dic,
							 self.pc,
							 utils.bin2int(self.reg["11100"]),
							 utils.bin2int(self.reg["11101"]),
							 disas.disassember(inst))) + "\n")
				res = self.fetch_instruction(inst)
				# halting at `hlt` instruction
				if(res == 0): break
		except Exception as e:
			with open(".mem-dump", "w") as dump:
				pickle.dump(self.mem, dump)
			sys.stderr.write("{}: {}".format(type(e), e))
			sys.stderr.write(traceback.format_exc())
			raise e
	
		# return the content of %v0 and %f2
		return (self.reg["00010"], self.freg["00010"])

	def load_header(self):
		"""
		Read the first 16 bytes of the target file as the header.
		The header format is
		[ "CARN" | .data size (4B) | .text size (4B) | entry point (4B) ]
		"""

		assert self.binary[0:4] == "CARN", "the target file must have a valid magic number at the head."

		self.text_size   = utils.word2int(self.binary[4:8])
		self.data_size   = utils.word2int(self.binary[8:12])
		self.entry_point = utils.word2int(self.binary[12:16])

	def load_instruction(self):
		"""
		Read instructions from the target binary and load them to the memory.
		Each single memory cell can be stored 32bit data.
		"""
		for i in range(self.text_size):
			# convert each word to hex encoded value
			offset = i * 4 + 16
			inst = utils.word2hex(self.binary[offset:offset+4])
			self.inst_mem.append(inst)

	def load_data(self):
		"""
		Load data (floating point values table) from the target binary to the simulator memory.
		"""
		for i in range(16 + self.text_size * 4, len(self.binary), 4):
			data = self.binary[i:i+4]
			self.mem.setdefault(format((i - 16) / 4, '032b'), format(utils.byte2int(data), '032b'))

	def fetch_instruction(self, inst):
		inst_bin = format(int(inst, 16), '032b')
		operation_bin = inst_bin[0:6]
		funct_bin = inst_bin[26:]
		fpuop_bin = inst_bin[0:11]
		fbranch_bin = inst_bin[0:16]
		if(operation_bin == "000000" and funct_bin == "100000"):
			return Simulator.add(self, inst_bin)
		elif(operation_bin == "001000"):
			return Simulator.addi(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "100100"):
			return Simulator.and_(self, inst_bin)
		elif(operation_bin == "001100"):
			return Simulator.andi(self, inst_bin)
		elif(operation_bin == "000100"):
			return Simulator.beq(self, inst_bin)
		elif(operation_bin == "000101"):
			return Simulator.bne(self, inst_bin)
		elif(operation_bin == "000010"):
			return Simulator.j(self, inst_bin)
		elif(operation_bin == "000011"):
			return Simulator.jal(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "001000"):
			return Simulator.jr(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "001001"):
			return Simulator.jral(self, inst_bin)
		elif(operation_bin == "100011"):
			return Simulator.lw(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "100111"):
			return Simulator.nor(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "100101"):
			return Simulator.or_(self, inst_bin)
		elif(operation_bin == "001101"):
			return Simulator.ori(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "101010"):
			return Simulator.slt(self, inst_bin)
		elif(operation_bin == "001010"):
			return Simulator.slti(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "000000"):
			return Simulator.sll(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "000010"):
			return Simulator.srl(self, inst_bin)
		elif(operation_bin == "101011"):
			return Simulator.sw(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "100010"):
			return Simulator.sub(self, inst_bin)
		elif(operation_bin == "111111" and funct_bin == "111111"):
			return Simulator.hlt(self, inst_bin)
		elif(fbranch_bin == "0100010100000001"):
			return Simulator.bclt(self, inst_bin)
		elif(fbranch_bin == "0100010100000000"):
			return Simulator.bclf(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "011000"):
			return Simulator.mult(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "011010"):
			return Simulator.div(self, inst_bin)
		elif(fpuop_bin == "01000110000" and funct_bin == "000000"):
			return Simulator.fadd(self, inst_bin)
		elif(fpuop_bin == "01000110000" and funct_bin == "000001"):
			return Simulator.fsub(self, inst_bin)
		elif(fpuop_bin == "01000110000" and funct_bin == "000010"):
			return Simulator.fmul(self, inst_bin)
		elif(fpuop_bin == "01000110000" and funct_bin == "000011"):
			return Simulator.finv(self, inst_bin)
		elif(operation_bin == "110001"):
			return Simulator.flw(self, inst_bin)
		elif(operation_bin == "111001"):
			return Simulator.fsw(self, inst_bin)
		elif(fpuop_bin == "01000110000"):
			return Simulator.fcmp(self, inst_bin)
		elif(operation_bin == "011010"):
			return Simulator.in_(self, inst_bin)
		elif(operation_bin == "011011"):
			return Simulator.out(self, inst_bin)
		else:
			raise ValueError("no match with any instruction for bytecode `{}`".format(inst_bin))

	@staticmethod
	def decode_R(inst_bin):
		return inst_bin[6:11], inst_bin[11:16], inst_bin[16:21], inst_bin[21:26]

	@staticmethod
	def decode_I(inst_bin):
		return inst_bin[6:11], inst_bin[11:16], inst_bin[16:]

	@staticmethod
	def decode_FR(inst_bin):
		return inst_bin[11:16], inst_bin[16:21], inst_bin[21:26]

	@staticmethod
	def decode_FI(inst_bin):
		return inst_bin[11:16], inst_bin[16:]

	@classmethod
	def add(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, reg_d_bin, _ = cls.decode_R(inst_bin)
		sim.reg[reg_d_bin] = format(utils.bin2int(sim.reg[reg_s_bin]) + utils.bin2int(sim.reg[reg_t_bin]), "032b")
		sim.pc += 1
		return 1

	@classmethod
	def addi(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, imm_bin = cls.decode_I(inst_bin)
		sim.reg[reg_t_bin] = format(utils.bin2int(sim.reg[reg_s_bin]) + utils.bin2int(imm_bin), "032b")
		sim.pc += 1
		return 1

	@classmethod
	def and_(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, reg_d_bin, _ = cls.decode_R(inst_bin)
		sim.reg[reg_d_bin] = and_bin(sim.reg[reg_s_bin], sim.reg[reg_t_bin])
		sim.pc += 1
		return 1

	@classmethod
	def andi(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, imm_bin = cls.decode_I(inst_bin)
		sim.reg[reg_t_bin] = format(utils.bin2int(sim.reg[reg_s_bin]) & utils.bin2int(imm_bin), "032b")
		sim.pc += 1
		return 1

	@classmethod
	def beq(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, imm_bin = cls.decode_I(inst_bin)
		if sim.reg[reg_s_bin] == sim.reg[reg_t_bin]:
			sim.pc += utils.bin2int(imm_bin) + 1
		else:
			sim.pc += 1
		return 1

	@classmethod
	def bne(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, imm_bin = cls.decode_I(inst_bin)
		if sim.reg[reg_s_bin] != sim.reg[reg_t_bin]:
			sim.pc += utils.bin2int(imm_bin) + 1
		else:
			sim.pc += 1
		return 1

	@staticmethod
	def j(sim, inst_bin):
		imm_bin = inst_bin[6:]
		sim.pc = utils.bin2int(imm_bin)
		return 1

	@staticmethod
	def jal(sim, inst_bin):
		sim.reg["11111"] = format(sim.pc + 1, "032b")
		imm_bin = inst_bin[6:]
		sim.pc = utils.bin2int(imm_bin)
		return 1

	@staticmethod
	def jr(sim, inst_bin):
		reg_s_bin = inst_bin[6:11]
		address_bin = sim.reg[reg_s_bin]
		sim.pc = utils.bin2int(address_bin)
		return 1

	@staticmethod
	def jral(sim, inst_bin):
		reg_s_bin = inst_bin[6:11]
		address_bin = sim.reg[reg_s_bin]
		sim.reg["11111"] = format(sim.pc + 1, "032b")
		sim.pc = utils.bin2int(address_bin)
		return 1

	@classmethod
	def lw(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, imm_bin = cls.decode_I(inst_bin)
		sim.reg[reg_t_bin] = sim.mem[format(utils.bin2int(sim.reg[reg_s_bin]) + utils.bin2int(imm_bin), "032b")]
		sim.pc += 1
		return 1

	@classmethod
	def nor(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, reg_d_bin, _ = cls.decode_R(inst_bin)
		sim.reg[reg_d_bin] = utils.nor_bin(sim.reg[reg_s_bin], sim.reg[reg_t_bin])
		sim.pc += 1
		return 1

	@classmethod
	def or_(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, reg_d_bin, _ = cls.decode_R(inst_bin)
		sim.reg[reg_d_bin] = utils.or_bin(sim.reg[reg_s_bin], sim.reg[reg_t_bin])
		sim.pc += 1
		return 1

	@classmethod
	def ori(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, imm_bin = cls.decode_I(inst_bin)
		sim.reg[reg_t_bin] = format(int("0000000000000000" + imm_bin, 2) | int(sim.reg[reg_s_bin], 2), "032b")
		sim.pc += 1
		return 1

	@classmethod
	def slt(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, reg_d_bin, _ = cls.decode_R(inst_bin)
		if utils.bin2int(sim.reg[reg_s_bin]) < utils.bin2int(sim.reg[reg_t_bin]):
			sim.reg[reg_d_bin] = format(1, "032b")
		else:
			sim.reg[reg_d_bin] = format(0, "032b")
		sim.pc += 1
		return 1

	@classmethod
	def slti(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, imm_bin = cls.decode_I(inst_bin)
		if utils.bin2int(sim.reg[reg_s_bin]) < utils.bin2int(imm_bin):
			sim.reg[reg_t_bin] = format(1, "032b")
		else:
			sim.reg[reg_t_bin] = format(0, "032b")
		sim.pc += 1
		return 1

	@classmethod
	def sll(cls, sim, inst_bin):
		reg_s_bin, _, reg_d_bin, shamt_bin = cls.decode_R(inst_bin)
		sim.reg[reg_d_bin] = format(utils.bin2int(sim.reg[reg_s_bin]) << int(shamt_bin, 2), "032b")
		sim.pc += 1
		return 1

	@classmethod
	def srl(cls, sim, inst_bin):
		reg_s_bin, _, reg_d_bin, shamt_bin = cls.decode_R(inst_bin)
		sim.reg[reg_d_bin] = format(utils.bin2int(sim.reg[reg_s_bin]) >> int(shamt_bin, 2), "032b")
		sim.pc += 1
		return 1

	@classmethod
	def sw(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, imm_bin = cls.decode_I(inst_bin)
		sim.mem[format(utils.bin2int(sim.reg[reg_s_bin]) + utils.bin2int(imm_bin), "032b")] = sim.reg[reg_t_bin]
		sim.pc += 1
		return 1

	@classmethod
	def sub(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, reg_d_bin, _ = cls.decode_R(inst_bin)
		sim.reg[reg_d_bin] = format(utils.bin2int(sim.reg[reg_s_bin]) - utils.bin2int(sim.reg[reg_t_bin]), "032b")
		sim.pc += 1
		return 1

	@staticmethod
	def hlt(sim, inst_bin):
		#print("HALT\n{}".format("=" * 20))
		#pprint.pprint(sim.freg)
		return 0

	@classmethod
	def bclt(cls, sim, inst_bin):
		_, imm = cls.decode_FI(inst_bin)
		if(sim.fpcond == 1):
			sim.pc += utils.bin2int(imm) + 1
		else:
			sim.pc = sim.pc + 1
		return 1

	@classmethod
	def bclf(cls, sim, inst_bin):
		_, imm = cls.decode_FI(inst_bin)
		if(sim.fpcond == 0):
			sim.pc += utils.bin2int(imm) + 1
		else:
			sim.pc = sim.pc + 1
		return 1

	@classmethod
	def mult(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, reg_d_bin, _ = cls.decode_R(inst_bin)
		sim.reg[reg_d_bin] = format(utils.bin2int(sim.reg[reg_s_bin]) * utils.bin2int(sim.reg[reg_t_bin]), "032b")
		sim.pc += 1
		return 1

	@classmethod
	def div(cls, sim, inst_bin):
		reg_s_bin, reg_t_bin, reg_d_bin, _ = cls.decode_R(inst_bin)
		sim.reg[reg_d_bin] = format(utils.bin2int(sim.reg[reg_s_bin]) / utils.bin2int(sim.reg[reg_t_bin]), "032b")
		sim.pc += 1
		return 1

	@classmethod
	def fadd(cls, sim, inst_bin):
		ft, fs, fd = cls.decode_FR(inst_bin)
		sim.freg[fd] = format(fpu.fadd(int(sim.freg[fs], 2), int(sim.freg[ft], 2)), '032b')
		sim.pc += 1
		return 1

	@classmethod
	def fsub(cls, sim, inst_bin):
		ft, fs, fd = cls.decode_FR(inst_bin)
		sim.freg[fd] = utils.float2reg(utils.reg2float(sim.freg[fs]) - utils.reg2float(sim.freg[ft]))
		sim.pc += 1
		return 1

	@classmethod
	def fmul(cls, sim, inst_bin):
		ft, fs, fd = cls.decode_FR(inst_bin)
		sim.freg[fd] = format(fpu.fmul(int(sim.freg[fs], 2), int(sim.freg[ft], 2)), '032b')
		sim.pc += 1
		return 1

	@classmethod
	def finv(cls, sim, inst_bin):
		ft, _, fd = cls.decode_FR(inst_bin)
		sim.freg[fd] = format(fpu.finv(int(sim.freg[ft], 2)), '032b')
		sim.pc += 1
		return 1

	@classmethod
	def flw(cls, sim, inst_bin):
		rs, ft, imm = cls.decode_I(inst_bin)
		sim.freg[ft] = sim.mem[format(utils.bin2int(sim.reg[rs]) + utils.bin2int(imm), '032b')]
		sim.pc += 1
		return 1

	@classmethod
	def fsw(cls, sim, inst_bin):
		rs, ft, imm = cls.decode_I(inst_bin)
		sim.mem[format(utils.bin2int(sim.reg[rs]) + utils.bin2int(imm), '032b')] = sim.freg[ft]
		sim.pc += 1
		return 1

	@classmethod
	def fcmp(cls, sim, inst_bin):
		ft, fs, _ = cls.decode_FR(inst_bin)
		op = inst_bin[26:32]
		f1 = utils.reg2float(sim.freg[fs])
		f2 = utils.reg2float(sim.freg[ft])
		if op == "110010":    # eq
			sim.fpcond = 1 if f1 == f2 else 0
		elif op == "111100":  # lt
			sim.fpcond = 1 if f1 < f2 else 0
		elif op == "111110":  # le
			sim.fpcond = 1 if f1 <= f2 else 0
		sim.pc += 1
		return 1

	@classmethod
	def in_(cls, sim, inst_bin):
		_, rs, _, _ = cls.decode_R(inst_bin)
		if sim.stdin is not None:
			sim.reg[rs] = "0"*24 + format(ord(sim.stdin.read(1)), "08b")
		else:
			sim.reg[rs] = "0"*24 + format(ord(sys.stdin.read(1)), "08b")
		sim.pc += 1
		return 1

	@classmethod
	def out(cls, sim, inst_bin):
		_, rs, _, _ = cls.decode_R(inst_bin)
		sys.stdout.write(chr(int(sim.reg[rs], 2)))
		sys.stdout.flush()
		sim.pc += 1
		return 1
