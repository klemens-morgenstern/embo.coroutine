/**
 * @file   test_state.cpp
 * @date   23.05.2017
 * @author Klemens D. Morgenstern
 *
 * Published under [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)
  <pre>
    /  /|  (  )   |  |  /
   /| / |   \/    | /| /
  / |/  |   /\    |/ |/
 /  /   |  (  \   /  |
               )
 </pre>
 */

#include <mw/test/backend.hpp>
#include <mw/test/calltrace.hpp>
#include <mw/coroutine.hpp>

void empty_plain()
{
    static int i = 0;

    std::uint32_t stack1[128];
    mw::coroutine<void()> cr1{stack1};
    auto cr1_func = +[](mw::yield_t<void()> yield_)
            {
                i = 42;
                MW_ASSERT_EXECUTE();
                yield_();
                i = 12;
                MW_ASSERT_EXECUTE();
                yield_();
                i = 776;
                MW_ASSERT_EXECUTE();

            };
    MW_ASSERT(!cr1.started());
    MW_ASSERT_EQUAL(i, 0);
    cr1.spawn(cr1_func);
    MW_ASSERT(cr1.started());

    MW_ASSERT_EQUAL(i, 42);
    cr1.reenter();

    MW_ASSERT_EQUAL(i, 12);
    MW_ASSERT(!cr1.exited());

    cr1.reenter();
    MW_ASSERT(cr1.exited());

    MW_ASSERT_EQUAL(i, 776);

}

void empty_lambda()
{
    int j = 0;

    std::uint32_t stack2[128];
    mw::coroutine<void()> cr2{stack2};
    auto cr2_func = [&j](mw::yield_t<void()> yield_)
            {
        MW_ASSERT_EXECUTE();
                j = 43;
                yield_();
                MW_ASSERT_EXECUTE();
                j = 13;
                yield_();
                MW_ASSERT_EXECUTE();
                j = 777;
            };

    MW_ASSERT(!cr2.started());


    MW_ASSERT_EQUAL(j, 0);

    cr2.spawn(cr2_func);
    MW_ASSERT(cr2.started());


    MW_ASSERT_EQUAL(j, 43);

    cr2.reenter();

    MW_ASSERT_EQUAL(j, 13);



    MW_ASSERT(!cr2.exited());
    cr2.reenter();
    MW_ASSERT(cr2.exited());

    MW_ASSERT_EQUAL(j, 777);
}

void simple_stack()
{
    std::uint32_t stack[128];
    mw::coroutine<void()> cr{stack};

    {
        volatile auto sl = cr.stack_left();
        auto se = reinterpret_cast<std::uint32_t>(stack + 127);
        MW_ASSERT_EQUAL(sl, 127*4);
        sl = cr.stack_ptr() ; MW_ASSERT_EQUAL(sl, se);
        sl = cr.stack_size(); MW_ASSERT_EQUAL(sl, 128*4);
        sl = cr.stack_left(); MW_ASSERT_EQUAL(sl, 127*4);
        sl = cr.stack_used(); MW_ASSERT_EQUAL(sl, 0);
    }
    auto func = +[](mw::yield_t<void()> yield_)
        {
            volatile int i = 13;
            yield_();
            MW_ASSERT_EQUAL(i, 13);
            {
                volatile int j = 42;
                yield_();
                MW_ASSERT_EQUAL(i, 13);
                MW_ASSERT_EQUAL(j, 42);

                {
                    volatile int k = 20;
                    yield_();
                    MW_ASSERT_EQUAL(i, 13);
                    MW_ASSERT_EQUAL(j, 42);
                    MW_ASSERT_EQUAL(k, 20);

                }
                MW_ASSERT_EQUAL(i, 13);
                MW_ASSERT_EQUAL(j, 42);

            }
            MW_ASSERT_EQUAL(i, 13);

        };

    cr.spawn(func);
    {
        volatile int i = 11;
        cr.reenter();
        MW_ASSERT_EQUAL(i, 11);
        {
            volatile int j = 43;
            cr.reenter();
            MW_ASSERT_EQUAL(i, 11);
            MW_ASSERT_EQUAL(j, 43);

            {
                volatile int k = 21;
                cr.reenter();
                MW_ASSERT_EQUAL(i, 11);
                MW_ASSERT_EQUAL(j, 43);
                MW_ASSERT_EQUAL(k, 21);

            }
            MW_ASSERT_EQUAL(i, 11);
            MW_ASSERT_EQUAL(j, 43);

        }
        MW_ASSERT_EQUAL(i, 11);
    }

}

void push_32_no_start()
{
    std::uint32_t stack[128];
    mw::coroutine<void(std::int32_t)> cr{stack};

    std::int32_t out[3] = {0,0,0};
    auto *itr = out;

    auto f = [&](mw::yield_t<void(std::int32_t)> yield_)
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

    it = out[0]; MW_ASSERT_EQUAL(42, it);
    it = out[1]; MW_ASSERT_EQUAL(12, it);
    it = out[2]; MW_ASSERT_EQUAL(32, it);
}


void push_32_start()
{
    std::uint32_t stack[128];
    mw::coroutine<void(std::int32_t)> cr{stack};

    std::int32_t out[3] = {0,0,0};
    auto *itr = out;

    auto f = [&](mw::yield_t<void(std::int32_t)> yield_, std::int32_t input)
         {
            *(itr++) = input;
            *(itr++) = yield_();
            *(itr++) = yield_();
         };

    cr.spawn(f, 43);

    cr.reenter(13);
    cr.reenter(33);

    std::int32_t it;

    it = out[0]; MW_ASSERT_EQUAL(43, it);
    it = out[1]; MW_ASSERT_EQUAL(13, it);
    it = out[2]; MW_ASSERT_EQUAL(33, it);
}

void push_64_no_start()
{
    std::uint32_t stack[128];
    mw::coroutine<void(std::int64_t)> cr{stack};

    std::int64_t out[3] = {0,0,0};
    auto *itr = out;

    auto f = [&](mw::yield_t<void(std::int64_t)> yield_)
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

    it = out[0]; MW_ASSERT_EQUAL(42, it);
    it = out[1]; MW_ASSERT_EQUAL(12, it);
    it = out[2]; MW_ASSERT_EQUAL(32, it);
}


void push_64_start()
{
    std::uint32_t stack[128];
    mw::coroutine<void(std::int64_t)> cr{stack};

    std::int64_t out[3] = {0,0,0};
    auto *itr = out;

    auto f = [&](mw::yield_t<void(std::int64_t)> yield_, std::int64_t input)
         {
            *(itr++) = input;
            *(itr++) = yield_();
            *(itr++) = yield_();
         };

    cr.spawn(f, 0x1234567890ABCDEFll);

    cr.reenter(0xF1234567890ABCDEll);
    cr.reenter(0xEF1234567890ABCDll);

    volatile std::int64_t it;

    it = out[0]; MW_ASSERT_EQUAL(0x1234567890ABCDEFll, it);
    it = out[1]; MW_ASSERT_EQUAL(0xF1234567890ABCDEll, it);
    it = out[2]; MW_ASSERT_EQUAL(0xEF1234567890ABCDll, it);
}

void pull_32()
{
    std::uint32_t stack[128];
    mw::coroutine<std::int32_t()> cr{stack};

    auto f = [](mw::yield_t<std::int32_t()> yield_)
         {
            yield_(42);
            yield_(24);
            return 78;
         };

    volatile auto val = cr.spawn(f); MW_ASSERT_EQUAL(val, 42);
    val = cr.reenter(); MW_ASSERT_EQUAL(val, 24);
    val = cr.reenter(); MW_ASSERT_EQUAL(val, 78);
    MW_ASSERT(cr.exited());
}

void pull_64()
{
    std::uint32_t stack[128];
    mw::coroutine<std::int64_t()> cr{stack};

    auto f = [](mw::yield_t<std::int64_t()> yield_)
         {
            yield_(0x1234567890ABCD42ll);
            yield_(0x1234567890ABCD24ll);
            return 0x1234567890ABCD78ll;
         };

    volatile auto val = cr.spawn(f); MW_ASSERT_EQUAL(val, 0x1234567890ABCD42ll);
    val = cr.reenter(); MW_ASSERT_EQUAL(val, 0x1234567890ABCD24ll);
    val = cr.reenter(); MW_ASSERT_EQUAL(val, 0x1234567890ABCD78ll);
    MW_ASSERT(cr.exited());
}

void push_pull_32()
{
    std::uint32_t stack[128];
    mw::coroutine<std::int32_t(std::int32_t)> cr{stack};

    auto f = +[](mw::yield_t<std::int32_t(std::int32_t)> yield_)
        {
            auto val = yield_(42);
            MW_ASSERT_EQUAL(val, 7);
            val = yield_(val + 5);

            MW_ASSERT_EQUAL(val, 24);

            return val - 5;
        };

    auto val = cr.spawn(f);

    MW_ASSERT_EQUAL(val, 42);
    val = cr.reenter(val-35);

    MW_ASSERT_EQUAL(val, 12);

    val = cr.reenter(val *2);
    MW_ASSERT_EQUAL(val, 19);
}

void push_pull_64()
{
    std::uint32_t stack[128];
    mw::coroutine<std::int64_t(std::int64_t)> cr{stack};

    auto f = +[](mw::yield_t<std::int64_t(std::int64_t)> yield_)
        {
            auto val = yield_(42);
            MW_ASSERT_EQUAL(val, 7);
            val = yield_(val + 5);

            MW_ASSERT_EQUAL(val, 24);

            return val - 5;
        };

    auto val = cr.spawn(f);

    MW_ASSERT_EQUAL(val, 42);
    val = cr.reenter(val-35);

    MW_ASSERT_EQUAL(val, 12);

    val = cr.reenter(val *2);
    MW_ASSERT_EQUAL(val, 19);
}

int main(int argc, char * argv[])
{
    MW_CALL(&empty_plain,  "empty coroutine with plain function");
    MW_CALL(&empty_lambda, "empty coroutine with lambda function");
    MW_CALL(&simple_stack, "simple stack test");
    MW_CALL(&push_32_no_start, "testing pushing 32-bit values without an initial value");
    MW_CALL(&push_32_start, "testing pushing 32-bit values with an initial value");
    MW_CALL(&push_64_no_start, "testing pushing 64-bit value without an initial value");
    MW_CALL(&push_64_start, "testing pushing 64-bit value with an initial value");
    MW_CALL(&pull_32, "pull 32-bit values from the coroutine");
    MW_CALL(&pull_64, "pull 64-bit values from the coroutine");
    MW_CALL(&push_pull_32, "push pull 32 bit values");
    MW_CALL(&push_pull_64, "push pull 64 bit values");
    return MW_REPORT();
}
