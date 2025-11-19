#!/usr/bin/env python3
"""
TBScript Compiler (tbsc)
Compiles TBScript source code to TBLVM bytecode

Usage: tbsc input.tbs -o output.tbc
"""

import sys
import struct
import re
from enum import IntEnum

# ========================================================================
# OPCODES (must match tblvm_nano.h)
# ========================================================================

class Op(IntEnum):
    # Control Flow
    NOP = 0x00
    HALT = 0x01
    EXIT = 0x02

    # Stack Operations
    PUSH = 0x10
    PUSH_BYTE = 0x11
    POP = 0x12
    DUP = 0x13
    SWAP = 0x14

    # Arithmetic
    ADD = 0x20
    SUB = 0x21
    MUL = 0x22
    DIV = 0x23
    MOD = 0x24
    NEG = 0x25
    INC = 0x26
    DEC = 0x27

    # Bitwise
    AND = 0x30
    OR = 0x31
    XOR = 0x32
    NOT = 0x33
    SHL = 0x34
    SHR = 0x35

    # Comparison
    EQ = 0x40
    NE = 0x41
    LT = 0x42
    LE = 0x43
    GT = 0x44
    GE = 0x45

    # Jumps
    JMP = 0x50
    JZ = 0x51
    JNZ = 0x52

    # Memory
    LOAD = 0x60
    STORE = 0x61
    LOADG = 0x62
    STOREG = 0x63

    # Functions
    CALL = 0x70
    RET = 0x71

    # I/O
    PRINT = 0x80
    PRINT_CHAR = 0x81
    READ = 0x82
    READ_CHAR = 0x83

# ========================================================================
# COMPILER
# ========================================================================

class TBSCompiler:
    def __init__(self):
        self.bytecode = bytearray()
        self.variables = {}  # name -> local index
        self.globals = {}    # name -> global index
        self.labels = {}     # label -> address
        self.fixups = []     # (addr, label) pairs to fix
        self.local_count = 0
        self.global_count = 0

    def emit(self, *bytes_):
        """Emit bytecode"""
        for b in bytes_:
            if isinstance(b, Op):
                self.bytecode.append(int(b))
            else:
                self.bytecode.append(b)

    def emit_uint16(self, value):
        """Emit 16-bit value (big-endian)"""
        self.bytecode.append((value >> 8) & 0xFF)
        self.bytecode.append(value & 0xFF)

    def emit_uint32(self, value):
        """Emit 32-bit value (big-endian)"""
        self.bytecode.append((value >> 24) & 0xFF)
        self.bytecode.append((value >> 16) & 0xFF)
        self.bytecode.append((value >> 8) & 0xFF)
        self.bytecode.append(value & 0xFF)

    def here(self):
        """Get current bytecode address"""
        return len(self.bytecode)

    def compile_expression(self, expr):
        """Compile an expression"""
        expr = expr.strip()

        # Number literal
        if expr.isdigit() or (expr.startswith('-') and expr[1:].isdigit()):
            val = int(expr)
            if -128 <= val <= 127:
                self.emit(Op.PUSH_BYTE, val & 0xFF)
            else:
                self.emit(Op.PUSH)
                self.emit_uint32(val)
            return

        # String literal - should not be used in expressions
        # Strings are only valid in print/println statements
        if expr.startswith('"') and expr.endswith('"'):
            print(f"Error: String literals not supported in expressions: {expr}")
            return

        # Variable
        if expr.isidentifier():
            if expr in self.variables:
                self.emit(Op.LOAD, self.variables[expr])
            elif expr in self.globals:
                self.emit(Op.LOADG, self.globals[expr])
            else:
                print(f"Error: Undefined variable '{expr}'")
            return

        # Binary operations
        for op_str, op_code in [('+', Op.ADD), ('-', Op.SUB), ('*', Op.MUL),
                                 ('/', Op.DIV), ('%', Op.MOD),
                                 ('==', Op.EQ), ('!=', Op.NE),
                                 ('<', Op.LT), ('<=', Op.LE),
                                 ('>', Op.GT), ('>=', Op.GE)]:
            if op_str in expr:
                parts = expr.split(op_str, 1)
                if len(parts) == 2:
                    self.compile_expression(parts[0].strip())
                    self.compile_expression(parts[1].strip())
                    self.emit(op_code)
                    return

        print(f"Error: Cannot compile expression '{expr}'")

    def compile_statement(self, stmt):
        """Compile a statement"""
        stmt = stmt.strip()
        if not stmt or stmt.startswith('//'):
            return

        # print(expr)
        if stmt.startswith('print('):
            expr = stmt[6:-1]  # Remove 'print(' and ')'
            # Check if it's a string literal
            if expr.strip().startswith('"') and expr.strip().endswith('"'):
                # String literal - print character by character
                s = expr.strip()[1:-1]
                for ch in s:
                    self.emit(Op.PUSH_BYTE, ord(ch))
                    self.emit(Op.PRINT_CHAR)
            else:
                # Number or variable - compile and print
                self.compile_expression(expr)
                self.emit(Op.PRINT)
            return

        # println(expr)
        if stmt.startswith('println('):
            expr = stmt[8:-1]
            # Check if it's a string literal
            if expr.strip().startswith('"') and expr.strip().endswith('"'):
                # String literal - print character by character
                s = expr.strip()[1:-1]
                for ch in s:
                    self.emit(Op.PUSH_BYTE, ord(ch))
                    self.emit(Op.PRINT_CHAR)
            else:
                # Number or variable - compile and print
                self.compile_expression(expr)
                self.emit(Op.PRINT)
            # Always add newline
            self.emit(Op.PUSH_BYTE, ord('\n'))
            self.emit(Op.PRINT_CHAR)
            return

        # var = expr
        if '=' in stmt and not any(op in stmt for op in ['==', '!=', '<=', '>=']):
            parts = stmt.split('=', 1)
            var_name = parts[0].strip()
            expr = parts[1].strip()

            # Declare variable if new
            if var_name not in self.variables:
                self.variables[var_name] = self.local_count
                self.local_count += 1

            self.compile_expression(expr)
            self.emit(Op.STORE, self.variables[var_name])
            return

        # if (condition) { ... }
        if stmt.startswith('if'):
            # Simple if implementation
            return

        # while (condition) { ... }
        if stmt.startswith('while'):
            # Simple while implementation
            return

        print(f"Error: Cannot compile statement '{stmt}'")

    def compile_function(self, lines, start_idx):
        """Compile a function"""
        # For now, just compile main() as the entry point
        i = start_idx + 1
        while i < len(lines):
            line = lines[i].strip()
            if line == '}':
                break
            if line:
                self.compile_statement(line.rstrip(';'))
            i += 1
        return i

    def compile(self, source):
        """Compile TBScript source to bytecode"""
        lines = source.split('\n')

        i = 0
        while i < len(lines):
            line = lines[i].strip()

            # Skip empty lines and comments
            if not line or line.startswith('//'):
                i += 1
                continue

            # Function definition
            if 'void main()' in line or 'int main()' in line:
                i = self.compile_function(lines, i)
            else:
                self.compile_statement(line.rstrip(';'))

            i += 1

        # Add HALT at end
        self.emit(Op.HALT)

        return bytes(self.bytecode)

# ========================================================================
# MAIN
# ========================================================================

def main():
    if len(sys.argv) < 2:
        print("Usage: tbsc input.tbs [-o output.tbc]")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[3] if len(sys.argv) >= 4 and sys.argv[2] == '-o' else 'out.tbc'

    # Read source
    try:
        with open(input_file, 'r') as f:
            source = f.read()
    except FileNotFoundError:
        print(f"Error: File '{input_file}' not found")
        sys.exit(1)

    # Compile
    compiler = TBSCompiler()
    bytecode = compiler.compile(source)

    # Write bytecode
    with open(output_file, 'wb') as f:
        f.write(bytecode)

    print(f"Compiled {input_file} -> {output_file}")
    print(f"Bytecode size: {len(bytecode)} bytes")
    print(f"Variables: {len(compiler.variables)} local, {len(compiler.globals)} global")

if __name__ == '__main__':
    main()
