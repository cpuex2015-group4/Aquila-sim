#!/usr/bin/env python
# coding: utf-8

import struct

#if minus, dissolve it
def bin2int(binary):
	def turn_binary(binary):
		if len(binary) == 0:
			return ""
		elif binary[0] == "0":
			return "1" + turn_binary(binary[1:])
		elif binary[0] == "1":
			return "0" + turn_binary(binary[1:])
	if binary[0] == "1":
		return -(int(turn_binary(binary[1:]), 2) + 1)
	else:
		return int(binary, 2)

def byte2int(bytecode):
	return sum([ord(c) * 256**(3-i) for i, c in enumerate(bytecode)])

def reg2float(v):
	if v[0] != "-":  # positive
		v = struct.pack('>I', int(v, 2))
	else:  # negative
		v = struct.pack('>I', int(v[1:], 2))
	return struct.unpack('>f', v)[0]

def float2reg(f):
	v = struct.pack('>f', f)
	v = struct.unpack('>I', v)[0]
	return format(v, '032b')

def word2int(bytecodes):
	"""
	Convert 32bit bytecodes to integer value.
	
	Parameters
	----------
	bytecodes: str
		the original bytecodes
	
	Returns
	----------
	int_val: int
		converted integer value
	"""
	return sum([ord(bytecodes[i])*(0x100**(3-i)) for i in range(4)])

def word2hex(bytecodes):
	"""
	Convert 32bit bytecodes to hex encoded integer value.
	
	Parameters
	----------
	bytecodes: str
		the original bytecodes
	
	Returns
	----------
	hex: str
		hex encoded integer value
	"""
	return format(word2int(bytecodes), "08x")

def bin2bytes(binary):
	"""
	Convert (binary encoded) integer value to bytecode (big-endian). 

	Parameters
	----------
	binary: str
		binary encoded integer value

	Returns
	----------
	bytecode: bytes
		ascii encoded bytecode
	"""
	assert len(binary) == 32
	byte1 = chr(int(binary[0:8], 2))
	byte2 = chr(int(binary[8:16], 2))
	byte3 = chr(int(binary[16:24], 2))
	byte4 = chr(int(binary[24:32], 2))
	return (byte1 + byte2 + byte3 + byte4)

def and_bin(bin1, bin2):
	ans_bin = ""
	for i, bit1 in enumerate(bin1):
		bit2 = bin2[i]
		and_bin += str(int(bit1) * int(bit2))
	return ans_bin

def nor_bin(bin1, bin2):
	ans_bin = ""
	for i, bit1 in enumerate(bin1):
		bit2 = bin2[i]
		and_bin += str((int(bit1) - 1) * (int(bit2) - 1))
	return ans_bin
