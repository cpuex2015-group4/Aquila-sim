#!/usr/bin/env python
# coding: utf-8

import utils

REG_DICT = {v:k for k,v in {"%zero" : 0, "%at" : 1,\
		"%v0" : 2, "%v1" : 3,\
		"%a0" : 4, "%a1" : 5, "%a2" : 6, "%a3" : 7,\
		"%t0" : 8, "%t1" : 9, "%t2" : 10, "%t3" : 11, "%t4" : 12, "%t5" : 13, "%t6" : 14, "%t7" : 15,\
		"%s0" : 16, "%s1" : 17, "%s2" : 18, "%s3" : 19, "%s4" : 20, "%s5" : 21, "%s6" : 22, "%s7" : 23,\
		"%t8" : 24, "%t9" : 25,\
		"%k0" : 26, "%k1" : 27,\
		"%gp" : 28, "%sp" : 29, "%fp" : 30, "%ra" : 31}.items()}

FREG_DICT = {v:k for k,v in {
		"%f0" : 0, "%f1" : 1, "%f2" : 2, "%f3" : 3, "%f4" : 4, "%f5" : 5, "%f6" : 6, "%f7" : 7, 
		"%f8" : 8, "%f9" : 9, "%f10" : 10, "%f11" : 11, "%f12" : 12, "%f13" : 13, "%f14" : 14, "%f15" : 15, 
		"%f16" : 16, "%f17" : 17, "%f18" : 18, "%f19" : 19, "%f20" : 20, "%f21" : 21, "%f22" : 22, "%f23" : 23, 
		"%f24" : 24, "%f25" : 25, "%f26" : 26, "%f27" : 27, "%f28" : 28, "%f29" : 29, "%f30" : 30, "%f31" : 31 }.items()}

def r(reg):
	return REG_DICT[reg]

def fr(reg):
	return FREG_DICT[reg]

class Disassembler:
	"""
	A class representing for Carina ISA disassembler.
	"""

	def disassember(self, inst):
		return self.fetch_instruction(inst)

	def fetch_instruction(self, inst):
		inst_bin = format(int(inst, 16), '032b')
		operation_bin = inst_bin[0:6]
		funct_bin = inst_bin[26:]
		fpuop_bin = inst_bin[0:11]
		fbranch_bin = inst_bin[0:16]
		if(operation_bin == "000000" and funct_bin == "100000"):
			return Disassembler.add(self, inst_bin)
		elif(operation_bin == "001000"):
			return Disassembler.addi(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "100100"):
			return Disassembler.and_(self, inst_bin)
		elif(operation_bin == "001100"):
			return Disassembler.andi(self, inst_bin)
		elif(operation_bin == "000100"):
			return Disassembler.beq(self, inst_bin)
		elif(operation_bin == "000101"):
			return Disassembler.bne(self, inst_bin)
		elif(operation_bin == "000010"):
			return Disassembler.j(self, inst_bin)
		elif(operation_bin == "000011"):
			return Disassembler.jal(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "001000"):
			return Disassembler.jr(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "001001"):
			return Disassembler.jral(self, inst_bin)
		elif(operation_bin == "100011"):
			return Disassembler.lw(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "100111"):
			return Disassembler.nor(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "100101"):
			return Disassembler.or_(self, inst_bin)
		elif(operation_bin == "001101"):
			return Disassembler.ori(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "101010"):
			return Disassembler.slt(self, inst_bin)
		elif(operation_bin == "001010"):
			return Disassembler.slti(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "000000"):
			return Disassembler.sll(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "000010"):
			return Disassembler.srl(self, inst_bin)
		elif(operation_bin == "101011"):
			return Disassembler.sw(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "100010"):
			return Disassembler.sub(self, inst_bin)
		elif(operation_bin == "111111" and funct_bin == "111111"):
			return Disassembler.hlt(self, inst_bin)
		elif(fbranch_bin == "0100010100000001"):
			return Disassembler.bclt(self, inst_bin)
		elif(fbranch_bin == "0100010100000000"):
			return Disassembler.bclf(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "011000"):
			return Disassembler.mult(self, inst_bin)
		elif(operation_bin == "000000" and funct_bin == "011010"):
			return Disassembler.div(self, inst_bin)
		elif(fpuop_bin == "01000110000" and funct_bin == "000000"):
			return Disassembler.fadd(self, inst_bin)
		elif(fpuop_bin == "01000110000" and funct_bin == "000001"):
			return Disassembler.fsub(self, inst_bin)
		elif(fpuop_bin == "01000110000" and funct_bin == "000010"):
			return Disassembler.fmul(self, inst_bin)
		elif(fpuop_bin == "01000110000" and funct_bin == "000011"):
			return Disassembler.finv(self, inst_bin)
		elif(operation_bin == "110001"):
			return Disassembler.flw(self, inst_bin)
		elif(operation_bin == "111001"):
			return Disassembler.fsw(self, inst_bin)
		elif(fpuop_bin == "01000110000"):
			return Disassembler.fcmp(self, inst_bin)
		elif(operation_bin == "011010"):
			return Disassembler.in_(self, inst_bin)
		elif(operation_bin == "011011"):
			return Disassembler.out(self, inst_bin)
		else:
			raise ValueError("no match with any instruction for bytecode `{}`".format(inst_bin))

	@staticmethod
	def decode_R(inst_bin):
		rs = int(inst_bin[6:11], 2)
		rt = int(inst_bin[11:16], 2)
		rd = int(inst_bin[16:21], 2)
		shamt = utils.bin2int(inst_bin[21:26])
		return rs, rt, rd, shamt

	@staticmethod
	def decode_I(inst_bin):
		rs = int(inst_bin[6:11], 2)
		rt = int(inst_bin[11:16], 2)
		imm = utils.bin2int(inst_bin[16:])
		return rs, rt, imm

	@staticmethod
	def decode_FR(inst_bin):
		ft = int(inst_bin[11:16], 2)
		fs = int(inst_bin[16:21], 2)
		fd = int(inst_bin[21:26], 2)
		return ft, fs, fd

	@staticmethod
	def decode_FI(inst_bin):
		fs = int(inst_bin[11:16], 2)
		imm = utils.bin2int(inst_bin[16:])
		return fs, imm

	@classmethod
	def add(cls, sim, inst_bin):
		rs, rt, rd, _ = cls.decode_R(inst_bin)
		return "add     {}, {}, {}".format(r(rd), r(rs), r(rt))

	@classmethod
	def addi(cls, sim, inst_bin):
		rs, rt, imm = cls.decode_I(inst_bin)
		return "addi    {}, {}, ${}".format(r(rt), r(rs), imm)

	@classmethod
	def and_(cls, sim, inst_bin):
		rs, rt, rd, _ = cls.decode_R(inst_bin)
		return "and     {}, {}, {}".format(r(rd), r(rs), r(rt))

	@classmethod
	def andi(cls, sim, inst_bin):
		rs, rt, imm = cls.decode_I(inst_bin)
		return "andi    {}, {}, ${}".format(r(rt), r(rs), imm)

	@classmethod
	def beq(cls, sim, inst_bin):
		rs, rt, imm = cls.decode_I(inst_bin)
		return "beq     {}, {}, ${}".format(r(rt), r(rs), imm)

	@classmethod
	def bne(cls, sim, inst_bin):
		rs, rt, imm = cls.decode_I(inst_bin)
		return "bne     {}, {}, ${}".format(r(rt), r(rs), imm)

	@staticmethod
	def j(sim, inst_bin):
		imm = inst_bin[6:]
		return "j       {}".format(utils.bin2int(imm))

	@staticmethod
	def jal(sim, inst_bin):
		imm = inst_bin[6:]
		return "jal     {}".format(utils.bin2int(imm))

	@staticmethod
	def jr(sim, inst_bin):
		rs = int(inst_bin[6:11], 2)
		return "jr      {}".format(r(rs))

	@staticmethod
	def jral(sim, inst_bin):
		rs = int(inst_bin[6:11], 2)
		return "jral    {}".format(r(rs))

	@classmethod
	def lw(cls, sim, inst_bin):
		rs, rt, imm = cls.decode_I(inst_bin)
		return "lw      {}, {}({})".format(r(rt), imm, r(rs))

	@classmethod
	def nor(cls, sim, inst_bin):
		rs, rt, rd, _ = cls.decode_R(inst_bin)
		return "nor     {}, {}, {}".format(r(rd), r(rs), r(rt))

	@classmethod
	def or_(cls, sim, inst_bin):
		rs, rt, rd, _ = cls.decode_R(inst_bin)
		return "or      {}, {}, {}".format(r(rd), r(rs), r(rt))

	@classmethod
	def ori(cls, sim, inst_bin):
		rs, rt, imm = cls.decode_I(inst_bin)
		return "ori     {}, {}, ${}".format(r(rt), r(rs), imm)

	@classmethod
	def slt(cls, sim, inst_bin):
		rs, rt, rd, _ = cls.decode_R(inst_bin)
		return "slt     {}, {}, {}".format(r(rd), r(rs), r(rt))

	@classmethod
	def slti(cls, sim, inst_bin):
		rs, rt, imm = cls.decode_I(inst_bin)
		return "slti    {}, {}, ${}".format(r(rt), r(rs), imm)

	@classmethod
	def sll(cls, sim, inst_bin):
		rs, _, rd, shamt_bin = cls.decode_R(inst_bin)
		return "sll     {}, {}, ${}".format(r(rd), r(rs), shamt_bin)

	@classmethod
	def srl(cls, sim, inst_bin):
		rs, _, rd, shamt_bin = cls.decode_R(inst_bin)
		return "srl     {}, {}, ${}".format(r(rd), r(rs), shamt_bin)

	@classmethod
	def sw(cls, sim, inst_bin):
		rs, rt, imm = cls.decode_I(inst_bin)
		return "sw      {}, {}({})".format(r(rt), imm, r(rs))

	@classmethod
	def sub(cls, sim, inst_bin):
		rs, rt, rd, _ = cls.decode_R(inst_bin)
		return "sub     {}, {}, {}".format(r(rd), r(rs), r(rt))

	@staticmethod
	def hlt(sim, inst_bin):
		return "hlt"

	@classmethod
	def bclt(cls, sim, inst_bin):
		_, imm = cls.decode_FI(inst_bin)
		return "bclt    {}".format(imm)

	@classmethod
	def bclf(cls, sim, inst_bin):
		_, imm = cls.decode_FI(inst_bin)
		return "bclt    {}".format(imm)

	@classmethod
	def mult(cls, sim, inst_bin):
		rs, rt, rd, _ = cls.decode_R(inst_bin)
		return "mult    {}, {}, {}".format(r(rd), r(rs), r(rt))

	@classmethod
	def div(cls, sim, inst_bin):
		rs, rt, rd, _ = cls.decode_R(inst_bin)
		return "div     {}, {}, {}".format(r(rd), r(rs), r(rt))

	@classmethod
	def fadd(cls, sim, inst_bin):
		ft, fs, fd = cls.decode_FR(inst_bin)
		return "fadd    {}, {}, {}".format(fr(fd), fr(fs), fr(ft))

	@classmethod
	def fsub(cls, sim, inst_bin):
		ft, fs, fd = cls.decode_FR(inst_bin)
		return "fsub    {}, {}, {}".format(fr(fd), fr(fs), fr(ft))

	@classmethod
	def fmul(cls, sim, inst_bin):
		ft, fs, fd = cls.decode_FR(inst_bin)
		return "fmul    {}, {}, {}".format(fr(fd), fr(fs), fr(ft))

	@classmethod
	def finv(cls, sim, inst_bin):
		ft, _, fd = cls.decode_FR(inst_bin)
		return "finv    {}, {}".format(fr(fd), fr(ft))

	@classmethod
	def flw(cls, sim, inst_bin):
		rs, ft, imm = cls.decode_I(inst_bin)
		return "lw.s    {}, {}({})".format(fr(ft), imm, fr(rs))

	@classmethod
	def fsw(cls, sim, inst_bin):
		rs, ft, imm = cls.decode_I(inst_bin)
		return "sw.s    {}, {}({})".format(fr(ft), imm, r(rs))

	@classmethod
	def fcmp(cls, sim, inst_bin):
		ft, fs, _ = cls.decode_FR(inst_bin)
		op = inst_bin[26:32]
		if op == "110010":    # eq
			return "c.eq.s  {}, {}".format(fr(ft), fr(fs))
		elif op == "111100":  # lt
			return "c.lt.s  {}, {}".format(fr(ft), fr(fs))
		elif op == "111110":  # le
			return "c.le.s  {}, {}".format(fr(ft), fr(fs))

	@classmethod
	def in_(cls, sim, inst_bin):
		_, rs, _, _ = cls.decode_R(inst_bin)
		return "in      {}".format(r(rs))

	@classmethod
	def out(cls, sim, inst_bin):
		_, rs, _, _ = cls.decode_R(inst_bin)
		return "out     {}".format(r(rs))
