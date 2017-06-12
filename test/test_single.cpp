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

#include <stdio.h>

void empty_plain()
{
    static int i = 0;

    std::array<std::uint32_t, 128> stack1;
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

    std::array<std::uint32_t, 128> stack2;
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


int main(int argc, char * argv[])
{
    MW_CALL(&empty_plain,  "empty coroutine with plain function");
    MW_CALL(&empty_lambda, "empty coroutine with lambda function");

    return 0;
}
