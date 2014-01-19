# accel [![Build Status](https://travis-ci.org/shalecraig/accel.png?branch=master)](https://travis-ci.org/shalecraig/accel)
---

#### What is accel?
Accel is a gesture recognition library written in C that uses accelerometer data to find gestures.
Though it is written with the Pebble in mind, it should work with other embedded devices as well.

## API Overview

### Return types

All methods in the accel api return a success value or error codes.

Return values are as follows:

- `ACCEL_SUCCESS` - the method being called is successful.
- `ACCEL_PARAM_ERROR` - parameters being passed are invalid.
- `ACCEL_MALLOC_ERROR` - returned when malloc fails
- `ACCEL_NO_VALID_GESTURE` - when gestures are queried, this value is returned when there are no applicable gestures.
- `ACCEL_INTERNAL_ERROR` - there is an internal error in the accel library, usually due to consistency issues in parameter attributes. It's best to report these.

## Using accel in Your Project

### Getting the accel Source Code
Getting the accel source is easy. Simply clone the github repository as follows:
```
git clone https://github.com/shalecraig/accel.git
```

### Using accel in Pebble Projects

Simply copy all files from the `src` directory in accel into the `src` directory in your pebble app. The `pebble build` command will find the newly added files, so everything should work smoothly.

```
cp accel/src/* myproject/src/
```

### Using accel in Other Projects

The source code is in the `src` directory. Simply include `accel.h` and make sure you compile all files. If you needed to make changes, feel free to submit pull requests to [the accel repo](https://github.com/shalecraig/accel).


### Current Version

The accel version is currently 1.0.0b.
This version is exposed in `accel.h` as the `ACCEL_VERSION_CODE` const, which is generated using the `ACCEL_VERSION_GEN` macro.


## Using Accel
## Using Accel
# Using Accel

### TODO:

- V1.1.0:
    - [ ] Decide on having callbacks instead of (or) `accel_find_most_likely_gesture`
        - Maybe needs a const for the values?
    - [ ] Implement a `public` reset gesture method

Attribution
=====

Gesture recognition adapted from:
- Jiayang Liu, Zhen Wang, Lin Zhong, Jehan Wickramasuriya, and Venu Vasudevan, "uWave: Accelerometer-based personalized gesture recognition and its applications," in Proc. IEEE Int. Conf. Pervasive Computing and Communication (PerCom), March 2009.
