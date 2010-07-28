/*
 * rust_dom.h
 */

#ifndef RUST_DOM_H
#define RUST_DOM_H

#include "sync/lock_free_queue.h"
#include "util/hash_map.h"

#include "rust_proxy.h"
#include "rust_message.h"

struct rust_dom
{
    // Fields known to the compiler:
    uintptr_t interrupt_flag;

    // Fields known only by the runtime:

    // NB: the root crate must remain in memory until the root of the
    // tree of domains exits. All domains within this tree have a
    // copy of this root_crate value and use it for finding utility
    // glue.
    rust_crate const *root_crate;
    rust_log _log;
    rust_srv *srv;
    ptr_vec<rust_task> running_tasks;
    ptr_vec<rust_task> blocked_tasks;
    ptr_vec<rust_task> dead_tasks;
    ptr_vec<rust_crate_cache> caches;
    randctx rctx;
    rust_task *root_task;
    rust_task *curr_task;
    int rval;

    condition_variable _progress;

    hash_map<rust_task *, rust_proxy<rust_task> *> _task_proxies;

    // Incoming messages from other domains.
    condition_variable _incoming_message_pending;
    lock_free_queue _incoming_message_queue;

#ifndef __WIN32__
    pthread_attr_t attr;
#endif

    rust_dom(rust_srv *srv, rust_crate const *root_crate);
    ~rust_dom();

    void activate(rust_task *task);
    void log(rust_task *task, uint32_t logbit, char const *fmt, ...);
    void log(uint32_t logbit, char const *fmt, ...);
    rust_log & get_log();
    void logptr(char const *msg, uintptr_t ptrval);
    template<typename T>
    void logptr(char const *msg, T* ptrval);
    void fail();
    void *malloc(size_t sz);
    void *calloc(size_t sz);
    void *realloc(void *data, size_t sz);
    void free(void *p);

    void send_message(rust_message *message);
    void drain_incoming_message_queue();
    rust_proxy<rust_task> *get_task_proxy(rust_task *task);
    void delete_proxies();

#ifdef __WIN32__
    void win32_require(LPCTSTR fn, BOOL ok);
#endif

    rust_crate_cache *get_cache(rust_crate const *crate);
    size_t n_live_tasks();
    void add_task_to_state_vec(ptr_vec<rust_task> *v, rust_task *task);
    void remove_task_from_state_vec(ptr_vec<rust_task> *v, rust_task *task);
    const char *state_vec_name(ptr_vec<rust_task> *v);

    void reap_dead_tasks();
    rust_task *schedule_task();
    int start_main_loop();

    void log_state();
};

//
// Local Variables:
// mode: C++
// fill-column: 78;
// indent-tabs-mode: nil
// c-basic-offset: 4
// buffer-file-coding-system: utf-8-unix
// compile-command: "make -k -C .. 2>&1 | sed -e 's/\\/x\\//x:\\//g'";
// End:
//

#endif /* RUST_DOM_H */
