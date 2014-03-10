#ifndef TEST_CALLBACK_UTIL_H
#define TEST_CALLBACK_UTIL_H

// TODO: build a list of params
// TODO: build a list of returned values.

#define TEST_CALLBACK_NAMESPACE(testClass, testName, uniqueIdentifier) \
    generated_##testClass##_##testName##_##uniqueIdentifier##_

#define TEST_CALLBACK_NAME(testClass, testName, uniqueIdentifier) \
    TEST_CALLBACK_NAMESPACE(testClass, testName, uniqueIdentifier)

#define TEST_CALLBACK_COUNTER(testClass, testName, uniqueIdentifier) \
    generated_##testClass##_##testName##_##uniqueIdentifier##_counter

#define TEST_CALLBACK(returnSignature, testClass, testName, uniqueIdentifier, args...) \
    /* count the number of times the callback was invoked. */ \
    uint TEST_CALLBACK_COUNTER(testClass, testName, uniqueIdentifier) = 0; \
    /* define a method that proxies calls to the real method. */ \
    returnSignature TEST_CALLBACK_NAME(testClass, testName, uniqueIdentifier) (args) { \
        /* increment the counter */ \
        TEST_CALLBACK_COUNTER(testClass, testName, uniqueIdentifier) += 1;

#endif
