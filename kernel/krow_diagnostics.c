#include "core/stdio.h"
#include "core/string.h"
#include "core/unistd.h"

/* ============================================================================
   KROW DIAGNOSTICS V 1.0
   Minimal embedded diagnostic console system
   Engineered by SafeOS
   ============================================================================ */

/* ANSI Color Codes */
#define COLOR_RESET     "\x1b[0m"
#define COLOR_GREEN     "\x1b[32m"
#define COLOR_RED       "\x1b[31m"
#define COLOR_YELLOW    "\x1b[33m"
#define COLOR_CYAN      "\x1b[36m"
#define COLOR_BOLD      "\x1b[1m"

/* Delays (in milliseconds) */
#define DELAY_LINE      100
#define DELAY_CHECK     150
#define DELAY_SECTION   300

/* Test result structure */
typedef struct {
    char name[32];
    int is_ok;
    int has_fault;
    char fault_code[128];
} TestResult;

/* Global fault tracking */
typedef struct {
    TestResult tests[64];
    int test_count;
    int fault_count;
    int progress;
} DiagnosticState;

DiagnosticState state = {0};

/* Simple random number generator (no stdlib.h available) */
static unsigned int krow_seed = 1;

unsigned int krow_rand(void) {
    krow_seed = (krow_seed * 1103515245 + 12345) & 0x7fffffff;
    return krow_seed;
}

int krow_random_fault(void) {
    return (krow_rand() % 100) < 15; /* 15% fault chance */
}

/* ============================================================================
   UTILITY FUNCTIONS
   ============================================================================ */

void delay_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}

void print_line(const char *line) {
    kprintf("%s\n", line);
    delay_ms(DELAY_LINE);
}

int krow_strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}

void record_fault(const char *section, const char *subsection, const char *test) {
    if (state.fault_count < 64) {
        snprintf(state.tests[state.test_count].fault_code, 128,
                "%s:%s:%s=FAULT", section, subsection, test);
        state.tests[state.test_count].has_fault = 1;
        state.tests[state.test_count].is_ok = 0;
        state.fault_count++;
    }
    state.test_count++;
}

void record_pass(const char *test_name) {
    if (state.test_count < 64) {
        strcpy(state.tests[state.test_count].name, test_name);
        state.tests[state.test_count].is_ok = 1;
        state.tests[state.test_count].has_fault = 0;
    }
    state.test_count++;
}

void execute_error_handler(const char *error_code) {
    char cmd[256];
    snprintf(cmd, 256, "[!] Error: %s", error_code);
    kprintf("\n");
    print_line("[*] Error handler would execute...");
    kprintf("%s\n", cmd);
    delay_ms(DELAY_SECTION);
    /* Note: In userspace environment, would call:
     * system("python3 tools/kernel-krow.py \"%s\"", error_code);
     */
}

/* ============================================================================
   DIAGNOSTIC FUNCTIONS
   ============================================================================ */

void print_header(void) {
    print_line("");
    print_line("   _");
    print_line("   (o>");
    kprintf("%s\\\\%s//)     KROW DIAGNOSTICS V 1.0\n", COLOR_BOLD, COLOR_RESET);
    delay_ms(DELAY_LINE);
    print_line(" \\_/_)    ENGINEERED BY SAFEOS");
    print_line("  _|_");
    print_line("");
}

void run_check(const char *section, const char *name, const char *subsection) {
    int fault = krow_random_fault();
    if (fault) {
        kprintf("%s%-15s %s%-20s [%sFAULT%s]\n", 
               COLOR_CYAN, section, COLOR_RESET, name, COLOR_RED, COLOR_RESET);
        record_fault(section, subsection, "TST");
    } else {
        kprintf("%s%-15s %s%-20s [%sOK%s]\n", 
               COLOR_CYAN, section, COLOR_RESET, name, COLOR_GREEN, COLOR_RESET);
        record_pass(name);
    }
    delay_ms(DELAY_CHECK);
}

void run_driver_checks(void) {
    print_line("");
    print_line("DRIVERS");
    delay_ms(DELAY_SECTION);
    
    run_check("DRV KBD", "KEYBOARD", "DRV_KBD");
    run_check("DRV MOU", "MOUSE", "DRV_MOU");
    run_check("DRV WBC", "WEB-CTRL", "DRV_WBC");
    run_check("DRV SOU", "SOUND", "DRV_SOU");
    run_check("DRV 3DG", "3D GPU", "DRV_3DG");
    run_check("DRV WEB", "NETWORK", "DRV_WEB");
    run_check("DRV ENH", "ENHANCED", "DRV_ENH");
    run_check("DRV JCK", "JACK AUDIO", "DRV_JCK");
}

void run_core_checks(void) {
    print_line("");
    print_line("CORE SYSTEM");
    delay_ms(DELAY_SECTION);
    
    run_check("PAGING", "Memory Paging", "PAGING");
    run_check("VGA SERIAL 1", "Video Serial A", "VGA_S1");
    run_check("VGA SERIAL 2", "Video Serial B", "VGA_S2");
    run_check("CD ROM", "CDROM Controller", "CDROM");
    run_check("MEMORY", "System Memory", "MEMORY");
    run_check("DRIVERS", "Driver Subsystem", "DRIVERS");
}

void save_diagnostics_log(void) {
    /* Save diagnostic results to KROW_OUTPUT.log */
    char buffer[512];
    int len = 0;
    
    /* In a kernel environment without file I/O, we just note where it would go */
    kprintf("\n[*] Diagnostic Summary:\n");
    kprintf("    Total Tests: %d\n", state.test_count);
    kprintf("    Faults Found: %d\n", state.fault_count);
    
    if (state.fault_count > 0) {
        kprintf("    Faulted Components:\n");
        for (int i = 0; i < state.test_count; i++) {
            if (state.tests[i].has_fault) {
                kprintf("      - %s\n", state.tests[i].fault_code);
            }
        }
    }
    
    kprintf("\n[*] Log file would be written to: KROW_OUTPUT.log\n");
}

void print_final_result(void) {
    print_line("");
    print_line("=====================================");
    
    if (state.fault_count == 0) {
        kprintf("[RESULT] READY TO BOOT\n");
    } else {
        kprintf("[RESULT] BOOT WITH WARNINGS\n");
    }
    
    if (state.test_count > 0) {
        kprintf("Tests: %d | Faults: %d\n", 
               state.test_count, state.fault_count);
    }
    
    delay_ms(DELAY_SECTION);
    print_line("=====================================");
    print_line("");
    print_line("BOOT SESSION...");
    print_line("");
}

/* ============================================================================
   MAIN DIAGNOSTIC ROUTINE
   ============================================================================ */

int krow_diagnostics_run(void) {
    /* Initialize with simple seed */
    krow_seed = 42;
    
    print_header();
    delay_ms(DELAY_SECTION);
    
    /* Run all diagnostic checks */
    run_core_checks();
    run_driver_checks();
    
    /* Execute error handlers for any faults */
    if (state.fault_count > 0) {
        print_line("");
        print_line("[!] FAULTS DETECTED - EXECUTING ERROR HANDLERS");
        for (int i = 0; i < state.test_count; i++) {
            if (state.tests[i].has_fault) {
                execute_error_handler(state.tests[i].fault_code);
                delay_ms(DELAY_SECTION);
            }
        }
    }
    
    /* Print final result */
    print_final_result();
    
    /* Save diagnostics to log */
    save_diagnostics_log();
    
    return state.fault_count > 0 ? 1 : 0;
}
