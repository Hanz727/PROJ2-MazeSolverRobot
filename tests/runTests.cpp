#include "../src/arduino_sketch/vec2.h"
#include "runTests.h"
#include <cfloat>
#include "../src/arduino_sketch/platformDefinitions.h"

#ifdef WINDOWS
    #include <iostream>
#endif // WINDOWS

static int testsRan = 0;
static int testsPassed = 0;


template <typename T, typename U>
bool ASSERT_EQUAL(const T& expected, const U& actual) {
    testsRan++;
    bool ret = expected == actual;
#ifdef WINDOWS
    std::cout << "Test " << testsRan;
    if (ret)
        std::cout << " Passed\n";
    else
        std::cout << " Failed\n";
#endif
    testsPassed += ret;
    return ret;
}


vec2<int> vec2_addition_simple(vec2<int> a, vec2<int> b) {
    return a + b;
}

vec2<double> vec2_addition_promotion(vec2<double> a, vec2<int> b) {
    return a + b;
}

vec2<int> vec2_subtraction_simple(vec2<int> a, vec2<int> b) {
    return a - b;
}

vec2<double> vec2_subtraction_promotion(vec2<double> a, vec2<int> b) {
    return a - b;
}

vec2<int> vec2_multiplication_simple(vec2<int> a, vec2<int> b) {
    return a * b;
}

vec2<double> vec2_multiplication_promotion(vec2<double> a, vec2<int> b) {
    return a * b;
}

vec2<int> vec2_division_simple(vec2<int> a, vec2<int> b) {
    return a / b;
}

vec2<double> vec2_division_promotion(vec2<double> a, vec2<int> b) {
    return a / b;
}


void vec2_addition_test() {
    // Simple integer vector addition tests
    ASSERT_EQUAL((vec2_addition_simple({ 0,0 }, { 0,0 })), (vec2<int>{0, 0}));
    ASSERT_EQUAL((vec2_addition_simple({ 1,1 }, { -1,-1 })), (vec2<int>{0, 0}));
    ASSERT_EQUAL((vec2_addition_simple({ 1000, 500 }, { 2000, 1000 })), (vec2<int>{3000, 1500}));
    ASSERT_EQUAL((vec2_addition_simple({ -5,-7 }, { 5,7 })), (vec2<int>{0, 0}));

    // Tests with negative numbers
    ASSERT_EQUAL((vec2_addition_simple({ -1, -1 }, { 1, 1 })), (vec2<int>{0, 0}));
    ASSERT_EQUAL((vec2_addition_simple({ -1000, 2000 }, { 500, -1000 })), (vec2<int>{-500, 1000}));

    // Promotion tests (int to double conversion)
    ASSERT_EQUAL((vec2_addition_promotion({ 0.5, 0.5 }, { 1, 1 })), (vec2<double>{1.5, 1.5}));
    ASSERT_EQUAL((vec2_addition_promotion({ 1.23, 4.56 }, { 7, 8 })), (vec2<double>{8.23, 12.56}));
    ASSERT_EQUAL((vec2_addition_promotion({ 0.0, 0.0 }, { 3, 5 })), (vec2<double>{3.0, 5.0}));
    ASSERT_EQUAL((vec2_addition_promotion({ 3.14159, 2.71828 }, { 1, 1 })), (vec2<double>{4.14159, 3.71828}));
    ASSERT_EQUAL((vec2_addition_promotion({ 0.0, -0.5 }, { 0, 0 })), (vec2<double>{0.0, -0.5}));

    // Boundary tests for large/small numbers
    ASSERT_EQUAL((vec2_addition_simple({ INT_MAX, INT_MIN }, { 1, -1 })), (vec2<int>{INT_MAX + 1, INT_MIN - 1}));
    ASSERT_EQUAL((vec2_addition_promotion({ DBL_MAX, DBL_MIN }, { 0, 0 })), (vec2<double>{DBL_MAX, DBL_MIN}));
}

void vec2_subtraction_test() {
    // Simple integer vector subtraction tests
    ASSERT_EQUAL((vec2_subtraction_simple({ 0, 0 }, { 0, 0 })), (vec2<int>{0, 0}));
    ASSERT_EQUAL((vec2_subtraction_simple({ 5, 3 }, { 2, 1 })), (vec2<int>{3, 2}));
    ASSERT_EQUAL((vec2_subtraction_simple({ 1000, 500 }, { 200, 100 })), (vec2<int>{800, 400}));
    ASSERT_EQUAL((vec2_subtraction_simple({ -5, -7 }, { 5, 7 })), (vec2<int>{-10, -14}));

    // Tests with negative numbers
    ASSERT_EQUAL((vec2_subtraction_simple({ -5, 3 }, { 1, -2 })), (vec2<int>{-6, 5}));
    ASSERT_EQUAL((vec2_subtraction_simple({ -1000, 2000 }, { -500, 1000 })), (vec2<int>{-500, 1000}));

    // Promotion tests (int to double conversion)
    ASSERT_EQUAL((vec2_subtraction_promotion({ 10.5, 7.25 }, { 3, 2 })), (vec2<double>{7.5, 5.25}));
    ASSERT_EQUAL((vec2_subtraction_promotion({ 15.67, 2.89 }, { 5, 3 })), (vec2<double>{10.67, -0.11}));
    ASSERT_EQUAL((vec2_subtraction_promotion({ 0.0, 0.0 }, { 3, 5 })), (vec2<double>{-3.0, -5.0}));

    // Corrected promotion tests with one vec2<int> and one vec2<double>
    ASSERT_EQUAL((vec2_subtraction_promotion({ 5.0, 4.5 }, { 2, 1 })), (vec2<double>{3.0, 3.5}));
    ASSERT_EQUAL((vec2_subtraction_promotion({ 3.14, 2.71 }, { 1, 1 })), (vec2<double>{2.14, 1.71}));

    // Boundary tests for large/small numbers
    ASSERT_EQUAL((vec2_subtraction_simple({ INT_MAX, INT_MIN }, { 1, -1 })), (vec2<int>{INT_MAX - 1, INT_MIN + 1}));
    ASSERT_EQUAL((vec2_subtraction_promotion({ DBL_MAX, DBL_MIN }, { 0, 0 })), (vec2<double>{DBL_MAX, DBL_MIN}));
}

void vec2_multiplication_test() {
    // Simple integer vector multiplication tests
    ASSERT_EQUAL((vec2_multiplication_simple({ 0, 0 }, { 0, 0 })), (vec2<int>{0, 0}));
    ASSERT_EQUAL((vec2_multiplication_simple({ 2, 3 }, { 4, 5 })), (vec2<int>{8, 15}));
    ASSERT_EQUAL((vec2_multiplication_simple({ -2, 3 }, { -4, 5 })), (vec2<int>{8, 15}));
    ASSERT_EQUAL((vec2_multiplication_simple({ 100, 50 }, { 2, 4 })), (vec2<int>{200, 200}));

    // Tests with negative numbers
    ASSERT_EQUAL((vec2_multiplication_simple({ -3, 4 }, { 2, -1 })), (vec2<int>{-6, -4}));
    ASSERT_EQUAL((vec2_multiplication_simple({ -2, -3 }, { 4, 5 })), (vec2<int>{-8, -15}));

    // Promotion tests (int to double conversion)
    ASSERT_EQUAL((vec2_multiplication_promotion({ 2.5, 3.5 }, { 4, 5 })), (vec2<double>{10.0, 17.5}));
    ASSERT_EQUAL((vec2_multiplication_promotion({ 1.5, 2.5 }, { -2, 4 })), (vec2<double>{-3.0, 10.0}));
    ASSERT_EQUAL((vec2_multiplication_promotion({ 1.0, 2.0 }, { 0, 1 })), (vec2<double>{0.0, 2.0}));

    // Corrected promotion tests with one vec2<int> and one vec2<double>
    ASSERT_EQUAL((vec2_multiplication_promotion({ 3.0, 4.0 }, { 2, 3 })), (vec2<double>{6.0, 12.0}));
    ASSERT_EQUAL((vec2_multiplication_promotion({ 5.5, 2.2 }, { -1, 3 })), (vec2<double>{-5.5, 6.6}));

    // Boundary tests for large/small numbers
    ASSERT_EQUAL((vec2_multiplication_simple({ INT_MAX, INT_MIN }, { 1, -1 })), (vec2<int>{INT_MAX, INT_MIN}));
    ASSERT_EQUAL((vec2_multiplication_promotion({ DBL_MAX, DBL_MIN }, { 1, 1 })), (vec2<double>{DBL_MAX, DBL_MIN}));
}

void vec2_division_test() {
    // Simple integer vector division tests
    ASSERT_EQUAL((vec2_division_simple({ 6, 12 }, { 2, 3 })), (vec2<int>{3, 4}));
    ASSERT_EQUAL((vec2_division_simple({ 10, -15 }, { 5, -3 })), (vec2<int>{2, 5}));
    ASSERT_EQUAL((vec2_division_simple({ 100, 200 }, { 10, 50 })), (vec2<int>{10, 4}));

    // Tests with negative numbers
    ASSERT_EQUAL((vec2_division_simple({ -6, -8 }, { 2, 2 })), (vec2<int>{-3, -4}));
    ASSERT_EQUAL((vec2_division_simple({ -100, 150 }, { -50, 25 })), (vec2<int>{2, 6}));

    // Promotion tests (int to double conversion)
    ASSERT_EQUAL((vec2_division_promotion({ 10.0, 15.0 }, { 2, 3 })), (vec2<double>{5.0, 5.0}));
    ASSERT_EQUAL((vec2_division_promotion({ 50.0, 100.0 }, { 5, 25 })), (vec2<double>{10.0, 4.0}));
    ASSERT_EQUAL((vec2_division_promotion({ 3.5, 7.5 }, { 1, 3 })), (vec2<double>{3.5, 2.5}));

    // Corrected promotion tests with one vec2<int> and one vec2<double>
    ASSERT_EQUAL((vec2_division_promotion({ 6.0, 12.0 }, { 3, 4 })), (vec2<double>{2.0, 3.0}));
    ASSERT_EQUAL((vec2_division_promotion({ 4.5, 9.0 }, { -3, 3 })), (vec2<double>{-1.5, 3.0}));

    // Boundary tests for large/small numbers
    ASSERT_EQUAL((vec2_division_simple({ INT_MAX-1, INT_MIN+1 }, { 1, -1 })), (vec2<int>{INT_MAX-1, -(INT_MIN+1)}));
    ASSERT_EQUAL((vec2_division_promotion({ DBL_MAX, DBL_MIN }, { 1, 1 })), (vec2<double>{DBL_MAX, DBL_MIN}));
}



void vec2_test_suite() {
    vec2_addition_test();
    vec2_subtraction_test();
    vec2_multiplication_test();
    vec2_division_test();

}

bool runTests() {
    vec2_test_suite();
#ifdef WINDOWS
    if (testsRan == testsPassed)
        std::cout << "\nAll tests passed\n";
    else {
        std::cout << "\nPassed " << testsPassed << "/" << testsRan << "\n";
    }
#endif
    return testsRan == testsPassed;
}
