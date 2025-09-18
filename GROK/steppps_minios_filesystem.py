#!/usr/bin/env python3
"""
STEPPPS Mini OS - File System Abstraction
=========================================
Universal file system with STEPPPS integration supporting multiple backends.
Works in both hosted and bare-metal modes with virtual and real storage.
"""

import os
import sys
import time
import json
import threading
import hashlib
from typing import Dict, List, Any, Optional, Union, BinaryIO
from dataclasses import dataclass, field
from enum import Enum
from abc import ABC, abstractmethod

class FileType(Enum):
    REGULAR = "regular"
    DIRECTORY = "directory"
    SYMLINK = "symlink"
    DEVICE = "device"
    PIPE = "pipe"
    SOCKET = "socket"

class FilePermission(Enum):
    READ = "r"
    WRITE = "w"
    EXECUTE = "x"

class FileSystemType(Enum):
    VIRTUAL = "virtual"
    HOST = "host"
    MEMORY = "memory"
    NETWORK = "network"

@dataclass
class FileMetadata:
    """File metadata with STEPPPS integration"""
    name: str
    path: str
    file_type: FileType
    size: int = 0
    permissions: str = "rw-r--r--"
    owner: str = "steppps"
    group: str = "steppps"
    created_time: float = field(default_factory=time.time)
    modified_time: float = field(default_factory=time.time)
    accessed_time: float = field(default_factory=time.time)
    
    # STEPPPS metadata
    steppps_metadata: Dict[str, Any] = field(default_factory=dict)
    
    def __post_init__(self):
        """Initialize STEPPPS metadata"""
        self.steppps_metadata = {
            "space": {
                "storage_location": "virtual",
                "compression": False,
                "encryption": False,
                "checksum": None
            },
            "time": {
                "creation_timestamp": self.created_time,
                "last_modified": self.modified_time,
                "last_accessed": self.accessed_time,
                "retention_policy": None
            },
            "event": {
                "access_count": 0,
                "modification_count": 0,
                "watchers": [],
                "triggers": []
            },
            "psychology": {
                "importance": "normal",
                "access_pattern": "sequential",
                "caching_preference": "auto"
            },
            "pixel": {
                "file_icon": self._get_file_icon(),
                "preview_available": False,
                "thumbnail": None
            },
            "prompt": {
                "ai_indexable": True,
                "content_type": "binary",
                "searchable": True
            },
            "script": {
                "executable": "x" in self.permissions,
                "auto_backup": False,
                "sync_enabled": False
            }
        }
    
    def _get_file_icon(self) -> str:
        """Get file icon based on type and extension"""
        if self.file_type == FileType.DIRECTORY:
            return "📁"
        elif self.file_type == FileType.SYMLINK:
            return "🔗"
        elif self.file_type == FileType.DEVICE:
            return "🔧"
        else:
            # Determine by extension
            ext = os.path.splitext(self.name)[1].lower()
            icons = {
                ".py": "🐍", ".js": "📜", ".html": "🌐", ".css": "🎨",
                ".txt": "📄", ".md": "📝", ".json": "📋", ".xml": "📊",
                ".jpg": "🖼️", ".png": "🖼️", ".gif": "🖼️", ".mp4": "🎬",
                ".mp3": "🎵", ".wav": "🎵", ".zip": "📦", ".tar": "📦"
            }
            return icons.get(ext, "📄")

@dataclass
class FileHandle:
    """File handle for open files"""
    file_id: str
    metadata: FileMetadata
    mode: str
    position: int = 0
    buffer: Optional[bytearray] = None
    locked: bool = False
    
    def __post_init__(self):
        """Initialize file buffer if needed"""
        if self.buffer is None:
            self.buffer = bytearray()

class IFileSystem(ABC):
    """Interface for file system implementations"""
    
    @abstractmethod
    def create_file(self, path: str, content: bytes = b"") -> bool:
        """Create a new file"""
        pass
    
    @abstractmethod
    def create_directory(self, path: str) -> bool:
        """Create a new directory"""
        pass
    
    @abstractmethod
    def open_file(self, path: str, mode: str = "r") -> Optional[str]:
        """Open file and return handle ID"""
        pass
    
    @abstractmethod
    def read_file(self, handle_id: str, size: int = -1) -> Optional[bytes]:
        """Read from file"""
        pass
    
    @abstractmethod
    def write_file(self, handle_id: str, data: bytes) -> bool:
        """Write to file"""
        pass
    
    @abstractmethod
    def close_file(self, handle_id: str) -> bool:
        """Close file"""
        pass
    
    @abstractmethod
    def delete_file(self, path: str) -> bool:
        """Delete file"""
        pass
    
    @abstractmethod
    def list_directory(self, path: str) -> List[FileMetadata]:
        """List directory contents"""
        pass

class VirtualFileSystem(IFileSystem):
    """Virtual file system implementation"""
    
    def __init__(self):
        self.files: Dict[str, FileMetadata] = {}
        self.file_data: Dict[str, bytearray] = {}
        self.open_handles: Dict[str, FileHandle] = {}
        self.next_handle_id = 1
        
        # Create root directory
        self._create_root_directory()
        
    def _create_root_directory(self):
        """Create root directory"""
        root_meta = FileMetadata(
            name="",
            path="/",
            file_type=FileType.DIRECTORY,
            permissions="rwxr-xr-x"
        )
        self.files["/"] = root_meta
        
    def _normalize_path(self, path: str) -> str:
        """Normalize file path"""
        if not path.startswith("/"):
            path = "/" + path
        return os.path.normpath(path).replace("\\", "/")
    
    def _get_parent_path(self, path: str) -> str:
        """Get parent directory path"""
        return os.path.dirname(path) or "/"
    
    def create_file(self, path: str, content: bytes = b"") -> bool:
        """Create a new file"""
        try:
            path = self._normalize_path(path)
            
            # Check if file already exists
            if path in self.files:
                return False
            
            # Check if parent directory exists
            parent_path = self._get_parent_path(path)
            if parent_path not in self.files:
                return False
            
            # Create file metadata
            filename = os.path.basename(path)
            metadata = FileMetadata(
                name=filename,
                path=path,
                file_type=FileType.REGULAR,
                size=len(content)
            )
            
            # Store file
            self.files[path] = metadata
            self.file_data[path] = bytearray(content)
            
            # Update STEPPPS metadata
            metadata.steppps_metadata["space"]["checksum"] = hashlib.md5(content).hexdigest()
            
            return True
            
        except Exception as e:
            print(f"❌ Failed to create file {path}: {e}")
            return False
    
    def create_directory(self, path: str) -> bool:
        """Create a new directory"""
        try:
            path = self._normalize_path(path)
            
            # Check if directory already exists
            if path in self.files:
                return False
            
            # Check if parent directory exists
            parent_path = self._get_parent_path(path)
            if parent_path not in self.files:
                return False
            
            # Create directory metadata
            dirname = os.path.basename(path)
            metadata = FileMetadata(
                name=dirname,
                path=path,
                file_type=FileType.DIRECTORY,
                permissions="rwxr-xr-x"
            )
            
            self.files[path] = metadata
            return True
            
        except Exception as e:
            print(f"❌ Failed to create directory {path}: {e}")
            return False
    
    def open_file(self, path: str, mode: str = "r") -> Optional[str]:
        """Open file and return handle ID"""
        try:
            path = self._normalize_path(path)
            
            # Check if file exists
            if path not in self.files:
                return None
            
            metadata = self.files[path]
            
            # Check if it's a regular file
            if metadata.file_type != FileType.REGULAR:
                return None
            
            # Create file handle
            handle_id = str(self.next_handle_id)
            self.next_handle_id += 1
            
            # Copy file data to handle buffer
            file_buffer = bytearray(self.file_data.get(path, bytearray()))
            
            handle = FileHandle(
                file_id=handle_id,
                metadata=metadata,
                mode=mode,
                buffer=file_buffer
            )
            
            self.open_handles[handle_id] = handle
            
            # Update access time
            metadata.accessed_time = time.time()
            metadata.steppps_metadata["time"]["last_accessed"] = metadata.accessed_time
            metadata.steppps_metadata["event"]["access_count"] += 1
            
            return handle_id
            
        except Exception as e:
            print(f"❌ Failed to open file {path}: {e}")
            return None
    
    def read_file(self, handle_id: str, size: int = -1) -> Optional[bytes]:
        """Read from file"""
        try:
            if handle_id not in self.open_handles:
                return None
            
            handle = self.open_handles[handle_id]
            
            # Check read permission
            if "r" not in handle.mode:
                return None
            
            # Read data
            if size == -1:
                data = bytes(handle.buffer[handle.position:])
                handle.position = len(handle.buffer)
            else:
                end_pos = min(handle.position + size, len(handle.buffer))
                data = bytes(handle.buffer[handle.position:end_pos])
                handle.position = end_pos
            
            # Update access time
            handle.metadata.accessed_time = time.time()
            
            return data
            
        except Exception as e:
            print(f"❌ Failed to read file: {e}")
            return None
    
    def write_file(self, handle_id: str, data: bytes) -> bool:
        """Write to file"""
        try:
            if handle_id not in self.open_handles:
                return False
            
            handle = self.open_handles[handle_id]
            
            # Check write permission
            if "w" not in handle.mode and "a" not in handle.mode:
                return False
            
            # Write data
            if "a" in handle.mode:  # Append mode
                handle.buffer.extend(data)
                handle.position = len(handle.buffer)
            else:  # Write mode
                # Extend buffer if necessary
                needed_size = handle.position + len(data)
                if needed_size > len(handle.buffer):
                    handle.buffer.extend(b'\x00' * (needed_size - len(handle.buffer)))
                
                # Write data at current position
                handle.buffer[handle.position:handle.position + len(data)] = data
                handle.position += len(data)
            
            # Update metadata
            handle.metadata.size = len(handle.buffer)
            handle.metadata.modified_time = time.time()
            handle.metadata.steppps_metadata["time"]["last_modified"] = handle.metadata.modified_time
            handle.metadata.steppps_metadata["event"]["modification_count"] += 1
            
            return True
            
        except Exception as e:
            print(f"❌ Failed to write file: {e}")
            return False
    
    def close_file(self, handle_id: str) -> bool:
        """Close file"""
        try:
            if handle_id not in self.open_handles:
                return False
            
            handle = self.open_handles[handle_id]
            
            # Write buffer back to file data if modified
            if "w" in handle.mode or "a" in handle.mode:
                self.file_data[handle.metadata.path] = handle.buffer
                
                # Update checksum
                checksum = hashlib.md5(handle.buffer).hexdigest()
                handle.metadata.steppps_metadata["space"]["checksum"] = checksum
            
            # Remove handle
            del self.open_handles[handle_id]
            
            return True
            
        except Exception as e:
            print(f"❌ Failed to close file: {e}")
            return False
    
    def delete_file(self, path: str) -> bool:
        """Delete file"""
        try:
            path = self._normalize_path(path)
            
            if path not in self.files:
                return False
            
            # Remove file data and metadata
            if path in self.file_data:
                del self.file_data[path]
            del self.files[path]
            
            return True
            
        except Exception as e:
            print(f"❌ Failed to delete file {path}: {e}")
            return False
    
    def list_directory(self, path: str) -> List[FileMetadata]:
        """List directory contents"""
        try:
            path = self._normalize_path(path)
            
            if path not in self.files:
                return []
            
            if self.files[path].file_type != FileType.DIRECTORY:
                return []
            
            # Find all files in this directory
            contents = []
            for file_path, metadata in self.files.items():
                if file_path != path and self._get_parent_path(file_path) == path:
                    contents.append(metadata)
            
            return contents
            
        except Exception as e:
            print(f"❌ Failed to list directory {path}: {e}")
            return []

class STEPPPSFileSystemManager:
    """STEPPPS File System Manager with multiple backend support"""
    
    def __init__(self):
        self.file_systems: Dict[str, IFileSystem] = {}
        self.mount_points: Dict[str, str] = {}  # path -> filesystem_name
        self.current_directory = "/"
        
        # Initialize default virtual file system
        self.file_systems["virtual"] = VirtualFileSystem()
        self.mount_points["/"] = "virtual"
        
        # File system statistics
        self.operations_count = 0
        self.bytes_read = 0
        self.bytes_written = 0
        
        # Add compatibility attributes for shutdown
        self.open_files = {}
    
    def close_file(self, handle: str) -> bool:
        """Close file handle"""
        if handle in self.open_files:
            del self.open_files[handle]
            return True
        return False
    
    def mount_filesystem(self, path: str, filesystem: IFileSystem, name: str) -> bool:
        """Mount a file system at specified path"""
        try:
            path = self._normalize_path(path)
            
            self.file_systems[name] = filesystem
            self.mount_points[path] = name
            
            print(f"📁 Mounted {name} filesystem at {path}")
            return True
            
        except Exception as e:
            print(f"❌ Failed to mount filesystem: {e}")
            return False
    
    def _normalize_path(self, path: str) -> str:
        """Normalize file path"""
        if not path.startswith("/"):
            path = os.path.join(self.current_directory, path)
        return os.path.normpath(path).replace("\\", "/")
    
    def _get_filesystem_for_path(self, path: str) -> Optional[IFileSystem]:
        """Get file system for given path"""
        path = self._normalize_path(path)
        
        # Find the longest matching mount point
        best_match = ""
        for mount_point in self.mount_points:
            if path.startswith(mount_point) and len(mount_point) > len(best_match):
                best_match = mount_point
        
        if best_match:
            fs_name = self.mount_points[best_match]
            return self.file_systems.get(fs_name)
        
        return None
    
    def create_file(self, path: str, content: str = "") -> bool:
        """Create a new file"""
        fs = self._get_filesystem_for_path(path)
        if fs:
            self.operations_count += 1
            return fs.create_file(path, content.encode())
        return False
    
    def create_directory(self, path: str) -> bool:
        """Create a new directory"""
        fs = self._get_filesystem_for_path(path)
        if fs:
            self.operations_count += 1
            return fs.create_directory(path)
        return False
    
    def read_file(self, path: str) -> Optional[str]:
        """Read entire file content"""
        fs = self._get_filesystem_for_path(path)
        if not fs:
            return None
        
        handle_id = fs.open_file(path, "r")
        if not handle_id:
            return None
        
        try:
            data = fs.read_file(handle_id)
            if data:
                self.bytes_read += len(data)
                return data.decode('utf-8', errors='ignore')
            return ""
        finally:
            fs.close_file(handle_id)
            self.operations_count += 1
    
    def write_file(self, path: str, content: str) -> bool:
        """Write content to file"""
        fs = self._get_filesystem_for_path(path)
        if not fs:
            return False
        
        # Create file if it doesn't exist
        if not fs.create_file(path):
            # File might already exist, try to open it
            pass
        
        handle_id = fs.open_file(path, "w")
        if not handle_id:
            return False
        
        try:
            data = content.encode()
            success = fs.write_file(handle_id, data)
            if success:
                self.bytes_written += len(data)
            return success
        finally:
            fs.close_file(handle_id)
            self.operations_count += 1
    
    def delete_file(self, path: str) -> bool:
        """Delete file"""
        fs = self._get_filesystem_for_path(path)
        if fs:
            self.operations_count += 1
            return fs.delete_file(path)
        return False
    
    def list_directory(self, path: str = None) -> List[Dict[str, Any]]:
        """List directory contents"""
        if path is None:
            path = self.current_directory
        
        fs = self._get_filesystem_for_path(path)
        if not fs:
            return []
        
        self.operations_count += 1
        metadata_list = fs.list_directory(path)
        
        # Convert to dict format
        result = []
        for metadata in metadata_list:
            file_info = {
                "name": metadata.name,
                "path": metadata.path,
                "type": metadata.file_type.value,
                "size": metadata.size,
                "permissions": metadata.permissions,
                "modified": metadata.modified_time,
                "icon": metadata.steppps_metadata["pixel"]["file_icon"],
                "steppps_enabled": True
            }
            result.append(file_info)
        
        return result
    
    def change_directory(self, path: str) -> bool:
        """Change current directory"""
        path = self._normalize_path(path)
        fs = self._get_filesystem_for_path(path)
        
        if fs:
            # Check if directory exists
            contents = fs.list_directory(path)
            if contents is not None:  # Directory exists
                self.current_directory = path
                return True
        
        return False
    
    def get_current_directory(self) -> str:
        """Get current directory"""
        return self.current_directory
    
    def get_filesystem_stats(self) -> Dict[str, Any]:
        """Get file system statistics"""
        return {
            "mounted_filesystems": len(self.file_systems),
            "mount_points": list(self.mount_points.keys()),
            "current_directory": self.current_directory,
            "operations_count": self.operations_count,
            "bytes_read": self.bytes_read,
            "bytes_written": self.bytes_written,
            "total_io": self.bytes_read + self.bytes_written
        }

def main():
    """Demo of STEPPPS File System"""
    print("📁 STEPPPS File System Demo")
    print("=" * 30)
    
    # Create file system manager
    fsm = STEPPPSFileSystemManager()
    
    try:
        # Create some directories
        print("\n📁 Creating directories...")
        fsm.create_directory("/home")
        fsm.create_directory("/home/user")
        fsm.create_directory("/tmp")
        fsm.create_directory("/etc")
        
        # Create some files
        print("\n📄 Creating files...")
        fsm.create_file("/home/user/hello.txt", "Hello STEPPPS File System!")
        fsm.create_file("/home/user/config.json", '{"steppps": true, "version": "1.0"}')
        fsm.create_file("/tmp/temp.log", "Temporary log file\nLine 2\nLine 3")
        fsm.create_file("/etc/system.conf", "# STEPPPS System Configuration\nmode=autonomous")
        
        # List root directory
        print("\n📋 Root directory contents:")
        root_contents = fsm.list_directory("/")
        for item in root_contents:
            print(f"   {item['icon']} {item['name']} ({item['type']}) - {item['size']} bytes")
        
        # List user directory
        print("\n📋 User directory contents:")
        user_contents = fsm.list_directory("/home/user")
        for item in user_contents:
            print(f"   {item['icon']} {item['name']} ({item['type']}) - {item['size']} bytes")
        
        # Read files
        print("\n📖 Reading files...")
        hello_content = fsm.read_file("/home/user/hello.txt")
        print(f"   hello.txt: {hello_content}")
        
        config_content = fsm.read_file("/home/user/config.json")
        print(f"   config.json: {config_content}")
        
        # Modify a file
        print("\n✏️  Modifying file...")
        fsm.write_file("/home/user/hello.txt", "Hello STEPPPS! File has been modified.")
        modified_content = fsm.read_file("/home/user/hello.txt")
        print(f"   Modified hello.txt: {modified_content}")
        
        # Change directory
        print("\n📂 Changing directory...")
        fsm.change_directory("/home/user")
        print(f"   Current directory: {fsm.get_current_directory()}")
        
        # List current directory
        current_contents = fsm.list_directory()
        print("   Contents:")
        for item in current_contents:
            print(f"     {item['icon']} {item['name']}")
        
        # Show statistics
        print("\n📊 File System Statistics:")
        stats = fsm.get_filesystem_stats()
        print(f"   Operations: {stats['operations_count']}")
        print(f"   Bytes Read: {stats['bytes_read']}")
        print(f"   Bytes Written: {stats['bytes_written']}")
        print(f"   Mount Points: {stats['mount_points']}")
        
        # Delete a file
        print("\n🗑️  Deleting file...")
        fsm.delete_file("/tmp/temp.log")
        tmp_contents = fsm.list_directory("/tmp")
        print(f"   /tmp contents after deletion: {len(tmp_contents)} items")
        
    except Exception as e:
        print(f"❌ Demo error: {e}")

if __name__ == "__main__":
    main()
