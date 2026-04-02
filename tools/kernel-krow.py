#!/usr/bin/env python3
"""
KROW DIAGNOSTICS ERROR HANDLER v1.0
Detailed error analysis and recovery suggestions

Accepts error format: SECTION:SUBSECTION:TEST=STATE
Example: BATTERY:PERCENT:TST=FAULT
"""

import sys
import re
from datetime import datetime

# ANSI Color codes
COLOR_RED = "\033[31m"
COLOR_YELLOW = "\033[33m"
COLOR_GREEN = "\033[32m"
COLOR_CYAN = "\033[36m"
COLOR_BOLD = "\033[1m"
COLOR_RESET = "\033[0m"

# ============================================================================
# ERROR KNOWLEDGE BASE
# ============================================================================

ERROR_DATABASE = {
    "PAGING": {
        "description": "Memory paging system failure",
        "analysis": "The memory management unit (MMU) or page table initialization failed.",
        "fixes": [
            "Verify physical memory is installed correctly",
            "Check kernel page table initialization in arch/x86/paging.c",
            "Ensure PAE (Physical Address Extension) is enabled if needed",
            "Run: dmesg | grep -i paging"
        ],
        "severity": "CRITICAL"
    },
    "VGA_S1": {
        "description": "Video serial port 1 communication failure",
        "analysis": "Serial communication on VGA serial 1 is not responding.",
        "fixes": [
            "Check VGA serial port driver initialization",
            "Verify UART hardware is present and enabled",
            "Check baudrate settings (typically 115200)",
            "Review kernel/arch/x86/framebuffer.c serial port setup"
        ],
        "severity": "HIGH"
    },
    "VGA_S2": {
        "description": "Video serial port 2 communication failure",
        "analysis": "Serial communication on VGA serial 2 is not responding.",
        "fixes": [
            "Check secondary serial port driver",
            "Verify hardware resource allocation (IRQ/IO port)",
            "Ensure dual serial support is compiled in",
            "Review kernel/arch/x86/framebuffer.c for dual port setup"
        ],
        "severity": "HIGH"
    },
    "CDROM": {
        "description": "CDROM controller detection failure",
        "analysis": "The CD/DVD-ROM drive interface is not responding.",
        "fixes": [
            "Check ATAPI/SATA driver initialization",
            "Verify CD/DVD drive is powered and connected",
            "Check kernel log for IDE/SATA errors",
            "Review kernel/core/pci.c for controller enumeration"
        ],
        "severity": "MEDIUM"
    },
    "MEMORY": {
        "description": "System memory test failure",
        "analysis": "Memory diagnostic found potential memory corruption or defect.",
        "fixes": [
            "Run memory test: memtest86+ from bootloader",
            "Check memory stick seating",
            "Try single memory module at a time",
            "Check BIOS/UEFI for memory errors",
            "Review kernel/core/memory.c memory initialization"
        ],
        "severity": "CRITICAL"
    },
    "DRIVERS": {
        "description": "Driver subsystem initialization failure",
        "analysis": "Core driver loading framework failed.",
        "fixes": [
            "Check kernel/core for missing driver_init()",
            "Verify all required driver modules are present",
            "Check dmesg for driver load errors",
            "Ensure driver search paths are correct"
        ],
        "severity": "CRITICAL"
    },
    "DRV_KBD": {
        "description": "Keyboard driver failure",
        "analysis": "Keyboard input driver initialization failed.",
        "fixes": [
            "Check keyboard is connected and powered",
            "Verify PS/2 or USB HID driver is compiled",
            "Reload keyboard driver: modprobe -r; modprobe <driver>",
            "Check kernel/gui/input.c for keyboard setup"
        ],
        "severity": "MEDIUM"
    },
    "DRV_MOU": {
        "description": "Mouse driver failure",
        "analysis": "Pointing device (mouse) driver failed to initialize.",
        "fixes": [
            "Check mouse is connected and has power",
            "Verify PS/2 or USB mouse driver is enabled",
            "Reload mouse driver: modprobe -r; modprobe <driver>",
            "Review kernel/gui/input.c mouse initialization"
        ],
        "severity": "LOW"
    },
    "DRV_WBC": {
        "description": "Web controller driver failure",
        "analysis": "Network/web control interface driver failed.",
        "fixes": [
            "Check network interface presence: ip link show",
            "Verify network driver is loaded: lsmod",
            "Review kernel/core/net.c initialization",
            "Check PCI device enumeration in dmesg"
        ],
        "severity": "MEDIUM"
    },
    "DRV_SOU": {
        "description": "Sound driver failure",
        "analysis": "Audio subsystem driver initialization failed.",
        "fixes": [
            "Check sound card is detected: aplay -l",
            "Verify ALSA/audio driver is compiled",
            "Check volume levels and muting",
            "Look for PCI enumeration errors in dmesg"
        ],
        "severity": "LOW"
    },
    "DRV_3DG": {
        "description": "3D GPU driver failure",
        "analysis": "3D graphics processing unit driver failed.",
        "fixes": [
            "Check GPU is detected: lspci | grep -i vga",
            "Verify proprietary/open GPU drivers are installed",
            "Check X11/Wayland display server logs",
            "Review kernel/gui/draw.c for 3D initialization"
        ],
        "severity": "MEDIUM"
    },
    "DRV_WEB": {
        "description": "Network connectivity driver failure",
        "analysis": "Network (Ethernet/WiFi) interface driver failed.",
        "fixes": [
            "Check network interface: ip link show",
            "Verify network driver module: lsmod | grep net",
            "Reload driver: service networking restart",
            "Review kernel/core/net.c network initialization"
        ],
        "severity": "HIGH"
    },
    "DRV_ENH": {
        "description": "Enhanced features driver failure",
        "analysis": "Optional enhanced features driver module failed to load.",
        "fixes": [
            "Check enhanced features are not mandatory",
            "Review what enhanced features are enabled",
            "Disable optional features if not critical",
            "Check driver dependencies in dmesg"
        ],
        "severity": "LOW"
    },
    "DRV_JCK": {
        "description": "Audio jack detection failure",
        "analysis": "Audio jack detection subsystem (ALSA/codec) failed.",
        "fixes": [
            "Check sound card: cat /proc/asound/cards",
            "Verify ALSA codec driver: aplay -l",
            "Reload sound driver: modprobe -r snd; modprobe snd",
            "Check jack detection in alsamixer"
        ],
        "severity": "LOW"
    },
    "BATTERY": {
        "description": "Battery subsystem failure",
        "analysis": "Battery health monitoring or power management failed.",
        "fixes": [
            "Check battery connection and contacts",
            "Review power management firmware",
            "Check kernel power management driver: ls /sys/class/power_supply/",
            "Verify ACPI/battery driver is enabled"
        ],
        "severity": "MEDIUM"
    },
    "DISPLAY": {
        "description": "Display subsystem failure",
        "analysis": "Screen rendering, framebuffer, or display driver failed.",
        "fixes": [
            "Check display cable connections",
            "Verify framebuffer driver is initialized",
            "Review kernel/arch/x86/framebuffer*.c",
            "Check display mode resolution settings"
        ],
        "severity": "HIGH"
    },
    "MULTIBOOT": {
        "description": "Multiboot ISO verification failure",
        "analysis": "ISO boot structure or multiboot headers are corrupted.",
        "fixes": [
            "Verify ISO integrity: sha256sum krowtest.iso",
            "Regenerate ISO from source",
            "Check Makefile build rules",
            "Verify multiboot header in kernel ELF"
        ],
        "severity": "CRITICAL"
    }
}

# ============================================================================
# ERROR PARSER
# ============================================================================

def parse_error_code(error_code):
    """
    Parse error format: SECTION:SUBSECTION:TEST=STATE
    Example: BATTERY:PERCENT:TST=FAULT
    
    Returns: (section, subsection, test, state)
    """
    match = re.match(r'([A-Z_]+):([A-Z_]+):([A-Z]+)=([A-Z]+)', error_code)
    if not match:
        return (None, None, None, None)
    
    return match.groups()

# ============================================================================
# ERROR REPORT GENERATOR
# ============================================================================

def generate_error_report(error_code):
    """Generate detailed error report from error code"""
    section, subsection, test, state = parse_error_code(error_code)
    
    if not section:
        print(f"{COLOR_RED}[ERROR] Invalid error format: {error_code}{COLOR_RESET}")
        print("Expected format: SECTION:SUBSECTION:TEST=STATE")
        print("Example: BATTERY:PERCENT:TST=FAULT")
        return False
    
    # Get error details from database
    error_info = ERROR_DATABASE.get(section) or ERROR_DATABASE.get(subsection)
    
    if not error_info:
        error_info = {
            "description": f"Unknown error in {section}/{subsection}",
            "analysis": f"The {section} subsystem encountered a {test} test failure.",
            "fixes": [
                f"Check {section} initialization in kernel code",
                "Review recent kernel changes",
                "Check hardware connections",
                "Run: dmesg | tail -50"
            ],
            "severity": "UNKNOWN"
        }
    
    # Severity colors
    severity_color = {
        "CRITICAL": COLOR_RED + COLOR_BOLD,
        "HIGH": COLOR_RED,
        "MEDIUM": COLOR_YELLOW,
        "LOW": COLOR_GREEN,
    }.get(error_info["severity"], COLOR_CYAN)
    
    # Print formatted report
    print()
    print(f"{COLOR_BOLD}{COLOR_CYAN}[ERROR REPORT]{COLOR_RESET}")
    print("=" * 70)
    print()
    
    print(f"  {COLOR_BOLD}Subsystem:{COLOR_RESET}")
    print(f"    Section:     {COLOR_CYAN}{section}{COLOR_RESET}")
    print(f"    Component:   {COLOR_CYAN}{subsection}{COLOR_RESET}")
    print(f"    Test:        {COLOR_CYAN}{test}{COLOR_RESET}")
    print(f"    Status:      {COLOR_RED}{state}{COLOR_RESET}")
    print()
    
    print(f"  {COLOR_BOLD}Description:{COLOR_RESET}")
    print(f"    {error_info['description']}")
    print()
    
    print(f"  {COLOR_BOLD}Analysis:{COLOR_RESET}")
    print(f"    {error_info['analysis']}")
    print()
    
    print(f"  {COLOR_BOLD}Suggested Fixes:{COLOR_RESET}")
    for i, fix in enumerate(error_info['fixes'], 1):
        print(f"    {i}. {fix}")
    print()
    
    print(f"  {COLOR_BOLD}Severity:{COLOR_RESET} {severity_color}{error_info['severity']}{COLOR_RESET}")
    print()
    
    print("=" * 70)
    print()
    
    # Log error
    log_error_to_file(error_code, section, subsection, test, state, error_info)
    
    return True

# ============================================================================
# LOGGING
# ============================================================================

def log_error_to_file(error_code, section, subsection, test, state, error_info):
    """Append error report to diagnostic log"""
    try:
        with open("/var/log/krow_diag.log", "a") as logfile:
            logfile.write(f"\n{'='*70}\n")
            logfile.write(f"[ERROR] {datetime.now().isoformat()}\n")
            logfile.write(f"Code: {error_code}\n")
            logfile.write(f"Section: {section}/{subsection}\n")
            logfile.write(f"Test: {test} = {state}\n")
            logfile.write(f"Severity: {error_info['severity']}\n")
            logfile.write(f"Description: {error_info['description']}\n")
            logfile.write(f"Analysis: {error_info['analysis']}\n")
            logfile.write(f"{'='*70}\n")
    except Exception as e:
        # Silently fail if log file can't be written
        pass

# ============================================================================
# MAIN
# ============================================================================

def main():
    if len(sys.argv) < 2:
        print(f"{COLOR_RED}[ERROR] No error code provided{COLOR_RESET}")
        print("Usage: python3 kernel-krow.py \"SECTION:SUBSECTION:TEST=STATE\"")
        print("Example: python3 kernel-krow.py \"BATTERY:PERCENT:TST=FAULT\"")
        sys.exit(1)
    
    error_code = sys.argv[1]
    success = generate_error_report(error_code)
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
