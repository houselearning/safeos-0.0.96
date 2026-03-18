/*
 * KROW DIAGNOSTICS INTEGRATION EXAMPLE FOR kernel/core/kmain.c
 * 
 * This shows how to integrate Krow Diagnostics into your OS boot sequence.
 * Copy the relevant sections and adapt to your kernel structure.
 */

#include "krow_diagnostics.h"

/* ============================================================================
   BEFORE YOUR MAIN KERNEL INITIALIZATION
   ============================================================================ */

/* Add this at the top of kmain.c */
#include "krow_diagnostics.h"

/* ============================================================================
   IN YOUR kmain() OR LINKER ENTRY FUNCTION
   ============================================================================ */

void kmain(void) {
    /* 
     * Your existing early kernel initialization code:
     * - Console setup
     * - GDT/IDT initialization
     * - Paging setup
     * - IRQ handlers
     * - etc.
     */
    
    // ... YOUR EXISTING CODE ...
    
    /* 
     * ===================================================================
     * KROW DIAGNOSTICS - Insert after basic kernel setup
     * ===================================================================
     * 
     * This should be called:
     * - AFTER console/kprintf is available
     * - AFTER basic hardware is initialized
     * - BEFORE main OS initialization
     */
    
    kprintf("\n");
    kprintf("[*] Starting embedded diagnostics...\n");
    kprintf("[*] This may take a few seconds...\n\n");
    
    /* Run diagnostics - returns number of faults detected */
    int diagnostic_faults = krow_diagnostics_run();
    
    /* Check results */
    if (diagnostic_faults == 0) {
        kprintf("\n[OK] System passed all diagnostic checks\n");
        kprintf("[*] Proceeding with normal OS initialization\n\n");
    } else {
        kprintf("\n[WARN] System diagnostics found %d fault(s)\n", 
                diagnostic_faults);
        kprintf("[*] Starting OS with degraded capabilities\n");
        kprintf("[*] See diagnostic output above for details\n\n");
    }
    
    /* 
     * Continue with rest of OS initialization
     * The diagnostic faults won't prevent boot, but indicate issues
     */
    
    // ... REST OF YOUR KERNEL CODE ...
}

/* ============================================================================
   EXAMPLE: ALTERNATIVE - Conditional Diagnostics
   ============================================================================ */

/*
 * If you want conditional diagnostics (e.g., only on debug builds):
 */

#ifdef ENABLE_KROW_DIAGNOSTICS
    kprintf("[*] Debug: Running Krow Diagnostics...\n");
    int fault_count = krow_diagnostics_run();
#else
    kprintf("[*] Diagnostics disabled in this build\n");
    int fault_count = 0;
#endif

if (fault_count > 0) {
    kprintf("[WARN] Detected %d diagnostic faults\n", fault_count);
}

/* ============================================================================
   COMPILATION
   ============================================================================ */

/*
 * To compile with Krow Diagnostics:
 * 
 * 1. Ensure krow_diagnostics.c is in your source list:
 *    - Place in kernel/core/ directory
 *    - Will be picked up by: $(shell find kernel -name '*.c')
 * 
 * 2. Build normally:
 *    $ make
 * 
 * 3. Or specifically:
 *    $ make krow
 *    $ make iso
 * 
 * The Makefile will handle compilation of krow_diagnostics.c
 * automatically as part of the kernel build.
 */

/* ============================================================================
   MINIMAL EXAMPLE FOR VERY EARLY BOOT
   ============================================================================ */

/*
 * If you need to run diagnostics in real mode (x86), 
 * you would need a different approach - running as separate executable
 * after kernel is fully loaded. Krow is designed for protected mode.
 */

void kernel_main_minimal(void) {
    /* Setup basic console */
    console_init();
    
    /* Setup paging */
    paging_init();
    
    /* Setup interrupts */
    interrupts_init();
    
    // NOW run diagnostics
    krow_diagnostics_run();
    
    // Continue with rest of OS
    gui_init();
    text_desktop_init();
}

/* ============================================================================
   LINKING NOTES
   ============================================================================ */

/*
 * Make sure your kernel's linker script includes:
 * - All kernel/core/*.c files
 * - The object files are linked with the main kernel binary
 * 
 * Example from SafeOS Makefile:
 * 
 * SRC_C = $(shell find kernel -name '*.c')
 * OBJ_C = $(SRC_C:.c=.o)
 * 
 * $(KERNEL): $(OBJ)
 *     $(LD) $(LDFLAGS) -o $@ kernel/arch/x86/boot.ao \
 *          $(filter-out kernel/arch/x86/boot.ao,$(OBJ))
 * 
 * This automatically includes krow_diagnostics.o
 */

/* ============================================================================
   DEBUGGING: Add to kmain if you need diagnostics info
   ============================================================================ */

extern DiagnosticState state;  /* Defined in krow_diagnostics.c */

void print_diagnostic_summary(void) {
    if (state.fault_count > 0) {
        kprintf("[DEBUG] Krow Faults Detected:\n");
        for (int i = 0; i < state.test_count; i++) {
            if (state.tests[i].has_fault) {
                kprintf("  - %s\n", state.tests[i].fault_code);
            }
        }
    }
}

/*
 * Call this from your debug shell or boot menu if available
 */
