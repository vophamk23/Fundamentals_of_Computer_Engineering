/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#ifndef OSCFG_H
#define OSCFG_H

#define MLQ_SCHED 1
#define MAX_PRIO 140

#define MM_PAGING
// Bỏ cmt 3 dòng này để chạy ./os_sched ./os sched_0 ./os shced_1
// Cmt 3 dòng này để chạy ./os os_0_mlp_paging ./os os_1_mlp_paging và các input còn lại

// #define MM_FIXED_MEMSZ
// #define VMDBG 1
// #define MMDBG 1
#define IODUMP 1
#define PAGETBL_DUMP 1

/*
 * @bksysnet:
 *    The address mode must be explicitly define in MM64 or no-MM64
 *    by commenting one of these following lines and uncommenting the other
 *
 */
#define MM64 1
// #undef MM64

#endif
