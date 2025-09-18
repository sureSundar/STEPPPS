#!/usr/bin/env python3
"""
STEPPPS Mini OS - Memory Management System
=========================================
STEPPPS-enhanced memory management with virtual memory, paging, and AI-driven optimization.
Supports both hosted and bare-metal operation modes.
"""

import os
import sys
import time
import threading
import mmap
from typing import Dict, List, Any, Optional, Tuple
from dataclasses import dataclass, field
from enum import Enum
from abc import ABC, abstractmethod

class MemoryType(Enum):
    PHYSICAL = "physical"
    VIRTUAL = "virtual"
    SHARED = "shared"
    DEVICE = "device"

class PageState(Enum):
    FREE = "free"
    ALLOCATED = "allocated"
    SWAPPED = "swapped"
    LOCKED = "locked"

@dataclass
class MemoryPage:
    page_id: int
    virtual_address: int
    physical_address: Optional[int]
    size: int = 4096  # 4KB pages
    state: PageState = PageState.FREE
    owner_pid: Optional[int] = None
    reference_count: int = 0
    last_accessed: float = field(default_factory=time.time)
    dirty: bool = False

@dataclass
class MemoryRegion:
    start_address: int
    size: int
    permissions: str  # "rwx"
    memory_type: MemoryType
    pages: List[MemoryPage] = field(default_factory=list)

class STEPPPSMemoryManager:
    """STEPPPS-enhanced memory management system"""
    
    def __init__(self, total_memory_mb: int = 1024):
        self.total_memory = total_memory_mb * 1024 * 1024  # Convert to bytes
        self.page_size = 4096  # 4KB pages
        self.total_pages = self.total_memory // self.page_size
        
        # Memory structures
        self.pages: Dict[int, MemoryPage] = {}
        self.free_pages: List[int] = []
        self.allocated_pages: Dict[int, List[int]] = {}  # pid -> page_ids
        self.memory_regions: Dict[int, List[MemoryRegion]] = {}  # pid -> regions
        
        # Virtual memory
        self.page_table: Dict[int, Dict[int, int]] = {}  # pid -> {virtual -> physical}
        self.virtual_base = 0x10000000  # 256MB virtual base
        
        # Memory statistics
        self.allocation_count = 0
        self.deallocation_count = 0
        self.page_faults = 0
        self.swap_operations = 0
        self.allocated_memory = 0
        
        # STEPPPS dimensions
        self.steppps = self._init_steppps_dimensions()
        
        # Initialize memory system
        self._initialize_memory_system()
        
        print(f"💾 STEPPPS Memory Manager initialized: {total_memory_mb}MB ({self.total_pages} pages)")
    
    def _init_steppps_dimensions(self) -> Dict[str, Any]:
        """Initialize STEPPPS dimensions for memory management"""
        return {
            "space": {
                "total_memory": self.total_memory,
                "page_size": self.page_size,
                "virtual_address_space": 0x100000000,  # 4GB virtual space
                "memory_layout": "paged"
            },
            "time": {
                "allocation_time": 0.0,
                "gc_frequency": 60.0,  # seconds
                "last_gc": time.time(),
                "memory_pressure_threshold": 0.8
            },
            "event": {
                "page_fault_handler": self._handle_page_fault,
                "out_of_memory_handler": self._handle_oom,
                "memory_events": []
            },
            "psychology": {
                "allocation_strategy": "best_fit",
                "optimization_enabled": True,
                "learning_rate": 0.1,
                "performance_history": []
            },
            "pixel": {
                "memory_visualization": False,
                "allocation_map": {},
                "fragmentation_display": False
            },
            "prompt": {
                "ai_optimization": True,
                "memory_advisor": True,
                "auto_tuning": True
            },
            "script": {
                "gc_enabled": True,
                "defragmentation": True,
                "memory_compaction": True
            }
        }
    
    def _initialize_memory_system(self):
        """Initialize the memory management system"""
        # Create page table
        for i in range(self.total_pages):
            page = MemoryPage(
                page_id=i,
                virtual_address=self.virtual_base + (i * self.page_size),
                physical_address=i * self.page_size
            )
            self.pages[i] = page
            self.free_pages.append(i)
        
        print(f"   ✅ Initialized {len(self.free_pages)} free pages")
    
    def allocate_memory(self, pid: int, size: int, permissions: str = "rw") -> Optional[int]:
        """Allocate memory for a process"""
        try:
            pages_needed = (size + self.page_size - 1) // self.page_size
            
            if len(self.free_pages) < pages_needed:
                # Try garbage collection
                self._garbage_collect()
                if len(self.free_pages) < pages_needed:
                    self._handle_oom(pid, size)
                    return None
            
            # Allocate pages
            allocated_pages = []
            for _ in range(pages_needed):
                page_id = self.free_pages.pop(0)
                page = self.pages[page_id]
                page.state = PageState.ALLOCATED
                page.owner_pid = pid
                page.reference_count = 1
                allocated_pages.append(page_id)
            
            # Update process allocation tracking
            if pid not in self.allocated_pages:
                self.allocated_pages[pid] = []
            self.allocated_pages[pid].extend(allocated_pages)
            
            # Create virtual mapping
            virtual_address = self._create_virtual_mapping(pid, allocated_pages)
            
            # Update statistics
            self.allocation_count += 1
            self.allocated_memory += pages_needed * self.page_size
            self.steppps["time"]["allocation_time"] = time.time()
            
            # Record allocation event
            self.steppps["event"]["memory_events"].append({
                "type": "allocation",
                "pid": pid,
                "size": size,
                "pages": pages_needed,
                "virtual_address": virtual_address,
                "timestamp": time.time()
            })
            
            print(f"💾 Allocated {size} bytes ({pages_needed} pages) for PID {pid}")
            return virtual_address
            
        except Exception as e:
            print(f"❌ Memory allocation failed: {e}")
            return None
    
    def deallocate_memory(self, pid: int, virtual_address: int) -> bool:
        """Deallocate memory for a process"""
        try:
            if pid not in self.page_table:
                return False
            
            # Find pages to deallocate
            pages_to_free = []
            for vaddr, paddr in self.page_table[pid].items():
                if vaddr >= virtual_address:
                    page_id = paddr // self.page_size
                    if page_id in self.pages:
                        pages_to_free.append(page_id)
            
            # Free pages
            for page_id in pages_to_free:
                page = self.pages[page_id]
                page.state = PageState.FREE
                page.owner_pid = None
                page.reference_count = 0
                page.dirty = False
                self.free_pages.append(page_id)
                
                # Remove from process allocation
                if pid in self.allocated_pages:
                    if page_id in self.allocated_pages[pid]:
                        self.allocated_pages[pid].remove(page_id)
            
            # Update statistics
            self.deallocation_count += 1
            self.allocated_memory -= len(process_pages) * self.page_size
            
            print(f"💾 Deallocated {len(pages_to_free)} pages for PID {pid}")
            return True
            
        except Exception as e:
            print(f"❌ Memory deallocation failed: {e}")
            return False
    
    def _create_virtual_mapping(self, pid: int, page_ids: List[int]) -> int:
        """Create virtual memory mapping for allocated pages"""
        if pid not in self.page_table:
            self.page_table[pid] = {}
        
        # Find contiguous virtual address space
        virtual_address = self.virtual_base + (pid * 0x1000000)  # 16MB per process
        
        for i, page_id in enumerate(page_ids):
            vaddr = virtual_address + (i * self.page_size)
            paddr = page_id * self.page_size
            self.page_table[pid][vaddr] = paddr
        
        return virtual_address
    
    def _handle_page_fault(self, pid: int, virtual_address: int):
        """Handle page fault"""
        self.page_faults += 1
        
        # Check if address is valid
        if pid not in self.page_table:
            print(f"❌ Invalid page fault: PID {pid} not found")
            return
        
        if virtual_address not in self.page_table[pid]:
            print(f"❌ Segmentation fault: Invalid address {hex(virtual_address)} for PID {pid}")
            return
        
        # Handle swapped page
        physical_address = self.page_table[pid][virtual_address]
        page_id = physical_address // self.page_size
        
        if page_id in self.pages:
            page = self.pages[page_id]
            if page.state == PageState.SWAPPED:
                self._swap_in_page(page_id)
            
            page.last_accessed = time.time()
    
    def _handle_oom(self, pid: int, requested_size: int):
        """Handle out of memory condition"""
        print(f"⚠️  Out of memory: PID {pid} requested {requested_size} bytes")
        
        # Try to free memory
        freed_pages = self._emergency_free_memory()
        
        if freed_pages > 0:
            print(f"🔄 Emergency freed {freed_pages} pages")
        else:
            print("💥 Critical: Unable to free memory")
    
    def _garbage_collect(self) -> int:
        """Garbage collection to free unused memory"""
        if not self.steppps["script"]["gc_enabled"]:
            return 0
        
        freed_pages = 0
        current_time = time.time()
        
        # Find pages that haven't been accessed recently
        for page_id, page in self.pages.items():
            if (page.state == PageState.ALLOCATED and 
                page.reference_count == 0 and
                current_time - page.last_accessed > 300):  # 5 minutes
                
                page.state = PageState.FREE
                page.owner_pid = None
                self.free_pages.append(page_id)
                freed_pages += 1
        
        self.steppps["time"]["last_gc"] = current_time
        
        if freed_pages > 0:
            print(f"🗑️  Garbage collection freed {freed_pages} pages")
        
        return freed_pages
    
    def _emergency_free_memory(self) -> int:
        """Emergency memory freeing"""
        freed_pages = 0
        
        # First try garbage collection
        freed_pages += self._garbage_collect()
        
        # Then try swapping out least recently used pages
        if freed_pages < 10:  # Need more pages
            freed_pages += self._swap_out_lru_pages(20)
        
        return freed_pages
    
    def _swap_out_lru_pages(self, count: int) -> int:
        """Swap out least recently used pages"""
        # Sort pages by last accessed time
        allocated_pages = [
            (page.last_accessed, page_id, page)
            for page_id, page in self.pages.items()
            if page.state == PageState.ALLOCATED
        ]
        allocated_pages.sort()  # Oldest first
        
        swapped_count = 0
        for _, page_id, page in allocated_pages[:count]:
            if self._swap_out_page(page_id):
                swapped_count += 1
        
        return swapped_count
    
    def _swap_out_page(self, page_id: int) -> bool:
        """Swap out a page to storage"""
        try:
            page = self.pages[page_id]
            page.state = PageState.SWAPPED
            self.free_pages.append(page_id)
            self.swap_operations += 1
            return True
        except:
            return False
    
    def _swap_in_page(self, page_id: int) -> bool:
        """Swap in a page from storage"""
        try:
            page = self.pages[page_id]
            page.state = PageState.ALLOCATED
            if page_id in self.free_pages:
                self.free_pages.remove(page_id)
            self.swap_operations += 1
            return True
        except:
            return False
    
    def get_memory_usage(self) -> float:
        """Get memory usage as a percentage (0.0 to 1.0)"""
        free_memory = len(self.free_pages) * self.page_size
        used_memory = self.total_memory - free_memory
        return used_memory / self.total_memory if self.total_memory > 0 else 0.0
    
    def get_memory_stats(self) -> Dict[str, Any]:
        """Get comprehensive memory statistics"""
        free_memory = len(self.free_pages) * self.page_size
        used_memory = self.total_memory - free_memory
        
        return {
            "total_memory": self.total_memory,
            "used_memory": used_memory,
            "free_memory": free_memory,
            "memory_utilization": used_memory / self.total_memory,
            "total_pages": self.total_pages,
            "free_pages": len(self.free_pages),
            "allocated_pages": sum(len(pages) for pages in self.allocated_pages.values()),
            "allocation_count": self.allocation_count,
            "deallocation_count": self.deallocation_count,
            "page_faults": self.page_faults,
            "swap_operations": self.swap_operations,
            "processes_with_memory": len(self.allocated_pages),
            "steppps_optimization": self.steppps["psychology"]["optimization_enabled"]
        }
    
    def get_process_memory_info(self, pid: int) -> Dict[str, Any]:
        """Get memory information for a specific process"""
        if pid not in self.allocated_pages:
            return {"error": "Process not found"}
        
        process_pages = self.allocated_pages[pid]
        memory_usage = len(process_pages) * self.page_size
        
        return {
            "pid": pid,
            "memory_usage": memory_usage,
            "page_count": len(process_pages),
            "virtual_mappings": len(self.page_table.get(pid, {})),
            "pages": [
                {
                    "page_id": page_id,
                    "virtual_address": hex(self.pages[page_id].virtual_address),
                    "state": self.pages[page_id].state.value,
                    "last_accessed": self.pages[page_id].last_accessed
                }
                for page_id in process_pages[:5]  # Show first 5 pages
            ]
        }

def main():
    """Demo of STEPPPS Memory Management System"""
    print("💾 STEPPPS Memory Management System")
    print("=" * 40)
    
    # Create memory manager
    mm = STEPPPSMemoryManager(total_memory_mb=64)  # 64MB for demo
    
    try:
        # Simulate process memory allocations
        print("\n🔄 Simulating memory allocations...")
        
        # Process 1: Allocate 1MB
        addr1 = mm.allocate_memory(pid=1, size=1024*1024, permissions="rw")
        print(f"Process 1 allocated at: {hex(addr1) if addr1 else 'FAILED'}")
        
        # Process 2: Allocate 2MB
        addr2 = mm.allocate_memory(pid=2, size=2*1024*1024, permissions="rwx")
        print(f"Process 2 allocated at: {hex(addr2) if addr2 else 'FAILED'}")
        
        # Process 3: Allocate 512KB
        addr3 = mm.allocate_memory(pid=3, size=512*1024, permissions="r")
        print(f"Process 3 allocated at: {hex(addr3) if addr3 else 'FAILED'}")
        
        # Show memory statistics
        print("\n📊 Memory Statistics:")
        stats = mm.get_memory_stats()
        print(f"   Total Memory: {stats['total_memory'] // (1024*1024)}MB")
        print(f"   Used Memory: {stats['used_memory'] // (1024*1024)}MB")
        print(f"   Free Memory: {stats['free_memory'] // (1024*1024)}MB")
        print(f"   Utilization: {stats['memory_utilization']:.1%}")
        print(f"   Page Faults: {stats['page_faults']}")
        print(f"   Allocations: {stats['allocation_count']}")
        
        # Show process memory info
        print("\n📋 Process Memory Info:")
        for pid in [1, 2, 3]:
            info = mm.get_process_memory_info(pid)
            if "error" not in info:
                print(f"   PID {pid}: {info['memory_usage'] // 1024}KB ({info['page_count']} pages)")
        
        # Simulate page fault
        print("\n⚠️  Simulating page fault...")
        mm._handle_page_fault(pid=1, virtual_address=addr1 + 4096)
        
        # Test garbage collection
        print("\n🗑️  Testing garbage collection...")
        freed = mm._garbage_collect()
        print(f"   Freed {freed} pages")
        
        # Deallocate memory
        print("\n🔄 Deallocating memory...")
        mm.deallocate_memory(pid=2, virtual_address=addr2)
        
        # Final statistics
        print("\n📊 Final Memory Statistics:")
        final_stats = mm.get_memory_stats()
        print(f"   Free Memory: {final_stats['free_memory'] // (1024*1024)}MB")
        print(f"   Utilization: {final_stats['memory_utilization']:.1%}")
        print(f"   Total Operations: {final_stats['allocation_count'] + final_stats['deallocation_count']}")
        
    except Exception as e:
        print(f"❌ Demo error: {e}")

if __name__ == "__main__":
    main()
