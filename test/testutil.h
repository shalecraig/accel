#ifndef TEST_CALLBACK_UTIL_H
#define TEST_CALLBACK_UTIL_H

// TODO: this is so hack.

#define TEST_CALLBACK_NAME(testClass, testName, uniqueIdentifier) \
    generated_##testClass##_##testName##_##uniqueIdentifier##_name

#define TEST_CALLBACK_COUNTER(testClass, testName, uniqueIdentifier) \
    generated_##testClass##_##testName##_##uniqueIdentifier##_counter

#define TEST_CALLBACK(returnSignature, testClass, testName, uniqueIdentifier, args...) \
    /* count the number of times the callback was invoked. */ \
    uint generated_##testClass##_##testName##_##uniqueIdentifier##_counter = 0; \
    /* define a method that proxies calls to the real method. */ \
    returnSignature generated_##testClass##_##testName##_##uniqueIdentifier##_name (args) { \
        /* increment the counter */ \
        generated_##testClass##_##testName##_##uniqueIdentifier##_counter += 1;

#endif
