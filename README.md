accel
=====

[not-even-alpha] C gesture recognition library from accelerometers, written with the Pebble in mind.


### TODO:


- V0.1.0a:
    - [x] Create tests.
    - [x] Move global variables into a struct.
    - [x] Record and find multiple movements, not just one.
    - [x] Clean up Headers.
    - [x] Rearrange structs to hide implementation.
    - [x] Beef up unit tests with unusual input.
    - [ ] Write sample app that uses this.
    - [x] Provide #define version number. [`ACCEL_VERSION_CODE`]
- V1.1.0:
    - [ ] Decide on having callbacks instead of (or) `accel_find_most_likely_gesture`
        - Maybe needs a const for the values?
    - [ ] Implement a `public` reset gesture method
    - [ ] Use the `-DPEBBLE` flag to conditionally load pebble libraries.
        - [ ] Define `realloc`, since it's not defined in pebble.h (ew)
        - [ ] Define `calloc`, since it's not defined in pebble.h (ew)
        - [ ] Define `cbrt`, since it's not defined in pebble.h (ew)

Attribution
=====

Gesture recognition adapted from:
- Jiayang Liu, Zhen Wang, Lin Zhong, Jehan Wickramasuriya, and Venu Vasudevan, "uWave: Accelerometer-based personalized gesture recognition and its applications," in Proc. IEEE Int. Conf. Pervasive Computing and Communication (PerCom), March 2009.
