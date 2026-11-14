; v1557: physically unique mounted-DTFS unit ownership for DTVOL2.
;
; The central DTFS write veneer has only a physical unit, not a vnode.
; Before future durable writable V2 is enabled, it must be impossible
; to associate one physical unit with both a V2 and another DTFS mount.
; Perform this test under the existing serialized provider lease and
; immediately before the VFS publication of a physically checked mount.
;
; AC1=physical DTC unit (0..7), AC2=1 for newly admitted DTVOL2,
; AC2=0 for legacy V1/unstamped native. Returns AC1=0 when allowed,
; -1 when any published DTFS mount on the same unit conflicts.
; AC3..AC7 scratch. No MM allocation and no new fixed KCORE state.
;
; V2 conflicts with ANY other DTFS mount of the same physical unit.
; V1 conflicts with an existing V2 mount of that unit, but V1/V1
; still permits the old multiple-mount behavior. Retired slots do not
; retain ownership even if their media slab contains a stale V2 tag.

        .text
        .globl dtfs_v2_mount_exclusive_v1557
        .globl vfs_mount_root_base
        .globl dtfs_media_base
        .globl vfs_mount_limit

dtfs_v2_mount_exclusive_v1557:
        jumpl   1,dtfs_v2_mount_conflict_v1557
        caile   1,7
        jrst    dtfs_v2_mount_conflict_v1557
        move    3,vfs_mount_root_base
        jumpe   3,dtfs_v2_mount_conflict_v1557
        move    4,dtfs_media_base
        jumpe   4,dtfs_v2_mount_conflict_v1557
        move    5,vfs_mount_limit
        jumple  5,dtfs_v2_mount_conflict_v1557
dtfs_v2_mount_next_v1557:
        move    6,(3)
        lsh     6,-036                 ; published vnode provider
        caie    6,5                     ; DTFS only
        jrst    dtfs_v2_mount_skip_v1557
        move    6,(4)
        move    7,6
        andi    7,7                     ; physical Type-551 unit
        came    7,1
        jrst    dtfs_v2_mount_skip_v1557
        jumpn   2,dtfs_v2_mount_conflict_v1557
        trne    6,0100000              ; existing DTVOL2
        jrst    dtfs_v2_mount_conflict_v1557
dtfs_v2_mount_skip_v1557:
        aoj     3,
        aoj     4,
        sojg    5,dtfs_v2_mount_next_v1557
        setz    1,
        popj    17,
dtfs_v2_mount_conflict_v1557:
        seto    1,
        popj    17,
