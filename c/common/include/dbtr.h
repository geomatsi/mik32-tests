#ifndef COMMON_DBTR_H
#define COMMON_DBTR_H

/* tinfo - trigger info */

#define TINFO_INFO(c)                  ((c) & 0xffUL)

enum tinfo_info_t {
	TINFO_INFO_NOEXIST  = 0x1UL,
	TINFO_INFO_MCONTROL = 0x4UL,
	TINFO_INFO_ICOUNT   = 0x8UL,
};

#define TINFO_DBTR_NOEXIST(c)          (TINFO_INFO(c) == TINFO_INFO_NOEXIST)
#define TINFO_DBTR_MCONTROL(c)         (TINFO_INFO(c) == TINFO_INFO_MCONTROL)
#define TINFO_DBTR_ICOUNT(c)           (TINFO_INFO(c) == TINFO_INFO_ICOUNT)

/* tdata1 - trigger data1 */

#define TDATA1_DMODE                   (0x1UL << 27)

#define TDATA1_TYPE_S                  (28UL)
#define TDATA1_TYPE_M                  (0xfUL)
#define TDATA1_TYPE_W(c)               (((c) & TDATA1_TYPE_M) << TDATA1_TYPE_S)
#define TDATA1_TYPE_R(c)               (((c) >> TDATA1_TYPE_S) & TDATA1_TYPE_M)

enum tdata1_type_t {
	TDATA1_TYPE_DBTR_NOEXIST  = 0,
	TDATA1_TYPE_DBTR_MCONTROL = 2,
	TDATA1_TYPE_DBTR_ICOUNT   = 3,
	TDATA1_TYPE_DBTR_SKIP     = 15,
};

/* tdata1 - match control */

#define TDATA1_MCONTROL_LOAD           (0x1UL << 0)
#define TDATA1_MCONTROL_STORE          (0x1UL << 1)
#define TDATA1_MCONTROL_EXEC           (0x1UL << 2)
#define TDATA1_MCONTROL_MODE_M         (0x1UL << 6)
#define TDATA1_MCONTROL_CHAIN          (0x1UL << 11)
#define TDATA1_MCONTROL_HIT            (0x1UL << 20)

#define TDATA1_MCONTROL_MATCH_S        (7UL)
#define TDATA1_MCONTROL_MATCH_M        (0xfUL)
#define TDATA1_MCONTROL_MATCH_W(c)     (((c) & TDATA1_MCONTROL_MATCH_M) << TDATA1_MCONTROL_MATCH_S)
#define TDATA1_MCONTROL_MATCH_R(c)     (((c) >> TDATA1_MCONTROL_MATCH_S) & TDATA1_MCONTROL_MATCH_M)

enum tdata1_mcontrol_match_t {
	TDATA1_MCONTROL_MATCH_EQUAL = 0,
};

#define TDATA1_MCONTROL_ACTION_S       (12UL)
#define TDATA1_MCONTROL_ACTION_M       (0xfUL)
#define TDATA1_MCONTROL_ACTION_W(c)    (((c) & TDATA1_MCONTROL_ACTION_M) << TDATA1_MCONTROL_ACTION_S)
#define TDATA1_MCONTROL_ACTION_R(c)    (((c) >> TDATA1_MCONTROL_ACTION_S) & TDATA1_MCONTROL_ACTION_M)

enum tdata1_mcontrol_action_t {
	TDATA1_MCONTROL_ACTION_EBREAK = 0,
	TDATA1_MCONTROL_ACTION_DMODE  = 1,
};

/* tdata1 - instruction count */

#define TDATA1_ICOUNT_MODE_M         (0x1UL << 9)
#define TDATA1_ICOUNT_HIT            (0x1UL << 24)

#define TDATA1_ICOUNT_ACTION_S       (0UL)
#define TDATA1_ICOUNT_ACTION_M       (0x3fUL)
#define TDATA1_ICOUNT_ACTION_W(c)    (((c) & TDATA1_ICOUNT_ACTION_M) << TDATA1_ICOUNT_ACTION_S)
#define TDATA1_ICOUNT_ACTION_R(c)    (((c) >> TDATA1_ICOUNT_ACTION_S) & TDATA1_ICOUNT_ACTION_M)

enum tdata1_icount_action_t {
	TDATA1_ICOUNT_ACTION_EBREAK = 0,
	TDATA1_ICOUNT_ACTION_DMODE  = 1,
};

#define TDATA1_ICOUNT_COUNT_S      (10UL)
#define TDATA1_ICOUNT_COUNT_M      (0x3fffUL)
#define TDATA1_ICOUNT_COUNT_W(c)   (((c) & TDATA1_ICOUNT_COUNT_M) << TDATA1_ICOUNT_COUNT_S)
#define TDATA1_ICOUNT_COUNT_R(c)   (((c) >> TDATA1_ICOUNT_COUNT_S) & TDATA1_ICOUNT_COUNT_M)
#define TDATA1_ICOUNT_COUNT_MAX    TDATA1_ICOUNT_COUNT_W(TDATA1_ICOUNT_COUNT_M)

#endif /* COMMON_DBTR_H */
