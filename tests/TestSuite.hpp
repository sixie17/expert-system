#pragma once
#include <exception>
#include <functional>
#include <iostream>
#include <string>

/**
 * @brief Minimal test suite: records checks, prints failures and a summary.
 *
 * @example
 *   TestSuite suite("Parser");
 *   suite.expectEqual("simple rule", std::string("(A => B)"), actual);
 *   suite.expectThrow<MissingOperationValue>("missing operand", [] { ... });
 *   return suite.summary();
 */
class TestSuite {
    private:
        const std::string name;
        size_t passed = 0;
        size_t failed = 0;
        const bool verbose;

        void pass(const std::string &description, const std::string &details = "") {
            this->passed++;
            if (!this->verbose)
                return;
            std::cout << "  PASS: " << description;
            if (!details.empty())
                std::cout << " -> " << details;
            std::cout << std::endl;
        }

        void fail(const std::string &description, const std::string &details) {
            this->failed++;
            std::cout << "  FAIL: " << description << std::endl
                      << "        " << details << std::endl;
        }

    public:
        /**
         * @param verbose If true, passing checks are printed too, not only failures.
         */
        explicit TestSuite(const std::string &name, bool verbose = true) : name(name), verbose(verbose) {
            std::cout << "=== " << name << " ===" << std::endl;
        }

        /**
         * @brief Passes if condition is true.
         */
        void check(const std::string &description, bool condition,
                   const std::string &details = "condition was false") {
            if (condition)
                pass(description);
            else
                fail(description, details);
        }

        /**
         * @brief Passes if expected == actual, prints both otherwise.
         */
        template <typename T>
        void expectEqual(const std::string &description, const T &expected, const T &actual) {
            if (expected == actual)
                pass(description, toString(actual));
            else
                fail(description, "expected: " + toString(expected) + " | actual: " + toString(actual));
        }

        /**
         * @brief Passes if fn throws exactly an exception of type E (or derived).
         */
        template <typename E>
        void expectThrow(const std::string &description, const std::function<void()> &fn) {
            try {
                fn();
                fail(description, "expected an exception, nothing was thrown");
            } catch (const E &error) {
                pass(description, std::string("threw: ") + error.what());
            } catch (const std::exception &error) {
                fail(description, std::string("wrong exception thrown: ") + error.what());
            }
        }

        /**
         * @brief Passes if fn does not throw.
         */
        void expectNoThrow(const std::string &description, const std::function<void()> &fn) {
            try {
                fn();
                pass(description);
            } catch (const std::exception &error) {
                fail(description, std::string("unexpected exception: ") + error.what());
            }
        }

        /**
         * @brief Prints the summary.
         * @return Exit code: 0 if every check passed, 1 otherwise.
         */
        int summary() const {
            std::cout << this->name << ": " << this->passed << " passed, "
                      << this->failed << " failed" << std::endl;
            return this->failed == 0 ? 0 : 1;
        }

    private:
        static std::string toString(const std::string &value) {
            return "\"" + value + "\"";
        }

        template <typename T>
        static std::string toString(const T &value) {
            return std::to_string(value);
        }
};
