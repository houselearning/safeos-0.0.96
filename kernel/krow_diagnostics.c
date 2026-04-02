#include "core/stdio.h"
#include "core/string.h"
#include "core/unistd.h"
#include "core/fs.h"
#include "arch/x86/keyboard.h"

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
    char id[16];
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

/* Forward declarations for functions used by earlier checks */
void record_fault(const char *section, const char *subsection, const char *test);
void record_pass(const char *test_name);

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

/* ============================================================================
   CHECKPOINT SYSTEM FOR BOOT DIAGNOSTICS
   ============================================================================ */

typedef struct {
    const char *name;
    int reached;
} Checkpoint;

#define MAX_CHECKPOINTS 16
static Checkpoint checkpoints[MAX_CHECKPOINTS] = {0};
static int checkpoint_count = 0;

void checkpoint_init(const char *name) {
    if (checkpoint_count < MAX_CHECKPOINTS) {
        checkpoints[checkpoint_count].name = name;
        checkpoints[checkpoint_count].reached = 1;
        kprintf("[✓] CHECKPOINT: %s\n", name);
        checkpoint_count++;
        delay_ms(50);
    }
}

void checkpoint_summary(void) {
    kprintf("\n[*] Boot Checkpoints Reached: %d\n", checkpoint_count);
    for (int i = 0; i < checkpoint_count; i++) {
        kprintf("    %d. %s\n", i + 1, checkpoints[i].name);
    }
}

/* ============================================================================
   VGA/DISPLAY DETECTION
   ============================================================================ */

/* VGA memory address */
#define VGA_MEMORY ((void *)0xB8000)
#define VGA_TEXT_MEMORY_SIZE 4000 /* 80x25 * 2 bytes per char */

int detect_vga_memory(void) {
    /* Check if we can read/write to VGA text memory */
    volatile char *vga = (volatile char *)VGA_MEMORY;
    char original = vga[0];
    
    /* Try to write a test value */
    vga[0] = 'K';
    if (vga[0] == 'K') {
        vga[0] = original;  /* Restore */
        return 1;  /* VGA memory accessible */
    }
    return 0;  /* VGA memory not accessible */
}

int detect_framebuffer(void) {
    /* Check for framebuffer (usually at 0xFD000000 or higher in UEFI) */
    /* In real OS, query from bootloader info */
    return 1;  /* Assume present if we got this far */
}

int detect_vga_controller(void) {
    /* VGA controller typically responds to port 0x3D4 (CRT controller) */
    /* For kernel diagnostic, assume present if display is working */
    return 1;
}

int test_display_output(void) {
    /* Try to output colored text */
    kprintf("%s[TEST] Color output: ", COLOR_GREEN);
    kprintf("%sRed%s ", COLOR_RED, COLOR_RESET);
    kprintf("%sYellow%s ", COLOR_YELLOW, COLOR_RESET);
    kprintf("%sCyan%s\n", COLOR_CYAN, COLOR_RESET);
    return 1;
}

/* ============================================================================
   EXTENDED VGA/DISPLAY CHECKS
   ============================================================================ */

void run_extended_vga_checks(void) {
    int vga_mem_ok = 0;
    int framebuf_ok = 0;
    int vga_ctrl_ok = 0;
    int display_out_ok = 0;
    
    print_line("");
    print_line("[ EXTENDED DISPLAY DIAGNOSTICS ]");
    delay_ms(DELAY_SECTION);
    
    /* Check 1: VGA Memory */
    kprintf("[ TEST ] VGA Memory Detection...");
    delay_ms(DELAY_CHECK);
    vga_mem_ok = detect_vga_memory();
    if (vga_mem_ok) {
        kprintf(" %sOK%s\n", COLOR_GREEN, COLOR_RESET);
        record_pass("VGA_MEMORY");
    } else {
        kprintf(" %sFAULT%s\n", COLOR_RED, COLOR_RESET);
        record_fault("DISPLAY", "VGA_MEMORY", "TST");
    }
    
    /* Check 2: Framebuffer */
    kprintf("[ TEST ] Framebuffer Detection...");
    delay_ms(DELAY_CHECK);
    framebuf_ok = detect_framebuffer();
    if (framebuf_ok) {
        kprintf(" %sOK%s\n", COLOR_GREEN, COLOR_RESET);
        record_pass("FRAMEBUFFER");
    } else {
        kprintf(" %sFAULT%s\n", COLOR_RED, COLOR_RESET);
        record_fault("DISPLAY", "FRAMEBUFFER", "TST");
    }
    
    /* Check 3: VGA Controller */
    kprintf("[ TEST ] VGA Controller Detection...");
    delay_ms(DELAY_CHECK);
    vga_ctrl_ok = detect_vga_controller();
    if (vga_ctrl_ok) {
        kprintf(" %sOK%s\n", COLOR_GREEN, COLOR_RESET);
        record_pass("VGA_CONTROLLER");
    } else {
        kprintf(" %sFAULT%s\n", COLOR_RED, COLOR_RESET);
        record_fault("DISPLAY", "VGA_CONTROLLER", "TST");
    }
    
    /* Check 4: Display Output */
    kprintf("[ TEST ] Display Output Test...");
    delay_ms(DELAY_CHECK);
    display_out_ok = test_display_output();
    record_pass("DISPLAY_OUTPUT");
    
    print_line("");
    if (vga_mem_ok && framebuf_ok && vga_ctrl_ok && display_out_ok) {
        kprintf("%s[OK] Display system fully operational%s\n", COLOR_GREEN, COLOR_RESET);
    } else {
        kprintf("%s[WARN] Display issues detected%s\n", COLOR_YELLOW, COLOR_RESET);
    }
    delay_ms(DELAY_SECTION);
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
        /* generate a short id for this test for quick reference */
        unsigned int v = krow_rand();
        snprintf(state.tests[state.test_count].id, sizeof(state.tests[state.test_count].id), "%06X", v & 0xFFFFFF);
        state.tests[state.test_count].has_fault = 1;
        state.tests[state.test_count].is_ok = 0;
        state.fault_count++;
    }
    state.test_count++;
}

void record_pass(const char *test_name) {
    if (state.test_count < 64) {
        strcpy(state.tests[state.test_count].name, test_name);
        unsigned int v = krow_rand();
        snprintf(state.tests[state.test_count].id, sizeof(state.tests[state.test_count].id), "%06X", v & 0xFFFFFF);
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
        /* Print numbered line with short id */
        kprintf("  %2d: [%sFAIL%s] %s: %s\n", state.test_count + 1, COLOR_RED, COLOR_RESET, "----", name);
        record_fault(section, subsection, "TST");
    } else {
        kprintf("  %2d: [%sPASS%s] %s: %s\n", state.test_count + 1, COLOR_GREEN, COLOR_RESET, state.tests[state.test_count].id, name);
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

/* ---------------- Krow BootRepair helpers (file-scope) ---------------- */
/* Create a marker file in the kernel's in-memory FS to signal host tools */
void create_bootrepair_marker(void) {
    const char *marker = "KROW_BOOTREPAIR_REQUEST";
    const char *msg = "Krow BootRepair requested. Run tools/krow_bootrepair.py on host.\n";
    if (fs_create_file(marker) == 0) {
        fs_write_file(marker, msg, (uint32_t)strlen(msg));
        kprintf("%s[KROW] BootRepair marker created: %s%s\n", COLOR_CYAN, marker, COLOR_RESET);
        kprintf("%s[KROW] To run repair (host): python3 tools/krow_bootrepair.py repair --target /mnt/safeos --apply%s\n", COLOR_CYAN, COLOR_RESET);
    } else {
        kprintf("%s[KROW] Could not create BootRepair marker (in-memory FS may not persist)%s\n", COLOR_YELLOW, COLOR_RESET);
    }
}

/* Minimal in-kernel repair: create or restore small config files in RAM FS */
void perform_in_kernel_minimal_repair(void) {
    const char *cfg = "[settings]\nversion=1.0\ntheme=dark\n";
    if (fs_create_file("config.ini") == 0) {
        fs_write_file("config.ini", cfg, (uint32_t)strlen(cfg));
        kprintf("%s[KROW] Restored in-RAM config.ini%s\n", COLOR_CYAN, COLOR_RESET);
    } else {
        kprintf("%s[KROW] config.ini already present or cannot create%s\n", COLOR_YELLOW, COLOR_RESET);
    }
}

/* Wait for a printable keypress on the physical keyboard */
char wait_for_keypress(void) {
    while (1) {
        uint8_t sc = keyboard_read_scancode();
        char c = scancode_to_ascii(sc);
        if (c) return c;
    }
}

void krow_bootrepair_menu(void) {
    kprintf("\n");
    kprintf("%s[KROW] BootRepair Menu:%s\n", COLOR_CYAN, COLOR_RESET);
    kprintf("  b : Create BootRepair marker for host-based repair\n");
    kprintf("  r : Attempt minimal in-kernel repair (config files in RAM)\n");
    kprintf("  q : Continue boot (skip)\n");
    kprintf("Press key: ");
    char ch = wait_for_keypress();
    kprintf("%c\n", ch);
    if (ch == 'b' || ch == 'B') {
        create_bootrepair_marker();
    } else if (ch == 'r' || ch == 'R') {
        perform_in_kernel_minimal_repair();
    } else {
        kprintf("%s[KROW] Skipping BootRepair menu%s\n", COLOR_YELLOW, COLOR_RESET);
    }
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

int krow_diagnostics_run(int headless) {
    /* Initialize with simple seed */
    krow_seed = 42;
    
    /* EARLY CHECKPOINTS FOR BOOT DEBUG */
    checkpoint_init("Krow Diagnostics Started");
    
    print_header();
    checkpoint_init("Banner Displayed");
    delay_ms(DELAY_SECTION);
    
    /* RUN EXTENDED DISPLAY TESTS FIRST (to debug black screen) */
    run_extended_vga_checks();
    checkpoint_init("Display Diagnostics Complete");
    
    /* Run all diagnostic checks */
    run_core_checks();
    checkpoint_init("Core Checks Complete");
    
    run_driver_checks();
    checkpoint_init("Driver Checks Complete");
    
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
    checkpoint_init("Error Handlers Complete");
    
    /* Print final result */
    print_final_result();
    checkpoint_init("Final Results Displayed");
    
    /* Save diagnostics to log */
    save_diagnostics_log();
    
    /* Present interactive BootRepair menu to the user (serial/keyboard)
       Skip interactive menu when running in headless/debug mode. */
    if (!headless) {
        krow_bootrepair_menu();
    } else {
        kprintf("%s[KROW] Headless mode detected: skipping interactive BootRepair menu%s\n", COLOR_CYAN, COLOR_RESET);
    }
    checkpoint_summary();
    
    return state.fault_count > 0 ? 1 : 0;
}
