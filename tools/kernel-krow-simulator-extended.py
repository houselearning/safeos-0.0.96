#!/usr/bin/env python3
"""
Krow Diagnostics Simulator - Virtual Boot Diagnostic Environment
Simulates Krow diagnostics running during OS boot sequence
Supports both standard and extended (force) modes
"""

import sys
import time
import random

# ANSI colors
class Color:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    RESET = '\033[0m'
    BOLD = '\033[1m'

# Deterministic RNG (Linear Congruential Generator)
class LCG:
    def __init__(self, seed=42):
        self.seed = seed
        self.a = 1103515245
        self.c = 12345
        self.m = 2**31
    
    def next(self):
        self.seed = (self.a * self.seed + self.c) % self.m
        return self.seed / self.m

def simulated_delay(min_ms=10, max_ms=50):
    """Simulate realistic diagnostic delay"""
    delay_ms = random.uniform(min_ms, max_ms)
    time.sleep(delay_ms / 1000.0)

def run_core_checks():
    """Simulate 6 core diagnostic checks"""
    print(f"\n{Color.YELLOW}╔═══════════════════════════════════════╗{Color.RESET}")
    print(f"{Color.YELLOW}║         CORE DIAGNOSTICS              ║{Color.RESET}")
    print(f"{Color.YELLOW}╚═══════════════════════════════════════╝{Color.RESET}\n")
    
    checks = [
        ("CPU", "x86 processor detection and features"),
        ("MEMORY", "RAM detection and page table setup"),
        ("GDT", "Global Descriptor Table initialization"),
        ("IDT", "Interrupt Descriptor Table setup"),
        ("PIC", "Programmable Interrupt Controller"),
        ("TIMER", "System timer and clock synchronization"),
    ]
    
    for name, description in checks:
        simulated_delay(20, 100)
        rng = LCG()
        fault_chance = rng.next()
        
        if fault_chance < 0.15:  # 15% fault rate
            status = f"{Color.RED}FAIL{Color.RESET}"
            result = f"Error: {name} initialization failed"
        else:
            status = f"{Color.GREEN}PASS{Color.RESET}"
            result = f"{description} OK"
        
        print(f"  [{status}] {Color.BOLD}{name}{Color.RESET}: {result}")
    
    print()

def run_driver_checks():
    """Simulate 8 driver checks"""
    print(f"{Color.YELLOW}╔═══════════════════════════════════════╗{Color.RESET}")
    print(f"{Color.YELLOW}║       DRIVER DIAGNOSTICS              ║{Color.RESET}")
    print(f"{Color.YELLOW}╚═══════════════════════════════════════╝{Color.RESET}\n")
    
    drivers = [
        ("SERIAL", "Serial console driver (COM1)"),
        ("VGA", "VGA text mode display"),
        ("FRAMEBUFFER", "VESA framebuffer graphics"),
        ("KEYBOARD", "USB/PS2 keyboard input"),
        ("MOUSE", "PS2 mouse input device"),
        ("ATA", "IDE/ATA disk controller"),
        ("RTL8139", "Realtek 8139 network card"),
        ("E1000", "Intel E1000 network adapter"),
    ]
    
    for name, description in drivers:
        simulated_delay(30, 150)
        rng = LCG()
        fault_chance = rng.next()
        
        if fault_chance < 0.15:  # 15% fault rate
            status = f"{Color.RED}FAIL{Color.RESET}"
            result = "Device not detected or failed to initialize"
        else:
            status = f"{Color.GREEN}PASS{Color.RESET}"
            result = description + " initialized"
        
        print(f"  [{status}] {Color.BOLD}{name}{Color.RESET}: {result}")
    
    print()

def run_extended_tests():
    """Simulate comprehensive boot checks"""
    print(f"{Color.YELLOW}╔═══════════════════════════════════════╗{Color.RESET}")
    print(f"{Color.YELLOW}║     EXTENDED DIAGNOSTICS              ║{Color.RESET}")
    print(f"{Color.YELLOW}╚═══════════════════════════════════════╝{Color.RESET}\n")
    
    extended = [
        ("VGA_MEMORY", "VGA memory mapped region detection"),
        ("VIDEO_MODE", "Current video mode querying"),
        ("FONT_LOADED", "Font memory validation"),
        ("BIOS_INFO", "BIOS version and date"),
        ("ACPI_TABLES", "ACPI description tables"),
        ("INTERRUPTS", "Interrupt handler validation"),
    ]
    
    for name, description in extended:
        simulated_delay(25, 120)
        rng = LCG()
        fault_chance = rng.next()
        
        if fault_chance < 0.1:  # 10% fault rate for extended checks
            status = f"{Color.RED}FAIL{Color.RESET}"
            result = "Check failed"
        else:
            status = f"{Color.GREEN}PASS{Color.RESET}"
            result = description + " OK"
        
        print(f"  [{status}] {Color.BOLD}{name}{Color.RESET}: {result}")
    
    print()

def run_comprehensive_tests(iterations=50):
    """Run comprehensive stress tests (600+ combinations)"""
    print(f"{Color.YELLOW}╔═══════════════════════════════════════╗{Color.RESET}")
    print(f"{Color.YELLOW}║    COMPREHENSIVE STRESS TESTS         ║{Color.RESET}")
    print(f"{Color.YELLOW}║    Running {iterations}x iterations (≈600 tests)      ║{Color.RESET}")
    print(f"{Color.YELLOW}╚═══════════════════════════════════════╝{Color.RESET}\n")
    
    checks = ["CPU", "MEMORY", "GDT", "IDT", "PIC", "TIMER"]
    drivers = ["SERIAL", "VGA", "FRAMEBUFFER", "KEYBOARD", "MOUSE", "ATA", "RTL8139", "E1000"]
    
    passed = 0
    failed = 0
    
    for iteration in range(iterations):
        for check in checks:
            rng = LCG(seed=42 + iteration)
            fault_chance = rng.next()
            if fault_chance >= 0.15:
                passed += 1
            else:
                failed += 1
            simulated_delay(5, 15)
        
        for driver in drivers:
            rng = LCG(seed=100 + iteration)
            fault_chance = rng.next()
            if fault_chance >= 0.15:
                passed += 1
            else:
                failed += 1
            simulated_delay(5, 15)
        
        progress = ((iteration + 1) / iterations) * 100
        print(f"  [{Color.CYAN}{progress:5.1f}%{Color.RESET}] Iteration {iteration+1}/{iterations} - Checks passed: {passed}, failed: {failed}")
    
    print(f"\n  {Color.GREEN}[✓] Comprehensive tests complete: {passed} passed, {failed} failed{Color.RESET}\n")
    return passed, failed

def run_network_diagnostics(mini_tests=40):
    """Run network and connectivity mini-tests"""
    print(f"{Color.YELLOW}╔═══════════════════════════════════════╗{Color.RESET}")
    print(f"{Color.YELLOW}║    NETWORK & CONNECTIVITY TESTS       ║{Color.RESET}")
    print(f"{Color.YELLOW}║    Running {mini_tests} mini-tests             ║{Color.RESET}")
    print(f"{Color.YELLOW}╚═══════════════════════════════════════╝{Color.RESET}\n")
    
    network_tests = [
        ("IFCONFIG", "Interface enumeration and configuration"),
        ("PING_LOCAL", "Loopback (127.0.0.1) connectivity"),
        ("PING_GW", "Gateway connectivity check"),
        ("DNS_RESOLVE", "DNS resolution (8.8.8.8)"),
        ("DNS_LOCAL", "Local DNS server check"),
        ("TCP_CONNECT", "TCP port connectivity test"),
        ("UDP_SEND", "UDP packet transmission"),
        ("ARP_TABLE", "ARP cache table validation"),
        ("ROUTE_TABLE", "Routing table integrity"),
        ("SOCKET_API", "Socket API functionality"),
    ]
    
    passed = 0
    failed = 0
    
    for i in range(mini_tests):
        test = network_tests[i % len(network_tests)]
        name, description = test
        
        rng = LCG(seed=200 + i)
        fault_chance = rng.next()
        
        if fault_chance < 0.2:  # 20% fault rate for network
            status = f"{Color.RED}FAIL{Color.RESET}"
            result = "Network test failed"
            failed += 1
        else:
            status = f"{Color.GREEN}PASS{Color.RESET}"
            result = description
            passed += 1
        
        print(f"  [{status}] {Color.BOLD}{name}{Color.RESET}: {result}")
        simulated_delay(10, 50)
    
    print(f"\n  {Color.GREEN}[✓] Network tests complete: {passed} passed, {failed} failed{Color.RESET}\n")
    return passed, failed

def run_iso_integrity_tests(mini_tests=20):
    """Run ISO filesystem integrity mini-tests"""
    print(f"{Color.YELLOW}╔═══════════════════════════════════════╗{Color.RESET}")
    print(f"{Color.YELLOW}║    ISO FILESYSTEM INTEGRITY TESTS     ║{Color.RESET}")
    print(f"{Color.YELLOW}║    Running {mini_tests} mini-tests             ║{Color.RESET}")
    print(f"{Color.YELLOW}╚═══════════════════════════════════════╝{Color.RESET}\n")
    
    iso_tests = [
        ("ISO_BOOT", "ISO boot sector validation"),
        ("VOLUME_DESC", "Volume descriptor table"),
        ("PATH_TABLE", "Path table integrity"),
        ("DIR_RECORDS", "Directory record structure"),
        ("FILE_EXTENTS", "File extent allocation"),
        ("JOLIET_EXT", "Joliet extension support"),
        ("ROCK_RIDGE", "Rock Ridge extension check"),
        ("CRC_CHECK", "CRC32 integrity validation"),
        ("SYMLINK_RES", "Symlink resolution"),
        ("LARGE_FILES", "Large file handling (>2GB)"),
    ]
    
    passed = 0
    failed = 0
    
    for i in range(mini_tests):
        test = iso_tests[i % len(iso_tests)]
        name, description = test
        
        rng = LCG(seed=300 + i)
        fault_chance = rng.next()
        
        if fault_chance < 0.1:  # 10% fault rate for ISO
            status = f"{Color.RED}FAIL{Color.RESET}"
            result = "ISO validation failed"
            failed += 1
        else:
            status = f"{Color.GREEN}PASS{Color.RESET}"
            result = description
            passed += 1
        
        print(f"  [{status}] {Color.BOLD}{name}{Color.RESET}: {result}")
        simulated_delay(15, 60)
    
    print(f"\n  {Color.GREEN}[✓] ISO integrity tests complete: {passed} passed, {failed} failed{Color.RESET}\n")
    return passed, failed

def run_hardware_detection_tests(mini_tests=40):
    """Run advanced hardware detection mini-tests"""
    print(f"{Color.YELLOW}╔═══════════════════════════════════════╗{Color.RESET}")
    print(f"{Color.YELLOW}║    ADVANCED HARDWARE DETECTION        ║{Color.RESET}")
    print(f"{Color.YELLOW}║    Running {mini_tests} mini-tests             ║{Color.RESET}")
    print(f"{Color.YELLOW}╚═══════════════════════════════════════╝{Color.RESET}\n")
    
    hw_tests = [
        ("PCI_ENUM", "PCI bus enumeration"),
        ("PCI_CONFIG", "PCI configuration space access"),
        ("USB_SCAN", "USB device scanning"),
        ("ACPI_PARSE", "ACPI table parsing"),
        ("CPUID", "CPUID instruction features"),
        ("MSR_READ", "Model-specific register access"),
        ("DMI_TABLE", "DMI/SMBIOS table parsing"),
        ("BIOS_ROM", "BIOS ROM signature detection"),
        ("MEMORY_MAP", "Memory map from bootloader"),
        ("E820_SCAN", "E820 memory ranges"),
    ]
    
    passed = 0
    failed = 0
    
    for i in range(mini_tests):
        test = hw_tests[i % len(hw_tests)]
        name, description = test
        
        rng = LCG(seed=400 + i)
        fault_chance = rng.next()
        
        if fault_chance < 0.12:  # 12% fault rate for hardware
            status = f"{Color.RED}FAIL{Color.RESET}"
            result = "Hardware detection failed"
            failed += 1
        else:
            status = f"{Color.GREEN}PASS{Color.RESET}"
            result = description
            passed += 1
        
        print(f"  [{status}] {Color.BOLD}{name}{Color.RESET}: {result}")
        simulated_delay(8, 40)
    
    print(f"\n  {Color.GREEN}[✓] Hardware detection complete: {passed} passed, {failed} failed{Color.RESET}\n")
    return passed, failed

def run_performance_benchmarks(mini_tests=20):
    """Run performance and stability benchmarks"""
    print(f"{Color.YELLOW}╔═══════════════════════════════════════╗{Color.RESET}")
    print(f"{Color.YELLOW}║    PERFORMANCE BENCHMARKS             ║{Color.RESET}")
    print(f"{Color.YELLOW}║    Running {mini_tests} mini-tests             ║{Color.RESET}")
    print(f"{Color.YELLOW}╚═══════════════════════════════════════╝{Color.RESET}\n")
    
    perf_tests = [
        ("MEMORY_READ", "Memory read bandwidth (MB/s)"),
        ("MEMORY_WRITE", "Memory write bandwidth (MB/s)"),
        ("CACHE_L1", "L1 cache latency test"),
        ("CONTEXT_SWITCH", "Context switch frequency"),
        ("INTERRUPT_LATENCY", "Interrupt response latency"),
        ("CPUID_PERF", "CPUID instruction performance"),
        ("TSC_FREQ", "Timestamp counter frequency"),
        ("TIMER_ACC", "System timer accuracy"),
        ("LOOP_UNROLL", "Loop execution optimization"),
        ("BRANCH_PRED", "Branch prediction accuracy"),
    ]
    
    passed = 0
    failed = 0
    
    for i in range(mini_tests):
        test = perf_tests[i % len(perf_tests)]
        name, description = test
        
        rng = LCG(seed=500 + i)
        fault_chance = rng.next()
        
        if fault_chance < 0.08:  # 8% fault rate for perf
            status = f"{Color.RED}FAIL{Color.RESET}"
            result = f"{random.randint(1000, 5000)} cycles"
            failed += 1
        else:
            status = f"{Color.GREEN}PASS{Color.RESET}"
            result = f"{random.randint(100, 500)} cycles (optimal)"
            passed += 1
        
        print(f"  [{status}] {Color.BOLD}{name}{Color.RESET}: {result}")
        simulated_delay(20, 80)
    
    print(f"\n  {Color.GREEN}[✓] Performance benchmarks complete: {passed} passed, {failed} failed{Color.RESET}\n")
    return passed, failed

def print_summary(comprehensive_data=None):
    """Print diagnostic summary"""
    print(f"{Color.BLUE}╔═══════════════════════════════════════╗{Color.RESET}")
    print(f"{Color.BLUE}║      DIAGNOSTIC SUMMARY               ║{Color.RESET}")
    print(f"{Color.BLUE}╚═══════════════════════════════════════╝{Color.RESET}\n")
    
    cpu_info = "Intel Core i7 (simulated)" if True else "Unknown CPU"
    memory_info = "4096 MB total, 3984 MB available"
    
    print(f"  {Color.CYAN}CPU:{Color.RESET} {cpu_info}")
    print(f"  {Color.CYAN}Memory:{Color.RESET} {memory_info}")
    print(f"  {Color.CYAN}Drivers:{Color.RESET} 8 initialized")
    print(f"  {Color.CYAN}Checkpoints:{Color.RESET} 14 passed")
    
    if comprehensive_data:
        total_passed = sum(x for x, y in comprehensive_data)
        total_failed = sum(y for x, y in comprehensive_data)
        total_tests = total_passed + total_failed
        pass_rate = (total_passed / total_tests * 100) if total_tests > 0 else 0
        
        print(f"  {Color.CYAN}Total Tests:{Color.RESET} {total_tests}")
        print(f"  {Color.CYAN}Passed:{Color.RESET} {total_passed} ({pass_rate:.1f}%)")
        print(f"  {Color.CYAN}Failed:{Color.RESET} {total_failed} ({100-pass_rate:.1f}%)")
        
        if pass_rate >= 95:
            status = f"{Color.GREEN}BOOT DIAGNOSTICS PASSED - SYSTEM READY{Color.RESET}"
        elif pass_rate >= 80:
            status = f"{Color.YELLOW}BOOT DIAGNOSTICS PASSED - WARNINGS PRESENT{Color.RESET}"
        else:
            status = f"{Color.RED}BOOT DIAGNOSTICS FAILED - CRITICAL ISSUES{Color.RESET}"
    else:
        status = f"{Color.GREEN}BOOT DIAGNOSTICS PASSED{Color.RESET}"
    
    print(f"  {Color.CYAN}Status:{Color.RESET} {status}")
    print(f"  {Color.CYAN}Timestamp:{Color.RESET} {time.strftime('%Y-%m-%d %H:%M:%S')}")
    print()

def main():
    force_mode = len(sys.argv) > 1 and sys.argv[1] == "force"
    
    random.seed(42)  # Deterministic randomness for reproducible tests
    
    print(f"{Color.BOLD}{Color.CYAN}")
    print("╔════════════════════════════════════════════╗")
    if force_mode:
        print("║  KROW DIAGNOSTICS v1.0 - EXTENDED MODE   ║")
        print("║  SafeOS Kernel Boot Diagnostic Console   ║")
        print("║  Running 600+ Tests + 200 Mini-tests     ║")
    else:
        print("║  KROW DIAGNOSTICS v1.0 - LIVE DEBUG MODE ║")
        print("║  SafeOS Kernel Boot Diagnostic Console   ║")
    print("╚════════════════════════════════════════════╝")
    print(f"{Color.RESET}")
    
    print(f"  {Color.GREEN}[*]{Color.RESET} System: x86 (i686) architecture")
    print(f"  {Color.GREEN}[*]{Color.RESET} Bootloader: Limine boot protocol")
    print(f"  {Color.GREEN}[*]{Color.RESET} Mode: {'Extended diagnostics' if force_mode else 'Diagnostic simulation'}")
    print(f"  {Color.GREEN}[*]{Color.RESET} Timestamp: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
    
    data = []
    
    if force_mode:
        print(f"{Color.BOLD}EXTENDED TEST SUITE (Part 1/5){Color.RESET}\n")
        simulated_delay(100, 400)
        data.append(run_comprehensive_tests(iterations=50))
        
        print(f"{Color.BOLD}EXTENDED TEST SUITE (Part 2/5){Color.RESET}\n")
        simulated_delay(100, 400)
        data.append(run_network_diagnostics(mini_tests=40))
        
        print(f"{Color.BOLD}EXTENDED TEST SUITE (Part 3/5){Color.RESET}\n")
        simulated_delay(100, 400)
        data.append(run_iso_integrity_tests(mini_tests=20))
        
        print(f"{Color.BOLD}EXTENDED TEST SUITE (Part 4/5){Color.RESET}\n")
        simulated_delay(100, 400)
        data.append(run_hardware_detection_tests(mini_tests=40))
        
        print(f"{Color.BOLD}EXTENDED TEST SUITE (Part 5/5){Color.RESET}\n")
        simulated_delay(100, 400)
        data.append(run_performance_benchmarks(mini_tests=20))
    else:
        simulated_delay(100, 400)
        run_core_checks()
        
        simulated_delay(100, 400)
        run_driver_checks()
        
        simulated_delay(100, 400)
        run_extended_tests()
    
    simulated_delay(50, 200)
    print_summary(comprehensive_data=data if force_mode else None)

if __name__ == "__main__":
    main()
