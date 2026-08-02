// UnitTestMain
// -----------------------------------------------------------------------------
// Console runner for all USAM unit tests. Returns 0 on success, 1 on any
// failure — suitable for CI and local dev.
#include <juce_core/juce_core.h>

// Logger that writes to stdout so test output is visible on the console and
// in CI logs (the default Logger is a no-op in console apps).
struct ConsoleLogger : juce::Logger
{
    void logMessage (const juce::String& message) override
    {
        std::cout << message << std::endl;
    }
};

int main()
{
    ConsoleLogger logger;
    juce::Logger::setCurrentLogger (&logger);

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runAllTests();

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        const auto* result = runner.getResult (i);
        if (result != nullptr && result->failures > 0)
            ++failures;
    }

    juce::Logger::setCurrentLogger (nullptr);

    if (failures == 0)
    {
        std::cout << "ALL USAM UNIT TESTS PASSED" << std::endl;
        return 0;
    }

    std::cout << failures << " USAM UNIT TEST FAILURES" << std::endl;
    return 1;
}
