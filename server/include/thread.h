#ifndef _THREAD_H
#define _THREAD_H
/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl.
 *
 * Change Date: 2019-07-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

/**
 * @file thread.h       The gateway threading interface
 *
 * An encapsulation of the threading used by the gateway. This is designed to
 * isolate the majority of the gateway code from the pthread library, enabling
 * the gateway to be ported to a different threading package with the minimum
 * of changes.
 */

/**
 * Thread type and thread identifier function macros
 */
#include <pthread.h>

/** Thread type */
#define THREAD         pthread_t

/** Thread identity
 *
 * This is never used to access the thread object but only to identify which
 * thread is active. This information is almost always logged so returning
 * an integer makes more sense. */
#define thread_self()  (size_t)pthread_self()

extern THREAD *thread_start(THREAD *thd, void (*entry)(void *), void *arg);
extern void thread_wait(THREAD thd);
extern void thread_millisleep(int ms);

#endif
