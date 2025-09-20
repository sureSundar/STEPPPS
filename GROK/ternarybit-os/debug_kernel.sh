#!/bin/bash

# Debug script to check kernel loading

# Check if kernel was loaded at 0x1000
echo "Checking kernel at 0x10000 (0x1000:0x0000)..."
hexdump -C -s 0x10000 -n 64 build/tbos.img

# Check if bootloader is present
echo -e "\nChecking bootloader at 0x7C00..."
hexdump -C -s 0x7C00 -n 32 build/tbos.img

# Check GDT setup
echo -e "\nChecking GDT at 0x7E00..."
hexdump -C -s 0x7E00 -n 32 build/tbos.img
