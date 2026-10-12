"""Symbolicate Shipping crash offsets against the local PDB."""
import ctypes
import struct
from ctypes import wintypes
from pathlib import Path

BIN = Path(
    r"C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311"
    r"\Packaged\Win64Shipping\TerraDyneOSS_Proto\Binaries\Win64"
)
EXE = BIN / "TerraDyneOSS_Proto-Win64-Shipping.exe"

STACKS = {
    "D3D12_PCallStack_and_crashed_worker": [
        0x4CBCE2F,
        0x3BA80E4,
        0x3BA4F22,
        0x125E0AF,
        0x124E640,
        0x1251D5C,
        0x1260190,
        0x124B16A,
        0x12DB8B4,
        0x1437237,
        0x1436A8F,
        0x142FFFF,
        0x76B15AE,
    ],
    "D3D11_RenderThread": [
        0x4CBCE2F,
        0x3BA80E4,
        0x3BA4F22,
        0x125E0AF,
        0x141B32E,
        0x141B27D,
        0x141BD98,
        0x141BCA8,
        0x3BA6B79,
        0x344D228,
        0x3B65E70,
        0x3B7103C,
        0x32AD1D7,
        0x1251A88,
        0x1258051,
        0x32C18F9,
        0x32C1AF7,
        0x1437237,
        0x1436A8F,
    ],
    "D3D12_GameThread_snapshot": [
        0x1B5E073,
        0x1B7E0D2,
        0x4F8ADEE,
        0x4CB9A2B,
        0x4CB985A,
        0x4F8BB26,
        0x4F8C856,
        0x25C0D6A,
        0x25ADDDB,
        0x310D9CC,
        0x125E0AF,
        0x1257F0F,
        0x125827C,
        0x3119B47,
        0x311E67E,
        0x2BC18BC,
        0x2A1221C,
        0x4646ED4,
        0x463EFFC,
        0x463F06A,
        0x46404C3,
        0x4647C44,
        0x769F472,
    ],
    "D3D12_ForegroundWorker0_nv_path": [
        0x43E76BC,
        0x440E832,
        0x43DF541,
        0x440C91D,
        0x4408895,
        0x43E2F35,
        0x332A25D,
        0x332214F,
        0x332A869,
        0x332AF0B,
    ],
}


class SYMBOL_INFO(ctypes.Structure):
    _fields_ = [
        ("SizeOfStruct", wintypes.ULONG),
        ("TypeIndex", wintypes.ULONG),
        ("Reserved", ctypes.c_uint64 * 2),
        ("Index", wintypes.ULONG),
        ("Size", wintypes.ULONG),
        ("ModBase", ctypes.c_uint64),
        ("Flags", wintypes.ULONG),
        ("Value", ctypes.c_uint64),
        ("Address", ctypes.c_uint64),
        ("Register", wintypes.ULONG),
        ("Scope", wintypes.ULONG),
        ("Tag", wintypes.ULONG),
        ("NameLen", wintypes.ULONG),
        ("MaxNameLen", wintypes.ULONG),
        ("Name", ctypes.c_char * 2048),
    ]


class IMAGEHLP_LINE64(ctypes.Structure):
    _fields_ = [
        ("SizeOfStruct", wintypes.DWORD),
        ("Key", ctypes.c_void_p),
        ("LineNumber", wintypes.DWORD),
        ("FileName", ctypes.c_char_p),
        ("Address", ctypes.c_uint64),
    ]


ctypes.WinDLL(r"C:\Program Files\Microsoft Visual Studio\18\Community\DIA SDK\bin\amd64\msdia140.dll")
dbghelp = ctypes.WinDLL(r"C:\Windows\System32\dbghelp.dll")
kernel32 = ctypes.WinDLL("kernel32")
dbghelp.SymSetOptions(0x00000002 | 0x00000010)
hProcess = kernel32.GetCurrentProcess()
dbghelp.SymInitializeW.argtypes = [wintypes.HANDLE, wintypes.LPCWSTR, wintypes.BOOL]
dbghelp.SymInitializeW.restype = wintypes.BOOL
if not dbghelp.SymInitializeW(hProcess, str(BIN), False):
    raise SystemExit(f"SymInitialize failed {kernel32.GetLastError()}")

data = EXE.read_bytes()
e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
size_of_image = struct.unpack_from("<I", data, e_lfanew + 24 + 56)[0]
BASE = 0x180000000
dbghelp.SymLoadModuleExW.argtypes = [
    wintypes.HANDLE,
    wintypes.HANDLE,
    wintypes.LPCWSTR,
    wintypes.LPCWSTR,
    ctypes.c_uint64,
    wintypes.DWORD,
    ctypes.c_void_p,
    wintypes.DWORD,
]
dbghelp.SymLoadModuleExW.restype = ctypes.c_uint64
dbghelp.SymLoadModuleExW(hProcess, None, str(EXE), None, BASE, size_of_image, None, 0)

dbghelp.SymFromAddr.argtypes = [
    wintypes.HANDLE,
    ctypes.c_uint64,
    ctypes.POINTER(ctypes.c_uint64),
    ctypes.POINTER(SYMBOL_INFO),
]
dbghelp.SymFromAddr.restype = wintypes.BOOL
dbghelp.SymGetLineFromAddr64.argtypes = [
    wintypes.HANDLE,
    ctypes.c_uint64,
    ctypes.POINTER(wintypes.DWORD),
    ctypes.POINTER(IMAGEHLP_LINE64),
]
dbghelp.SymGetLineFromAddr64.restype = wintypes.BOOL


def resolve(off: int) -> str:
    info = SYMBOL_INFO()
    info.SizeOfStruct = 88
    info.MaxNameLen = 2048
    disp = ctypes.c_uint64(0)
    if not dbghelp.SymFromAddr(hProcess, BASE + off, ctypes.byref(disp), ctypes.byref(info)):
        return f"{off:#010x}  NO_SYM err={kernel32.GetLastError()}"
    name = info.Name.decode("utf-8", "replace")
    extra = ""
    line_info = IMAGEHLP_LINE64()
    line_info.SizeOfStruct = ctypes.sizeof(IMAGEHLP_LINE64)
    disp32 = wintypes.DWORD(0)
    if dbghelp.SymGetLineFromAddr64(hProcess, BASE + off, ctypes.byref(disp32), ctypes.byref(line_info)):
        fname = line_info.FileName.decode("utf-8", "replace") if line_info.FileName else "?"
        extra = f"  [{fname}:{line_info.LineNumber}]"
    return f"{off:#010x}  {name}+{disp.value:#x}{extra}"


out_path = Path(
    r"C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\Results\shipping_symbols.txt"
)
lines = []
for title, offs in STACKS.items():
    lines.append(f"==== {title} ====")
    for off in offs:
        lines.append(resolve(off))
    lines.append("")
text = "\n".join(lines)
out_path.write_text(text, encoding="utf-8")
print(text)
dbghelp.SymCleanup(hProcess)
