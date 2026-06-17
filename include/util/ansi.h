#pragma once

namespace ansi {

    constexpr const char* reset            = "\033[0m";
    constexpr const char* bold             = "\033[1m";
    constexpr const char* dim              = "\033[2m";
    constexpr const char* italic           = "\033[3m";
    constexpr const char* underline        = "\033[4m";
    constexpr const char* blink            = "\033[5m";
    constexpr const char* rapidBlink       = "\033[6m";
    constexpr const char* reverse          = "\033[7m";
    constexpr const char* hidden           = "\033[8m";
    constexpr const char* strikethrough    = "\033[9m";
    
    constexpr const char* dimOff           = "\033[22m";
    constexpr const char* italicOff        = "\033[23m";
    constexpr const char* underlineOff     = "\033[24m";
    constexpr const char* blinkOff         = "\033[25m";
    constexpr const char* reverseOff       = "\033[27m";
    constexpr const char* hiddenOff        = "\033[28m";
    constexpr const char* strikeOff        = "\033[29m";

    constexpr const char* black            = "\033[30m";
    constexpr const char* red              = "\033[31m";
    constexpr const char* green            = "\033[32m";
    constexpr const char* yellow           = "\033[33m";
    constexpr const char* blue             = "\033[34m";
    constexpr const char* magenta          = "\033[35m";
    constexpr const char* cyan             = "\033[36m";
    constexpr const char* white            = "\033[37m";

    constexpr const char* bgBlack          = "\033[40m";
    constexpr const char* bgRed            = "\033[41m";
    constexpr const char* bgGreen          = "\033[42m";
    constexpr const char* bgYellow         = "\033[43m";
    constexpr const char* bgBlue           = "\033[44m";
    constexpr const char* bgMagenta        = "\033[45m";
    constexpr const char* bgCyan           = "\033[46m";
    constexpr const char* bgWhite          = "\033[47m";

    constexpr const char* ctopleft         = "\033[H";
    constexpr const char* cclear           = "\033[2J";
    constexpr const char* cclearline       = "\033[K";
    constexpr const char* csave            = "\033[s";
    constexpr const char* crestore         = "\033[u";
    constexpr const char* chide            = "\033[?25l";
    constexpr const char* cshow            = "\033[?25h";

}