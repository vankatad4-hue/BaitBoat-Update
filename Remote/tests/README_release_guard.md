# Release guard test
Run: `g++ -std=c++17 -Wall -Wextra -Werror -IRemote/tests/stubs -IRemote/source Remote/source/release_button_guard.cpp Remote/tests/release_guard_unit_test.cpp -o release_guard_test && ./release_guard_test`. Tests timing, confirmation, timeout and backup link rejection. Hardware scanning/interlocks are not covered.
