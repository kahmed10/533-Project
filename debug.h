// 1 = enable extra checks and error messages
// else = maximum performance
#define DEBUG 1

#if DEBUG == 1
    #define check(condition, message) \
        if (!(condition)) \
        { \
            fprintf \
            ( \
                stderr, \
                "Error in %s line %u ~ %s\n", \
                __FILE__, \
                __LINE__, \
                message \
            ); \
        }
#else
    #define check(condition, message) {}
#endif
