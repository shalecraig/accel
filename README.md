# accel [![Build Status](https://travis-ci.org/shalecraig/accel.png?branch=master)](https://travis-ci.org/shalecraig/accel)
---

#### What is accel?
Accel is a simple library written in C that uses accelerometer data to find gestures in realtime. 
Though it is written with the Pebble in mind, it should work with other embedded devices as well.

## API Overview

### Return types

All methods in the accel API return a success value or error codes.

Return values are as follows:

- `ACCEL_SUCCESS` - the method being called is successful.
- `ACCEL_PARAM_ERROR` - parameters being passed are invalid.
- `ACCEL_MALLOC_ERROR` - returned when malloc fails
- `ACCEL_NO_VALID_GESTURE` - when gestures are queried, this value is returned when there are no applicable gestures.
- `ACCEL_INTERNAL_ERROR` - there is an internal error in the accel library, usually due to consistency issues in parameter attributes. It's best to report these.

### Methods

- `int accel_generate_state(accel_state **state, int dimensions, int window_size)`
    - Generates and sets up a valid `accel_state` object that tracks input of `dimensions` dimensions, using a rolling window of size `window_size` to smooth measurements.
- `int accel_destroy_state(accel_state **state)`
    - Destroys any `accel_state` objects passed in, and resets `state` to point to the `NULL` value.
- `int accel_start_record_gesture(accel_state *state, int *gesture)`
    - Starts to record a gesture, saving the gesture id to `gesture`.
- `int accel_end_record_gesture(accel_state *state, int gesture_id)`
    - Completes recording the gesture at gesture id `gesture_id`.
- `int accel_process_timer_tick(accel_state *state, int *accel_data)`
    - Called whenever a tick happens, this allows `accel` to update projected most-likely gestures.
- `int accel_find_most_likely_gesture(accel_state *state, int *gesture_id, int *distance)`
    - Returns the id of the gesture that is most likely to have occurred.

## Using accel in Your Project

### Getting the accel Source Code
Getting the accel source is easy. Simply clone the github repository as follows:
```
git clone https://github.com/shalecraig/accel.git
```

### Using accel in Pebble Projects

Simply copy all files from the `accel/src` directory in accel into the `accel/src` directory in your pebble app. The `pebble build` command will find the newly added files, so everything should work smoothly.

```
cp accel/src/* myproject/src/
```

A sample of library usage is in the `accel/samples` directory.

### Using accel in Other Projects

The source code is in the `accel/src` directory. Simply include `accel.h` and make sure you compile all files. If you needed to make changes, feel free to submit pull requests to [the accel repo](https://github.com/shalecraig/accel).

## Running tests

After going to the test directory (`cd test/`), running `make run` should execute tests successfully.

TODO: As of right now, C tests are compiled by the C++ compiler. We should fix this.

## Current Version

The accel version is currently 1.0.0b.
This version is defined in `accel.h` as `ACCEL_VERSION_CODE` , which is generated using the `ACCEL_VERSION_GEN` macro.

Attribution
=====

Gesture recognition adapted from:
- Jiayang Liu, Zhen Wang, Lin Zhong, Jehan Wickramasuriya, and Venu Vasudevan, "uWave: Accelerometer-based personalized gesture recognition and its applications," in Proc. IEEE Int. Conf. Pervasive Computing and Communication (PerCom), March 2009.
