#include "stdafx.h"
#include "relacy/relacy.hpp"

// RLC - 2014-08-16: this whole block doesn't appear to serve any purpose, seems out-dated and should probably be removed altogether...
//#ifdef RL_TES-
//#define ATOMIC(x) rl::atomic<x>
//#define VAR_T(x) rl::var<x>
//#define ATOMIC_FETCH_ADD(x, v) x($).fetch_add(v)
//#define ATOMIC_COMPARE_EXCHANGE(x, c, v) x($).compare_exchange(c, v)
//#define LOAD_ACQ(x) x($).load(rl::memory_order_acquire)
//#define STORE_REL(x, v) x($).store(v, rl::memory_order_release)
//#else
//#define ATOMIC(x) x volatile
//#define VAR_T(x) x
//#define ATOMIC_FETCH_ADD(x, v) _InterlockedExchangeAdd((long*)&x, v)
//#define ATOMIC_COMPARE_EXCHANGE(x, c, v) interlocked_compare_exchange(x, c, v)
//#define LOAD_ACQ(x) x
//#define STORE_REL(x, v) x = v
//
//template<typename T>
//bool interlocked_compare_exchange(T& x, T& c, T v)
//{
//    T c0 = _InterlockedCompareExchange((long*)&x), v, c);
//    if (c0 == c)
//    {
//        return true;
//    }
//    else
//    {
//        c = c0;
//        return false;
//    }
//}
//#endif

//#include "pcx.h"


/*
template<typename T>
class mpmcq
{
public:
	mpmcq()
	{
        STORE_REL(head_, alloc_block());
        STORE_REL(tail_, LOAD_ACQ(head_));
	}

	void enqueue(T v)
	{
        for (;;)
        {
            block* b = LOAD_ACQ(head_);
            unsigned raw = ATOMIC_FETCH_ADD(b->state_, state_head_inc);
            unsigned idx = raw >> state_head_pos;
            if (idx < item_count)
            {
                STORE_REL(b->data_[idx], v);
                return;
            }
            unsigned last = raw & state_last_msk;
            if (0 == last)
            {
                ATOMIC_COMPARE_EXCHANGE(head_, b, b+1);
            }
            else
            {
                block* b2 = LOAD_ACQ(b->next_);
                if (b2)
                {
                    ATOMIC_COMPARE_EXCHANGE(head_, b, b2);
                }
                else
                {
                    b2 = alloc_block();
                    block* b3 = 0;
                    if (ATOMIC_COMPARE_EXCHANGE(b->next_, b3, b2))
                    {
                        ATOMIC_COMPARE_EXCHANGE(head_, b, b2);
                    }
                    else
                    {
                        for (;;)
                        {
                            b = LOAD_ACQ(head_);
                            while (0 == (LOAD_ACQ(b->state_) & state_last_msk))
                                b = b + 1;
                            while (LOAD_ACQ(b->next_))
                                b = LOAD_ACQ(b->next_) + block_count - 1;
                            b3 = 0;
                            if (ATOMIC_COMPARE_EXCHANGE(b->next_, b3, b2))
                                break;
                        }
                    }
                }
            }
        }
	}

	T dequeue()
	{
        for (;;)
        {
            block* b = LOAD_ACQ(tail_);
            unsigned cmp = LOAD_ACQ(b->state_);
            unsigned tail = cmp & (state_last_msk - 1);
            if (tail < item_count)
            {
                unsigned head = cmp >> state_head_pos;
                if (tail < head)
                {
                    unsigned xchg = cmp + state_tail_inc;
                    if (ATOMIC_COMPARE_EXCHANGE(b->state_, cmp, xchg))
                    {
                        for (;;)
                        {
                            T v = LOAD_ACQ(b->data_[tail]);
                            if (v != T())
                                return v;
                            rl::yield($);
                        }
                    }
                }
                else
                {
                    return T();
                }
            }
            else
            {
                unsigned last = cmp & state_last_msk;
                if (0 == last)
                {
                    ATOMIC_COMPARE_EXCHANGE(tail_, b, b+1);
                }
                else
                {
                    block* b2 = LOAD_ACQ(b->next_);
                    if (0 == b2)
                        return T();
                    ATOMIC_COMPARE_EXCHANGE(tail_, b, b2);
                }
            }
        }
	}

private:
    static unsigned const state_head_pos = 7;
    static unsigned const state_head_inc = 1 << state_head_pos;
    static unsigned const state_last_msk = 1 << 6;
    static unsigned const state_tail_inc = 1 << 0;

    static unsigned const item_count = 2;
    static unsigned const block_count = 16;

    struct block
    {
        //unsigned          head_ : 24;
        //unsigned          last_ : 1;
        //unsigned          tail_ : 7;
        ATOMIC(unsigned)    state_;
        ATOMIC(block*)      next_;
        ATOMIC(T)           data_ [item_count];
    };

    struct superblock
    {
        block           blocks_ [block_count];
    };

	char                pad0_ [64];
    ATOMIC(block*)      head_;
	char                pad1_ [64];
    ATOMIC(block*)      tail_;
	char                pad2_ [64];

    block* alloc_block()
    {
        superblock* sb = RL_NEW(superblock);
        for (int x = 0; x != block_count; ++x)
        {
            block* b = &sb->blocks_[x];
            STORE_REL(b->state_, 0);
            STORE_REL(b->next_, 0);
            for (int y = 0; y != item_count; ++y)
            {
                STORE_REL(b->data_[y], 0);
            }
        }
        STORE_REL(sb->blocks_[block_count - 1].state_, 1 * state_head_inc + 1 * state_tail_inc + state_last_msk);
        return &sb->blocks_[0];
    }
};


struct test_mpmc : rl::test_suite<test_mpmc, 6>
{
    mpmcq<int> q;

    void thread(unsigned idx)
    {
        if (idx < thread_count / 2)
        {
            for (int i = 0; i != 2; ++i)
                q.enqueue(1);
        }
        else
        {
            for (int i = 0; i != 2; ++i)
                q.dequeue();
        }
    }
};
*/



struct thread_node
{
    rl::var<thread_node*>       next;
    rl::var<size_t>             count;
    rl::var<size_t>             unconsumed;
    rl::rl_HANDLE               sema;
    rl::rl_CRITICAL_SECTION     mtx;
};


void on_thread_exit(thread_node*& t_thread_node)
{
    thread_node* head = t_thread_node;
    thread_node* my = 0;
    if (head)
    {
        rl::rl_EnterCriticalSection(&head->mtx, $);
        rl::atomic_thread_fence(rl::memory_order_seq_cst);
        if (head->next($))
        {
            my = head->next($);
            head->next($) = (thread_node*)my->next($);
        }
        else
        {
            my = head;
        }
        rl::atomic_thread_fence(rl::memory_order_seq_cst);
        rl::rl_LeaveCriticalSection(&head->mtx, $);

        while (my->unconsumed($))
        {
            rl::rl_WaitForSingleObject(my->sema, rl::rl_INFINITE, $);
            my->unconsumed($) -= 1;
        }

        rl::rl_DeleteCriticalSection(&my->mtx, $);
        rl::rl_CloseHandle(my->sema, $);
        delete my;
    }

}

struct eventcount
{
    eventcount()
    {
        root($) = 0;
        rl::rl_InitializeCriticalSection(&mtx, $);
    }

    ~eventcount()
    {
        rl::rl_DeleteCriticalSection(&mtx, $);
    }

    void prepare_wait(thread_node*& t_thread_node)
    {
        thread_node* my = 0;
        thread_node* head = t_thread_node;
        if (head)
        {
            rl::rl_EnterCriticalSection(&head->mtx, $);
            rl::atomic_thread_fence(rl::memory_order_seq_cst);
            //RL_ASSERT(head->status == stat_root);
            RL_ASSERT (root($) != head);
            if (head->next($))
            {
                my = head->next($);
                head->next($) = (thread_node*)my->next($);
                my->next($) = 0;

                //node_status st;
                //if (stat_bucket != (st = (node_status)_InterlockedExchange(&my->status, stat_private)))
                //    __asm int 3;
                RL_ASSERT (0 == my->count($));
            }
            else
            {
                my = head;

                //node_status st;
                //if (stat_root != (st = (node_status)_InterlockedExchange(&my->status, stat_private)))
                //    __asm int 3;
                RL_ASSERT(0 == my->count($));
            }
            rl::atomic_thread_fence(rl::memory_order_seq_cst);
            rl::rl_LeaveCriticalSection(&head->mtx, $);
        }
        else
        {
            my = new thread_node;
            my->next($) = 0;
            my->count($) = 0;
            my->unconsumed($) = 0;
            my->sema = rl_CreateSemaphore(0, 0, LONG_MAX, 0);
            //my->status = stat_private;
            rl::rl_InitializeCriticalSection(&my->mtx, $);
        }

        while (my->unconsumed($))
        {
            rl::rl_WaitForSingleObject(my->sema, rl::rl_INFINITE, $);
            my->unconsumed($) -= 1;
        }

        RL_ASSERT(0 == my->next($));
        RL_ASSERT(0 == my->count($));
        //if (my->status != stat_private) __asm int 3;

        rl::rl_EnterCriticalSection(&mtx, $);
        rl::atomic_thread_fence(rl::memory_order_seq_cst);
        RL_ASSERT(root($) != my);
        if (root($))
        {
            my->next($) = (thread_node*)((thread_node*)root($))->next($);
            ((thread_node*)root($))->next($) = my;

            //node_status st;
            //if (stat_private != (st = (node_status)_InterlockedExchange(&my->status, stat_bucket)))
            //    __asm int 3;

            my = root($);
        }
        else
        {
            root($) = my;

            //node_status st;
            //if (stat_private != (st = (node_status)_InterlockedExchange(&my->status, stat_root)))
            //    __asm int 3;
        }
        ((thread_node*)root($))->count($) += 1;
        rl::atomic_thread_fence(rl::memory_order_seq_cst);
        rl::rl_LeaveCriticalSection(&mtx, $);
        t_thread_node = my;
    }

    void wait(thread_node*& t_thread_node)
    {
        thread_node* head = t_thread_node;
        if (head == root($))
        {
            rl::rl_WaitForSingleObject(head->sema, rl::rl_INFINITE, $);
        }
        else
        {
            rl::rl_EnterCriticalSection(&head->mtx, $);
            rl::atomic_thread_fence(rl::memory_order_seq_cst);
            head->unconsumed($) += 1;
            rl::atomic_thread_fence(rl::memory_order_seq_cst);
            rl::rl_LeaveCriticalSection(&head->mtx, $);
        }
    }

    void retire_wait(thread_node*& t_thread_node)
    {
        thread_node* head = t_thread_node;
        if (head == root($))
        {
            rl::rl_EnterCriticalSection(&mtx, $);
            rl::atomic_thread_fence(rl::memory_order_seq_cst);
            if (head == root($))
            {
                thread_node* my = 0;
                head->count($) -= 1;
                if (head->next($))
                {
                    my = head->next($);
                    head->next($) = (thread_node*)my->next($);
                    my->next($) = 0;
                }
                else
                {
                    my = head;
                    root($) = 0;
                }
                rl::atomic_thread_fence(rl::memory_order_seq_cst);
                rl::rl_LeaveCriticalSection(&mtx, $);
                //my->status = stat_root;
                t_thread_node = my;
                return;
            }
            rl::atomic_thread_fence(rl::memory_order_seq_cst);
            rl::rl_LeaveCriticalSection(&mtx, $);
        }
        rl::rl_EnterCriticalSection(&head->mtx, $);
        rl::atomic_thread_fence(rl::memory_order_seq_cst);
        head->unconsumed($) += 1;
        rl::atomic_thread_fence(rl::memory_order_seq_cst);
        rl::rl_LeaveCriticalSection(&head->mtx, $);
    }

    void signal_all()
    {
        //std::
        //_mm_mfence();
        thread_node* head = root($);
        if (0 == head)
            return;
        rl::rl_EnterCriticalSection(&mtx, $);
        rl::atomic_thread_fence(rl::memory_order_seq_cst);
        if (head != root($))
        {
            rl::atomic_thread_fence(rl::memory_order_seq_cst);
            rl::rl_LeaveCriticalSection(&mtx, $);
            return;
        }
        size_t count = head->count($);
        head->count($) = 0;
        root($) = 0;
        rl::atomic_thread_fence(rl::memory_order_seq_cst);
        rl::rl_LeaveCriticalSection(&mtx, $);
        rl::rl_ReleaseSemaphore(head->sema, count, 0, $);
    }

    rl::atomic<thread_node*>       root;
    rl::rl_CRITICAL_SECTION            mtx;

}; 




struct test_ec : rl::test_suite<test_ec, 8>
{
    rl::atomic<int> x [2];
    eventcount ec;

    void before()
    {
        x[0]($) = 0;
        x[1]($) = 0;
    }

    void thread(unsigned idx)
    {
        if (idx < 4)
        {
            for (int i = 0; i != 3; ++i)
            {
                x[idx % 2]($).fetch_add(1);
                ec.signal_all();
            }
        }
        else
        {
            thread_node* my = 0;
            for (int i = 0; i != 3; ++i)
            {
                for (;;)
                {
                    int cmp = x[idx % 2]($);
                    if (cmp > 0)
                    {
                        if (x[idx % 2]($).compare_exchange_strong(cmp, cmp - 1))
                            break;
                    }
                    else
                    {
                        for (;;)
                        {
                            ec.prepare_wait(my);
                            cmp = x[idx % 2]($);
                            if (cmp > 0)
                            {
                                ec.retire_wait(my);
                                break;
                            }
                            ec.wait(my);
                            cmp = x[idx % 2]($);
                            if (cmp > 0)
                            {
                                break;
                            }
                        }
                    }
                }
            }
            on_thread_exit(my);
        }
    }
};



int main()
{
    rl::test_params p;
    p.iteration_count = 2000/*0000*/;
	p.initial_state = "1000";
    rl::simulate<test_ec>(p);
}

