/**
 * @file   test_state.cpp
 * @date   23.05.2017
 * @author Klemens D. Morgenstern
 *
 * Published under [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)
 */

#include <cstdint>
#include <embo/coroutine.hpp>

static std::size_t test_cnt = 0;
#define TEST_REPORT() test_cnt
#define TEST_ASSERT(exp) if (!(exp)) test_cnt++;
#define TEST_ASSERT_EQUAL(a, b) TEST_ASSERT(a == b);

void empty_plain()
{
    static int i = 0;

    std::uint32_t stack1[128];
    embo::coroutine<void()> cr1{stack1};
    auto cr1_func = +[](embo::yield_t<void()> yield_)
            {
                i = 42;
                yield_();
                i = 12;
                yield_();
                i = 776;
             };
    TEST_ASSERT(!cr1.started());
    TEST_ASSERT_EQUAL(i, 0);
    cr1.spawn(cr1_func);
    TEST_ASSERT(cr1.started());

    TEST_ASSERT_EQUAL(i, 42);
    cr1.reenter();

    TEST_ASSERT_EQUAL(i, 12);
    TEST_ASSERT(!cr1.exited());

    cr1.reenter();
    TEST_ASSERT(cr1.exited());

    TEST_ASSERT_EQUAL(i, 776);

}

void empty_lambda()
{
    int j = 0;

    std::uint32_t stack2[128];
    embo::coroutine<void()> cr2{stack2};
    auto cr2_func = [&j](embo::yield_t<void()> yield_)
            {
                j = 43;
                yield_();
                j = 13;
                yield_();
                j = 777;
            };

    TEST_ASSERT(!cr2.started());


    TEST_ASSERT_EQUAL(j, 0);

    cr2.spawn(cr2_func);
    TEST_ASSERT(cr2.started());


    TEST_ASSERT_EQUAL(j, 43);

    cr2.reenter();

    TEST_ASSERT_EQUAL(j, 13);



    TEST_ASSERT(!cr2.exited());
    cr2.reenter();
    TEST_ASSERT(cr2.exited());

    TEST_ASSERT_EQUAL(j, 777);
}

void simple_stack()
{
    std::uint32_t stack[128];
    embo::coroutine<void()> cr{stack};

    {
        volatile auto sl = cr.stack_left();
        auto se = reinterpret_cast<std::uint32_t>(stack + 127);
        TEST_ASSERT_EQUAL(sl, 127*4);
        sl = cr.stack_ptr() ; TEST_ASSERT_EQUAL(sl, se);
        sl = cr.stack_size(); TEST_ASSERT_EQUAL(sl, 128*4);
        sl = cr.stack_left(); TEST_ASSERT_EQUAL(sl, 127*4);
        sl = cr.stack_used(); TEST_ASSERT_EQUAL(sl, 0);
    }
    auto func = +[](embo::yield_t<void()> yield_)
        {
            volatile int i = 13;
            yield_();
            TEST_ASSERT_EQUAL(i, 13);
            {
                volatile int j = 42;
                yield_();
                TEST_ASSERT_EQUAL(i, 13);
                TEST_ASSERT_EQUAL(j, 42);

                {
                    volatile int k = 20;
                    yield_();
                    TEST_ASSERT_EQUAL(i, 13);
                    TEST_ASSERT_EQUAL(j, 42);
                    TEST_ASSERT_EQUAL(k, 20);

                }
                TEST_ASSERT_EQUAL(i, 13);
                TEST_ASSERT_EQUAL(j, 42);

            }
            TEST_ASSERT_EQUAL(i, 13);

        };

    cr.spawn(func);
    {
        volatile int i = 11;
        cr.reenter();
        TEST_ASSERT_EQUAL(i, 11);
        {
            volatile int j = 43;
            cr.reenter();
            TEST_ASSERT_EQUAL(i, 11);
            TEST_ASSERT_EQUAL(j, 43);

            {
                volatile int k = 21;
                cr.reenter();
                TEST_ASSERT_EQUAL(i, 11);
                TEST_ASSERT_EQUAL(j, 43);
                TEST_ASSERT_EQUAL(k, 21);

            }
            TEST_ASSERT_EQUAL(i, 11);
            TEST_ASSERT_EQUAL(j, 43);

        }
        TEST_ASSERT_EQUAL(i, 11);
    }

}

void push_32_no_start()
{
    std::uint32_t stack[128];
    embo::coroutine<void(std::int32_t)> cr{stack};

    std::int32_t out[3] = {0,0,0};
    auto *itr = out;

    auto f = [&](embo::yield_t<void(std::int32_t)> yield_)
         {
            *(itr++) = yield_();
            *(itr++) = yield_();
            *(itr++) = yield_();
         };

    cr.spawn(f);

    cr.reenter(42);
    cr.reenter(12);
    cr.reenter(32);

    std::int32_t it;

    it = out[0]; TEST_ASSERT_EQUAL(42, it);
    it = out[1]; TEST_ASSERT_EQUAL(12, it);
    it = out[2]; TEST_ASSERT_EQUAL(32, it);
}


void push_32_start()
{
    std::uint32_t stack[128];
    embo::coroutine<void(std::int32_t)> cr{stack};

    std::int32_t out[3] = {0,0,0};
    auto *itr = out;

    auto f = [&](embo::yield_t<void(std::int32_t)> yield_, std::int32_t input)
         {
            *(itr++) = input;
            *(itr++) = yield_();
            *(itr++) = yield_();
         };

    cr.spawn(f, 43);

    cr.reenter(13);
    cr.reenter(33);

    std::int32_t it;

    it = out[0]; TEST_ASSERT_EQUAL(43, it);
    it = out[1]; TEST_ASSERT_EQUAL(13, it);
    it = out[2]; TEST_ASSERT_EQUAL(33, it);
}

void push_64_no_start()
{
    std::uint32_t stack[128];
    embo::coroutine<void(std::int64_t)> cr{stack};

    std::int64_t out[3] = {0,0,0};
    auto *itr = out;

    auto f = [&](embo::yield_t<void(std::int64_t)> yield_)
         {
            *(itr++) = yield_();
            *(itr++) = yield_();
            *(itr++) = yield_();
         };

    cr.spawn(f);

    cr.reenter(42);
    cr.reenter(12);
    cr.reenter(32);

    volatile std::int64_t it;

    it = out[0]; TEST_ASSERT_EQUAL(42, it);
    it = out[1]; TEST_ASSERT_EQUAL(12, it);
    it = out[2]; TEST_ASSERT_EQUAL(32, it);
}


void push_64_start()
{
    std::uint32_t stack[128];
    embo::coroutine<void(std::int64_t)> cr{stack};

    std::int64_t out[3] = {0,0,0};
    auto *itr = out;

    auto f = [&](embo::yield_t<void(std::int64_t)> yield_, std::int64_t input)
         {
            *(itr++) = input;
            *(itr++) = yield_();
            *(itr++) = yield_();
         };

    cr.spawn(f, 0x1234567890ABCDEFll);

    cr.reenter(0xF1234567890ABCDEll);
    cr.reenter(0xEF1234567890ABCDll);

    volatile std::int64_t it;

    it = out[0]; TEST_ASSERT_EQUAL(0x1234567890ABCDEFll, it);
    it = out[1]; TEST_ASSERT_EQUAL(0xF1234567890ABCDEll, it);
    it = out[2]; TEST_ASSERT_EQUAL(0xEF1234567890ABCDll, it);
}

void pull_32()
{
    std::uint32_t stack[128];
    embo::coroutine<std::int32_t()> cr{stack};

    auto f = [](embo::yield_t<std::int32_t()> yield_)
         {
            yield_(42);
            yield_(24);
            return 78;
         };

    volatile auto val = cr.spawn(f); TEST_ASSERT_EQUAL(val, 42);
    val = cr.reenter(); TEST_ASSERT_EQUAL(val, 24);
    val = cr.reenter(); TEST_ASSERT_EQUAL(val, 78);
    TEST_ASSERT(cr.exited());
}

void pull_64()
{
    std::uint32_t stack[128];
    embo::coroutine<std::int64_t()> cr{stack};

    auto f = [](embo::yield_t<std::int64_t()> yield_)
         {
            yield_(0x1234567890ABCD42ll);
            yield_(0x1234567890ABCD24ll);
            return 0x1234567890ABCD78ll;
         };

    volatile auto val = cr.spawn(f); TEST_ASSERT_EQUAL(val, 0x1234567890ABCD42ll);
    val = cr.reenter(); TEST_ASSERT_EQUAL(val, 0x1234567890ABCD24ll);
    val = cr.reenter(); TEST_ASSERT_EQUAL(val, 0x1234567890ABCD78ll);
    TEST_ASSERT(cr.exited());
}

void push_pull_32()
{
    std::uint32_t stack[128];
    embo::coroutine<std::int32_t(std::int32_t)> cr{stack};

    auto f = +[](embo::yield_t<std::int32_t(std::int32_t)> yield_)
        {
            auto val = yield_(42);
            TEST_ASSERT_EQUAL(val, 7);
            val = yield_(val + 5);

            TEST_ASSERT_EQUAL(val, 24);

            return val - 5;
        };

    auto val = cr.spawn(f);

    TEST_ASSERT_EQUAL(val, 42);
    val = cr.reenter(val-35);

    TEST_ASSERT_EQUAL(val, 12);

    val = cr.reenter(val *2);
    TEST_ASSERT_EQUAL(val, 19);
}

void push_pull_64()
{
    std::uint32_t stack[128];
    embo::coroutine<std::int64_t(std::int64_t)> cr{stack};

    auto f = +[](embo::yield_t<std::int64_t(std::int64_t)> yield_)
        {
            auto val = yield_(42);
            TEST_ASSERT_EQUAL(val, 7);
            val = yield_(val + 5);

            TEST_ASSERT_EQUAL(val, 24);

            return val - 5;
        };

    auto val = cr.spawn(f);

    TEST_ASSERT_EQUAL(val, 42);
    val = cr.reenter(val-35);

    TEST_ASSERT_EQUAL(val, 12);

    val = cr.reenter(val *2);
    TEST_ASSERT_EQUAL(val, 19);
}

int main(int argc, char * argv[])
{
    empty_plain();
    empty_lambda();
    simple_stack();
    push_32_no_start();
    push_32_start();
    push_64_no_start();
    push_64_start();
    pull_32();
    pull_64();
    push_pull_32();
    push_pull_64();
    return TEST_REPORT();
}
