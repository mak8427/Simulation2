#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace test {

inline std::vector<std::pair<std::string, std::function<void()>>>& registry() {
    static std::vector<std::pair<std::string, std::function<void()>>> reg;
    return reg;
}

inline int register_test(const std::string& name, std::function<void()> fn) {
    registry().emplace_back(name, std::move(fn));
    return 0;
}

inline void fail(const std::string& msg, const char* file, int line) {
    std::cerr << "  " << file << ":" << line << ": " << msg << std::endl;
}

} // namespace test

#define TEST_UTILS_CONCAT_IMPL(x, y) x##y
#define TEST_UTILS_CONCAT(x, y) TEST_UTILS_CONCAT_IMPL(x, y)

#define TEST(name)                                                        \
    static void test_##name();                                            \
    static int TEST_UTILS_CONCAT(test_##name##_reg_, __COUNTER__) =       \
        test::register_test(#name, test_##name);                          \
    static void test_##name()

#define EXPECT_EQ(a, b)                                                          \
    do {                                                                         \
        auto _e_a = (a);                                                         \
        auto _e_b = (b);                                                         \
        if (!(_e_a == _e_b)) {                                                   \
            std::ostringstream _oss;                                              \
            _oss << "EXPECT_EQ failed: " << #a << " != " << #b                    \
                 << " (" << _e_a << " vs " << _e_b << ")";                        \
            test::fail(_oss.str(), __FILE__, __LINE__);                           \
            throw std::runtime_error("EXPECT_EQ");                                \
        }                                                                        \
    } while (false)

#define EXPECT_FLOAT_EQ(a, b, epsilon)                                           \
    do {                                                                         \
        auto _f_a = (a);                                                         \
        auto _f_b = (b);                                                         \
        auto _f_eps = (epsilon);                                                 \
        if (std::fabs(_f_a - _f_b) > _f_eps) {                                   \
            std::ostringstream _oss;                                              \
            _oss << "EXPECT_FLOAT_EQ failed: " << #a << " != " << #b              \
                 << " (" << _f_a << " vs " << _f_b << ", eps=" << _f_eps << ")";  \
            test::fail(_oss.str(), __FILE__, __LINE__);                           \
            throw std::runtime_error("EXPECT_FLOAT_EQ");                          \
        }                                                                        \
    } while (false)

#define EXPECT_TRUE(cond)                                                        \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::ostringstream _oss;                                              \
            _oss << "EXPECT_TRUE failed: " << #cond << " is false";               \
            test::fail(_oss.str(), __FILE__, __LINE__);                           \
            throw std::runtime_error("EXPECT_TRUE");                              \
        }                                                                        \
    } while (false)

#define EXPECT_FALSE(cond)                                                       \
    do {                                                                         \
        if ((cond)) {                                                            \
            std::ostringstream _oss;                                              \
            _oss << "EXPECT_FALSE failed: " << #cond << " is true";               \
            test::fail(_oss.str(), __FILE__, __LINE__);                           \
            throw std::runtime_error("EXPECT_FALSE");                             \
        }                                                                        \
    } while (false)

#define EXPECT_GT(a, b)                                                          \
    do {                                                                         \
        auto _g_a = (a);                                                         \
        auto _g_b = (b);                                                         \
        if (!(_g_a > _g_b)) {                                                    \
            std::ostringstream _oss;                                              \
            _oss << "EXPECT_GT failed: " << #a << " > " << #b                     \
                 << " (" << _g_a << " > " << _g_b << ")";                         \
            test::fail(_oss.str(), __FILE__, __LINE__);                           \
            throw std::runtime_error("EXPECT_GT");                                \
        }                                                                        \
    } while (false)

#define EXPECT_LT(a, b)                                                          \
    do {                                                                         \
        auto _l_a = (a);                                                         \
        auto _l_b = (b);                                                         \
        if (!(_l_a < _l_b)) {                                                    \
            std::ostringstream _oss;                                              \
            _oss << "EXPECT_LT failed: " << #a << " < " << #b                     \
                 << " (" << _l_a << " < " << _l_b << ")";                         \
            test::fail(_oss.str(), __FILE__, __LINE__);                           \
            throw std::runtime_error("EXPECT_LT");                                \
        }                                                                        \
    } while (false)

#define EXPECT_GE(a, b)                                                          \
    do {                                                                         \
        auto _g_a = (a);                                                         \
        auto _g_b = (b);                                                         \
        if (!(_g_a >= _g_b)) {                                                   \
            std::ostringstream _oss;                                              \
            _oss << "EXPECT_GE failed: " << #a << " >= " << #b                    \
                 << " (" << _g_a << " >= " << _g_b << ")";                        \
            test::fail(_oss.str(), __FILE__, __LINE__);                           \
            throw std::runtime_error("EXPECT_GE");                                \
        }                                                                        \
    } while (false)

#define RUN_ALL_TESTS()                                                          \
    do {                                                                         \
        int _failures = 0;                                                       \
        int _passed = 0;                                                         \
        std::cout << "Running " << test::registry().size() << " test(s)...\n";   \
        std::cout << "========================================\n";               \
        for (auto& _t : test::registry()) {                                      \
            try {                                                                \
                _t.second();                                                     \
                std::cout << "[PASS] " << _t.first << "\n";                      \
                ++_passed;                                                       \
            } catch (...) {                                                      \
                std::cout << "[FAIL] " << _t.first << "\n";                      \
                ++_failures;                                                     \
            }                                                                    \
        }                                                                        \
        std::cout << "========================================\n";               \
        std::cout << _passed << " passed, " << _failures << " failed\n";         \
        if (_failures > 0) return _failures;                                     \
    } while (false)

#endif // TEST_UTILS_H
