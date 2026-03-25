# KROW INTEGRATION - STEP-BY-STEP INSTRUCTIONS

## 🎯 Goal
Add Krow Diagnostics to your SafeOS kernel boot sequence in 3 steps.

---

## STEP 1: Edit kernel/core/kmain.c (2 minutes)

### Find your main kernel entry point:

Look for a file like `kernel/core/kmain.c` with a function like:
```c
void kmain(void)
```

or 

```c
int main(void)
```

### Add the include at the top:

```c
// Add this with your other includes
#include "krow_diagnostics.h"
```

### Add the function call in the main function:

Find where your kernel initialization happens (after basic console setup):

```c
void kmain(void) {
    
    // Your existing code here...
    // - console_init()
    // - gdt_init()
    // - idt_init()
    // - paging_init()
    // etc.
    
    // ===== ADD THIS SECTION =====
    kprintf("\n");
    kprintf("[*] Running embedded diagnostics...\n");
    
    int krow_result = krow_diagnostics_run();
    
    if (krow_result > 0) {
        kprintf("[WARN] Diagnostics completed with %d fault(s)\n", krow_result);
    } else {
        kprintf("[OK] All diagnostic checks passed\n");
    }
    // ===== END ADDITION =====
    
    // Continue with your OS initialization...
}
```

---

## STEP 2: Build (1 minute)

### Clean previous build:
```bash
cd vsls:/
make clean
```

### Build the ISO:
```bash
make iso
```

**What happens:**
- Makefile finds `kernel/core/krow_diagnostics.c`
- Automatically compiles it with kernel
- Links into main kernel binary
- No special flags needed

### Verify it compiled:

```bash
make krow-check
```

Should output:
```
[*] Checking Krow compilation...
[OK] Krow Diagnostics is in kernel binary
```

---

## STEP 3: Boot and Test (2 minutes)

### Run your ISO in a VM or Qemu:

```bash
qemu-system-i386 -cdrom SafeOS-1.0.iso
```

or your normal boot method

### Watch for Krow output:

You should see:
```
   _
   (o>
\\\///)     KROW DIAGNOSTICS V 1.0
 \_/_)    ENGINEERED BY SAFEOS
  _|_

======================================
Initializing Diagnostic System...
======================================

[ CHECKING CORE SYSTEMS ]
----------------------------------------
[ TEST ] CORE:PAGING... OK
[ TEST ] CORE:VGA_SERIAL_1... OK
...
```

---

## ✅ That's It!

Once you see the Krow diagnostics output, the integration is complete.

---

## 🔍 Troubleshooting

### Issue: "Krow diagnostics not running"

**Check 1:** Verify the function was called
```bash
grep -n "krow_diagnostics_run" kernel/core/kmain.c
```
Should show your added line

**Check 2:** Did you include the header?
```bash
grep -n "krow_diagnostics.h" kernel/core/kmain.c
```
Should show: `#include "krow_diagnostics.h"`

**Check 3:** Try rebuilding
```bash
make clean
make iso
```

### Issue: "Compilation error about krow_diagnostics.h"

**Fix:** Use relative path:
```c
#include "krow_diagnostics.h"  // ✅ Correct
// NOT:
#include <krow_diagnostics.h>   // ✗ Wrong
```

### Issue: "Undefined reference to krow_diagnostics_run"

**This is OK** - If using minimal kernel without krow_diagnostics.c in build:
- Verify file exists: `ls kernel/core/krow_diagnostics.c`
- Clean and rebuild: `make clean && make iso`

---

## 📊 Expected Output

### Success (No Faults)
```
RESULT: READY TO BOOT
Tests: 14 | Faults: 0
```

### With Faults (Expected ~15% of tests)
```
[!] FAULT DETECTED: CORE:CDROM:TST=FAULT
[*] Executing error handler...

╔══════════════════════════════════════════════════════════════╗
║         KROW DIAGNOSTICS ERROR REPORT v1.0                   ║
║         SAFEOS EMBEDDED DIAGNOSTIC SYSTEM                    ║
╚══════════════════════════════════════════════════════════════╝

RESULT: BOOT WITH WARNINGS
```

---

## 🎨 Customization

After integration works, you can customize:

### Adjust Timing
Edit `kernel/core/krow_diagnostics.c`:
```c
#define DELAY_LINE      50    // Faster output (was 100)
#define DELAY_CHECK     75    
#define DELAY_SECTION   150   
```

### Change Fault Rate
In `krow_random_fault()`:
```c
return (krow_rand() % 100) < 30;  // 30% faults (was 15%)
```

### Add New Checks
Add to `krow_diagnostics_run()`:
```c
run_check("MY_SECTION", "My Check", "MY_TEST");
```

---

## 📚 More Information

- **Complete Guide:** `KROW_INTEGRATION.md`
- **Code Examples:** `KROW_KERNEL_INTEGRATION.c`
- **Features Overview:** `KROW_README.md`
- **File Structure:** `KROW_MANIFEST.md`

---

## ⏱️ Total Integration Time: ~5 minutes

✅ Edit kmain.c (2 min)
✅ Build (1 min)  
✅ Test (2 min)
✅ Done!

---

## ✨ Done!

Your SafeOS kernel now includes Krow Diagnostics v1.0!

Boot and observe the professional diagnostic display on startup.

## See Also
- [KROW_EXTENDED_TESTS.md](KROW_EXTENDED_TESTS.md)
- [KROW_DELIVERY_SUMMARY.md](KROW_DELIVERY_SUMMARY.md)
- [KROW_INTEGRATION.md](KROW_INTEGRATION.md)
- [KROW_MANIFEST.md](KROW_MANIFEST.md)
- [KROW_QUICK_START.md](KROW_QUICK_START.md)
- [KROW_README.md](KROW_README.md)
- [KROW_REAL_VS_SIMULATED.md](KROW_REAL_VS_SIMULATED.md)
- [KROW_KERNEL_INTEGRATION.c](KROW_KERNEL_INTEGRATION.c)
- [tools/kernel-krow.py](tools/kernel-krow.py)
- [scripts/S01krow](scripts/S01krow)
