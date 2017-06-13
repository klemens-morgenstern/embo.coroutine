/**
@file   couroutine.cpp
@ate   10.06.2017
@author Klemens D. Morgenstern

Published under [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)

<pre>
   /  /|  (  )   |  |  /
  /| / |   \/    | /| /
 / |/  |   /\    |/ |/
/  /   |  (  \   /  |
</pre>              )

ARM Core Register

Register | Synonym | Special | Role in the procedure call standard
---------|---------|---------|------------------------------------
r15      |         | PC      | The Program Counter.
r14      |         | LR      | The Link Register.
r13      |         | SP      | The Stack Pointer.
r12      |         | IP      | The Intra-Procedure-call scratch register.
r11      | v8      |         | Variable-register 8.
r10      | v7      |         | Variable-register 7.
r9       | v6      |  SB TR  | Platform register. The meaning of this register is defined by the platform standard.
r8       | v5      |         | Variable-register 5.
r7       | v4      |         | Variable register 4.
r6       | v3      |         | Variable register 3.
r5       | v2      |         | Variable register 2.
r4       | v1      |         | Variable register 1.
r3       | a4      |         | Argument / scratch register 4.
r2       | a3      |         | Argument / scratch register 3.
r1       | a2      |         | Argument / result / scratch register 2.
r0       | a1      |         | Argument / result / scratch register 1
*/

.text
.globl __mw_make_context_0
.align 2
.type __mw_make_context_0,%function
.thumb
.syntax unified
__mw_make_context_0:
    @__mw_make_context_0(impl * const, void * target, void * executor);
    @the executor has the following signature: (impl * const, void * func) --> we don't even need to change the
	@now impl also points to the stack_ptr store we need.

    push {v1-v8, lr} @push  the link register
    mov v1, sp   @move the stack pointer to v1
    ldr sp, [a1] @set the stack pointer
    str v1, [a1] @store the old stack pointer

    bx a3               @call the function -> note the link register on top of the stack still points to the old location, so we'll get this back in switch_context

.text
.globl __mw_make_context_1
.align 2
.type __mw_make_context_1,%function
.thumb
.syntax unified
__mw_make_context_1:
    @__mw_make_context_0(impl * const, void * target, void * executor, std::uint32_t);
    @the executor has the following signature: (impl * const, void * func, std::uint32_t) --> we don't even need to change the
	@now impl also points to the stack_ptr store we need.

    @we need to store v0-v8, those are variables. tje IP, SP, LR, PC
    push {v1-v8, lr} @push  the link register

    mov v1, sp   @move the stack pointer to v1
    ldr sp, [a1] @set the stack pointer
    str v1, [a1] @store the old stack pointer

	mov v1, a3 @move the executor

	mov a3, a4 @move the value to the proper position
    bx v1      @call the function -> note the link register on top of the stack still points to the old location, so we'll get this back in switch_context


.text
.globl __mw_make_context_2
.align 2
.type __mw_make_context_2,%function
.thumb
.syntax unified
__mw_make_context_2:
    @__mw_make_context_0(impl * const, void * target, void * executor, std::uint64_t * );
    @the executor has the following signature: (impl * const, void * func, std::uint64_t) --> we don't even need to change the
	@now impl also points to the stack_ptr store we need.

    @we need to store v0-v8, those are variables. tje IP, SP, LR, PC
    push {v1-v8, lr} @push  the link register

    mov v1, sp   @move the stack pointer to v1
    ldr sp, [a1] @set the stack pointer
    str v1, [a1] @store the old stack pointer

	mov v1, a3 @move the executor
	@load the pointed to value
	ldr a3, [a4]
	ldr a4, [a4, #4]

    bx v1      @call the function -> note the link register on top of the stack still points to the old location, so we'll get this back in switch_context


.text
.globl __mw_switch_context_0
.align 2
.type __mw_switch_context_0,%function
.thumb
.syntax unified
__mw_switch_context_0:
    @__mw_make_context_0(impl * const);
    push {v1-v8, lr}

    mov v1, sp
    ldr sp, [a1]
    str v1, [a1]

    pop {v1-v8, lr}

    bx lr

.text
.globl __mw_switch_context_1
.align 2
.type __mw_switch_context_1,%function
.thumb
.syntax unified
__mw_switch_context_1:
    @__mw_make_context_1(std::uint32_t, impl * const);
    push {v1-v8, lr}

    mov v1, sp
    ldr sp, [a2]
    str v1, [a2]

    pop {v1-v8, lr}

    bx lr

.text
.globl __mw_switch_context_2
.align 2
.type __mw_switch_context_2,%function
.thumb
.syntax unified
__mw_switch_context_2:
    @__mw_make_context_2(std::uint64_t, impl * const);
    push {v1-v8, lr}

    mov v1, sp
    ldr sp, [a3]
    str v1, [a3]

    pop {v1-v8, lr}

    bx lr
