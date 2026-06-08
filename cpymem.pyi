# cpymem.pyi
from typing import Any

def PingPong(a: Any, b: Any) -> None:
    """Swap two variable bindings in locals"""
    ...

def Copy(dest: Any, src: Any, size: int) -> None:
    """Copy Src to Dest memory"""
    ...

def Alloc(size: int) -> Any:
    """Alloc RWX Memory, returns a CMemory capsule"""
    ...

def Free(capsule: Any) -> None:
    """Free Memory capsule"""
    ...

def Write(capsule: Any, offset: int, byte: int) -> None:
    """Write a byte to Memory"""
    ...

def Read(capsule: Any, offset: int) -> int:
    """Read a byte from Memory"""
    ...

