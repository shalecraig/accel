accel [![Build Status](https://travis-ci.org/shalecraig/accel.png?branch=master)](https://travis-ci.org/shalecraig/accel)
=====

[alpha] C gesture recognition library from accelerometers, written with the Pebble in mind. Currently version 0.1.0a.


### TODO:

- V0.2.0a:
    - [ ] Decide on having callbacks instead of (or) `accel_find_most_likely_gesture`
        - Maybe needs a const for the values?
    - [ ] Implement a `public` reset gesture method

Attribution
=====

Gesture recognition adapted from:
- Jiayang Liu, Zhen Wang, Lin Zhong, Jehan Wickramasuriya, and Venu Vasudevan, "uWave: Accelerometer-based personalized gesture recognition and its applications," in Proc. IEEE Int. Conf. Pervasive Computing and Communication (PerCom), March 2009.
