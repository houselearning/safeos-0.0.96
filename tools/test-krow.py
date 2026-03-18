#!/usr/bin/env python3
"""
KROW DIAGNOSTICS TEST SUITE
Automated testing and validation for Krow Diagnostics v1.0

Usage:
    python3 tools/test-krow.py [options]

Options:
    --run-tests         Run all tests
    --check-syntax      Validate C and Python syntax
    --benchmark         Run performance benchmark
    --stress-test N     Stress test with N runs
    --help              Show this help
"""

import os
import sys
import subprocess
import tempfile
from pathlib import Path

# ANSI Colors
GREEN = '\033[32m'
RED = '\033[31m'
YELLOW = '\033[33m'
RESET = '\033[0m'

class KrowTestSuite:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.project_root = Path(__file__).parent.parent
        self.krow_binary = self.project_root / "kernel" / "krow_diagnostics"
        self.krow_script = self.project_root / "tools" / "kernel-krow.py"
    
    def test(self, name, condition, error_msg=""):
        """Record test result"""
        if condition:
            print(f"{GREEN}[PASS]{RESET} {name}")
            self.passed += 1
        else:
            print(f"{RED}[FAIL]{RESET} {name}")
            if error_msg:
                print(f"  Error: {error_msg}")
            self.failed += 1
    
    def run_command(self, cmd):
        """Run shell command and return result"""
        try:
            result = subprocess.run(cmd, shell=True, capture_output=True, 
                                  text=True, timeout=10)
            return result.returncode, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return -1, "", "Timeout"
        except Exception as e:
            return -1, "", str(e)
    
    # ========================================================================
    # TEST SUITE
    # ========================================================================
    
    def test_c_syntax(self):
        """Test C source syntax"""
        print("\n[TEST SUITE] C Syntax Validation")
        
        c_file = self.project_root / "kernel" / "krow_diagnostics.c"
        self.test("C source exists", c_file.exists(), 
                 f"File not found: {c_file}")
        
        # Check syntax with gcc
        ret, _, stderr = self.run_command(f"gcc -fsyntax-only {c_file}")
        self.test("C syntax valid", ret == 0, stderr)
        
        # Check for required includes
        content = c_file.read_text()
        self.test("Has stdio.h", "#include <stdio.h>" in content)
        self.test("Has stdlib.h", "#include <stdlib.h>" in content)
        self.test("Has unistd.h", "#include <unistd.h>" in content)
        
        # Check for required functions
        self.test("Has run_check function", "void run_check" in content)
        self.test("Has run_driver_checks function", "void run_driver_checks" in content)
        self.test("Has run_multiboot_tests function", "void run_multiboot_tests" in content)
        self.test("Has run_display_tests function", "void run_display_tests" in content)
    
    def test_python_syntax(self):
        """Test Python script syntax"""
        print("\n[TEST SUITE] Python Syntax Validation")
        
        self.test("Python script exists", self.krow_script.exists(),
                 f"File not found: {self.krow_script}")
        
        # Check Python syntax
        ret, _, stderr = self.run_command(f"python3 -m py_compile {self.krow_script}")
        self.test("Python syntax valid", ret == 0, stderr)
        
        # Check for required functions
        content = self.krow_script.read_text()
        self.test("Has parse_error_code function", "def parse_error_code" in content)
        self.test("Has generate_error_report function", 
                 "def generate_error_report" in content)
        self.test("Has ERROR_DATABASE", "ERROR_DATABASE = {" in content)
    
    def test_compilation(self):
        """Test building the C program"""
        print("\n[TEST SUITE] Compilation")
        
        # Clean previous build
        if self.krow_binary.exists():
            self.krow_binary.unlink()
        
        # Compile
        c_file = self.project_root / "kernel" / "krow_diagnostics.c"
        ret, _, stderr = self.run_command(
            f"gcc -Wall -Wextra -std=c99 -o {self.krow_binary} {c_file}")
        
        self.test("Compilation successful", ret == 0, stderr)
        self.test("Binary created", self.krow_binary.exists())
        
        if self.krow_binary.exists():
            # Check binary size
            size = self.krow_binary.stat().st_size
            self.test("Binary size reasonable", size < 100000,
                     f"Size: {size} bytes (expected <100KB)")
    
    def test_execution(self):
        """Test running the diagnostic program"""
        print("\n[TEST SUITE] Execution")
        
        if not self.krow_binary.exists():
            print(f"{RED}[SKIP]{RESET} Binary not built")
            return
        
        # Run without arguments
        ret, stdout, stderr = self.run_command(str(self.krow_binary))
        self.test("Program runs without arguments", ret in [0, 1],
                 f"Exit code: {ret}")
        self.test("Output contains ASCII art", "Krow" in stdout or "KROW" in stdout,
                 "ASCII art not found in output")
        
        # Run with seed
        ret, _, _ = self.run_command(f"{self.krow_binary} 12345")
        self.test("Program accepts seed argument", ret in [0, 1])
    
    def test_error_handler(self):
        """Test Python error handler"""
        print("\n[TEST SUITE] Error Handler")
        
        if not self.krow_script.exists():
            print(f"{RED}[SKIP]{RESET} Python script not found")
            return
        
        # Test with valid error code
        ret, stdout, stderr = self.run_command(
            f"python3 {self.krow_script} 'BATTERY:PERCENT:TST=FAULT'")
        
        self.test("Error handler runs", ret == 0, stderr)
        self.test("Outputs error report", "[ERROR REPORT]" in stdout,
                 "No error report in output")
        self.test("Includes analysis", "Analysis:" in stdout or "analysis" in stdout.lower())
        self.test("Includes fixes", "Suggested Fix" in stdout or "fix" in stdout.lower())
        self.test("Includes severity", "Severity:" in stdout or "severity" in stdout.lower())
        
        # Test with invalid error code
        ret, _, _ = self.run_command(
            f"python3 {self.krow_script} 'INVALID'")
        self.test("Rejects invalid error code", ret == 1)
    
    def test_patterns(self):
        """Test regex patterns"""
        print("\n[TEST SUITE] Pattern Validation")
        
        import re
        
        # Test error code pattern
        pattern = r'([A-Z_]+):([A-Z_]+):([A-Z]+)=([A-Z]+)'
        
        valid_codes = [
            "BATTERY:PERCENT:TST=FAULT",
            "DISPLAY:APP:TST=FAULT",
            "PAGING:MMU:TST=OK",
            "DRV_KBD:INPUT:TST=FAULT"
        ]
        
        for code in valid_codes:
            match = re.match(pattern, code)
            self.test(f"Pattern matches '{code}'", match is not None)
        
        invalid_codes = [
            "INVALID",
            "SECTION:SUBSECTION",
            "SECTION:SUBSECTION:TEST",
        ]
        
        for code in invalid_codes:
            match = re.match(pattern, code)
            self.test(f"Pattern rejects '{code}'", match is None)
    
    def benchmark(self):
        """Run performance benchmark"""
        print("\n[BENCHMARK] Performance Test")
        
        if not self.krow_binary.exists():
            print(f"{RED}[SKIP]{RESET} Binary not built")
            return
        
        import time
        
        start = time.time()
        self.run_command(str(self.krow_binary))
        elapsed = time.time() - start
        
        print(f"  Execution time: {elapsed:.2f} seconds")
        self.test("Execution time reasonable", elapsed < 30,
                 f"Took {elapsed:.2f}s (expected <30s)")
    
    def stress_test(self, iterations):
        """Stress test with multiple runs"""
        print(f"\n[STRESS TEST] Running {iterations} iterations")
        
        if not self.krow_binary.exists():
            print(f"{RED}[SKIP]{RESET} Binary not built")
            return
        
        fault_count = 0
        success_count = 0
        
        for i in range(iterations):
            ret, _, _ = self.run_command(f"{self.krow_binary} {i}")
            if ret == 0:
                success_count += 1
            elif ret == 1:
                fault_count += 1
            
            if (i + 1) % 10 == 0:
                print(f"  Completed: {i + 1}/{iterations}")
        
        fault_rate = (fault_count / iterations) * 100 if iterations > 0 else 0
        print(f"  Success: {success_count}/{iterations}")
        print(f"  Faults: {fault_count}/{iterations}")
        print(f"  Fault rate: {fault_rate:.1f}%")
        
        self.test("Fault rate reasonable", 5 < fault_rate < 25,
                 f"Fault rate {fault_rate:.1f}% (expected 10-20%)")
    
    # ========================================================================
    # REPORTING
    # ========================================================================
    
    def print_summary(self):
        """Print test summary"""
        total = self.passed + self.failed
        
        print("\n" + "="*70)
        print(f"TEST SUMMARY: {self.passed} passed, {self.failed} failed ({total} total)")
        
        if self.failed == 0:
            print(f"\n{GREEN}[SUCCESS] All tests passed!{RESET}")
        else:
            print(f"\n{RED}[FAILURE] Some tests failed{RESET}")
        
        print("="*70)
        
        return self.failed == 0
    
    def run_all_tests(self):
        """Run complete test suite"""
        print("\n" + "="*70)
        print("KROW DIAGNOSTICS TEST SUITE v1.0")
        print("="*70)
        
        self.test_c_syntax()
        self.test_python_syntax()
        self.test_compilation()
        self.test_execution()
        self.test_error_handler()
        self.test_patterns()
        self.benchmark()
        
        return self.print_summary()

# ============================================================================
# MAIN
# ============================================================================

def main():
    suite = KrowTestSuite()
    
    if len(sys.argv) < 2:
        all_pass = suite.run_all_tests()
        return 0 if all_pass else 1
    
    cmd = sys.argv[1]
    
    if cmd == "--help":
        print(__doc__)
        return 0
    elif cmd == "--run-tests":
        all_pass = suite.run_all_tests()
        return 0 if all_pass else 1
    elif cmd == "--check-syntax":
        suite.test_c_syntax()
        suite.test_python_syntax()
        return suite.print_summary()
    elif cmd == "--benchmark":
        suite.test_compilation()
        suite.benchmark()
        return 0
    elif cmd == "--stress-test":
        iterations = int(sys.argv[2]) if len(sys.argv) > 2 else 50
        suite.test_compilation()
        suite.stress_test(iterations)
        return 0
    else:
        print(f"Unknown command: {cmd}")
        print("Use --help for usage information")
        return 1

if __name__ == "__main__":
    sys.exit(main())
