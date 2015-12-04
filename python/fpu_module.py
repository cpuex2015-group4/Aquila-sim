#!/usr/bin/env python
# coding: utf-8

import os
from ctypes import *

CHDIR = os.path.dirname(os.path.abspath(__file__))

class FpuModule:
	c_module_path = os.path.join(CHDIR, "..", "fpu", "libfpu.so")
	c_module = CDLL(c_module_path)
	c_fop2_func_type = CFUNCTYPE(c_uint, c_uint, c_uint)
	c_fop1_func_type = CFUNCTYPE(c_uint, c_uint)
	c_fadd_func = cast(c_module.fadd, c_fop2_func_type)
	c_fmul_func = cast(c_module.fmul, c_fop2_func_type)
	c_finv_func = cast(c_module.finv, c_fop1_func_type)

	@classmethod
	def fadd(cls, f1, f2):
		return cls.c_fadd_func(f1, f2)

	@classmethod
	def fmul(cls, f1, f2):
		return cls.c_fmul_func(f1, f2)

	@classmethod
	def finv(cls, f1):
		return cls.c_finv_func(f1)
