# PDP-6 and KA10 contiguous-core source ownership. The common sources
# retain the original object order and the PDP-6 instruction subset.
# Only native processor differences are selected at assembly time.
DAIMOS_MACHINE ?= pdp6
ifneq ($(filter $(DAIMOS_MACHINE),pdp6 ka10),$(DAIMOS_MACHINE))
$(error DAIMOS_MACHINE=$(DAIMOS_MACHINE) has no KCORE/boot backend)
endif
ifeq ($(DAIMOS_MACHINE),ka10)
DAIMOS_CPU_ASM_PREFIX = .set DAIMOS_KA10,1
DAIMOS_CPU_CFLAGS = -DDAIMOS_KA10=1
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
# v1609: machine-private, compact terminal PACKED_PROCESS MM owner scan.
# The common proc_swap_detach policy does not interpret an APR base.
DAIMOS_VM_PURE_DETACH_SOURCE = proc/proc_pure_packed_detach_v1609.s
# v1611: exclusively PDP-6 contiguous-core PURE service policy.
# A later CPU backend replaces these source owners at build time.
DAIMOS_VM_PURE_SERVICE_SOURCE = proc/proc_pure_service_pdp6_v1612.c
DAIMOS_VM_PURE_REBASE_SOURCE = mm/mm_pure_rebase_v1381.s
DAIMOS_VM_PURE_EXPAND_SOURCE = mm/mm_pure_expand_v1384.s
DAIMOS_VM_PURE_IMAGE_SOURCE = proc/exec_image_common_v1386.s

# Startup, image loading and other target-specific support.
DAIMOS_CPU_EXEC_LOW_SOURCE = proc/exec_lowmem_pdp6.s
DAIMOS_CPU_KINIT_IO_SOURCE = boot/kinit_io_pdp6.s
DAIMOS_CPU_KINIT_LATE_SOURCE = boot/kinit_late_begin_pdp6.s
DAIMOS_CPU_MODULE_RUNTIME_SOURCE = modules/module_runtime_pdp6.s
DAIMOS_CPU_BCACHE_SOURCE = storage/bcache_pdp6.s
DAIMOS_CPU_KFMT_SOURCE = core/kfmt_pdp6.s
