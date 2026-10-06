/**
 * @file wcnsls_io.s
 * @brief Resident PDP-6 Spacewar console and color-scope primitives.
 *
 * Device 0420 needs no interrupt handler or BSS. KINIT owns the disposable
 * single-operation probe/banner primitives; this MRES retains only batched raw
 * userspace word I/O. MonitorFS counts completed read/write requests.
 */
        .globl mfsdev_io_in
        .globl mfsdev_io_out
        .text
        .globl wcnsls_read_words
        .globl wcnsls_write_words

/**
 * @brief Read raw device-0420 DATAI samples in one userspace word-I/O call.
 * @param AC1 Destination word buffer.
 * @param AC2 Number of samples requested.
 * @return AC1 = number of words read, or 0 for an empty request.
 *
 * Privilege is enforced when /DEV/WCNSLS is opened.  The descriptor is then
 * the capability, so the realtime path performs no repeated credential check.
 */
wcnsls_read_words:
        jumpe 2,wcnsls_words_zero
        datai 0420,3
        movem 3,(1)
        aos mfsdev_io_in+012
        movei 1,1
        popj 017,

/**
 * @brief Execute tagged raw WCNSLS operations from one userspace batch.
 * @param AC1 Source word buffer.
 * @param AC2 Number of operations.
 * @return AC1 = number of operations consumed, or 0 for an empty request.
 *
 * Bit 35 selects CONO; otherwise the remaining 35 bits are sent unchanged by
 * DATAO. CONO naturally consumes the word's 18-bit effective-address half.
 * The kernel assigns no graphics meaning: userspace owns refresh and pictures.
 */
wcnsls_write_words:
        jumpe 2,wcnsls_words_zero
        move 3,1
        move 4,2
wcnsls_write_words_loop:
        move 5,(3)
        tlne 5,400000
        jrst wcnsls_write_words_cono
        datao 0420,5
        jrst wcnsls_write_words_next
wcnsls_write_words_cono:
        cono 0420,0(5)
wcnsls_write_words_next:
        addi 3,1
        sojg 4,wcnsls_write_words_loop
        aos mfsdev_io_out+012
        move 1,2
        popj 017,

wcnsls_words_zero:
        setz 1,
        popj 017,
