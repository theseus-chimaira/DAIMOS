# PDP-6 KCORE source-level machine selection; zero resident cost.
#
# This is a list of the source owners in the CURRENT PDP-6 image, not a
# promise that the named files are exclusively hardware-specific.
# A KA10 port replaces the low-level CPU and VM backends; a KI10 port
# additionally implements page maps and 22-bit physical-frame ownership.
# Neither backend is implemented or silently enabled by this fragment.
#
# Keep the existing generated names, object order, fixed-image placement,
# KCORE end marker, and P10FOLD input order intact during preparation.
# A future port gets a separate architecture-specific boot/link manifest;
# it must not reuse the PDP-6 hardware-specific object names blindly.

DAIMOS_MACHINE ?= pdp6
ifneq ($(DAIMOS_MACHINE),pdp6)
$(error DAIMOS_MACHINE=$(DAIMOS_MACHINE) has no accepted KCORE/boot backend)
endif

# CPU-sensitive process, trap, priority interrupt and syscall boundaries.
DAIMOS_CPU_PROCESS_SOURCE = proc/proc_pdp6.s
DAIMOS_CPU_TRAP_SOURCE = proc/mach_user_pdp6.s
DAIMOS_CPU_PI_SOURCE = core/kcore_pi_pdp6.s
DAIMOS_CPU_SYSCALL_SOURCE = proc/syscall_dispatch_pdp6.s

# One-extent PDP-6 MM/RAW swap implementations; do not conflate with a
# generic frame allocator for KI10.  Common policy should not decode RH.
DAIMOS_VM_CONTIG_MM_SOURCE = mm/mm_pdp6.s
DAIMOS_VM_SWAP_SOURCE = proc/proc_swap_pdp6.s
DAIMOS_VM_BACKING_SOURCE = proc/proc_pure_backing_pdp6.s

# Startup, image loading and other target-specific support.
DAIMOS_CPU_EXEC_LOW_SOURCE = proc/exec_lowmem_pdp6.s
DAIMOS_CPU_KINIT_IO_SOURCE = boot/kinit_io_pdp6.s
DAIMOS_CPU_KINIT_LATE_SOURCE = boot/kinit_late_begin_pdp6.s
DAIMOS_CPU_MODULE_RUNTIME_SOURCE = modules/module_runtime_pdp6.s
DAIMOS_CPU_BCACHE_SOURCE = storage/bcache_pdp6.s
DAIMOS_CPU_KFMT_SOURCE = core/kfmt_pdp6.s
