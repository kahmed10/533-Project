// 1 = enable extra checks and error messages
// else = maximum performance
#define DEBUG 0

#if DEBUG == 1
    #define check(condition, message) \
        if (!(condition)) \
        { \
            std::cerr \
                << "Error in " \
                << __FILE__ \
                << " line " \
                << __LINE__ \
                << " ~ " \
                << message \
                << std::endl; \
        }
#else
    #define check(condition, message) {}
#endif

