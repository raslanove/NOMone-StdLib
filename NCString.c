
#include <NCString.h>
#include <NError.h>
#include <NSystemUtils.h>

static int32_t stringLength(const char* string) {
    int32_t length=0;
    while (string[length]) length++;
    return length;
}

static boolean startsWith(const char* string, const char* value) {

    int32_t index=0;
    while (string[index] && value[index]) {
        if (string[index] != value[index]) return False;
        index++;
    }

    // Check if loop ended because string ended while value didn't,
    if (value[index]) return False;

    // Oh well, they must be equals then :D
    return True;
}

// Returns last occurrence index, -1 if not found,
static int32_t lastIndexOf(const char* string, const char* value) {

#ifdef EXTRA_CHECKS
    if (!string || !value) {
        NERROR("NCString", "lastIndexOf(): string and value should be non-zero");
        return -1;
    }
#endif

    int32_t  textLength = stringLength(string);
    int32_t valueLength = stringLength( value);
    textLength -= valueLength;
    while (textLength >= 0) {
        if (startsWith(&string[textLength], value)) return textLength;
        textLength--;
    }

    return -1; // Not found.
}

static boolean equals(const char* string, const char* value) {

    int32_t index=0;
    while (string[index] && value[index]) {
        if (string[index] != value[index]) return False;
        index++;
    }

    // Check if the loop ended while either of the two strings didn't reach the end yet,
    if (string[index] || value[index]) return False;

    // Yep, they are equal :)
    return True;
}

static char* copy(char* destination, const char* source) {

    int32_t index=0;
    char currentChar;
    do {
        currentChar = source[index];
        destination[index] = currentChar;
        index++;
    } while (currentChar);

    return destination;
}

static char* clone(const char* source) {
    int32_t length = stringLength(source)+1;
    char *newCopy = NMALLOC(length, "NCString.clone() newCopy");
    NSystemUtils.memcpy(newCopy, source, length);
    return newCopy;
}

static int32_t parseInteger(const char* string) {

    // Max: 2,147,483,647
    int32_t integerLength = stringLength(string);
    if (integerLength > 11) {
        NERROR("NCString.parseInteger()", "Integer length can't exceed 10 digits and a sign. Found: %s%s", NTCOLOR(HIGHLIGHT), string);
        return 0;
    }

    // Parse,
    // Sign,
    int32_t currentDigitMagnitude = 1;
    int32_t offset=0;
    if (string[0] == '-') {
        offset = 1;
        integerLength--;
        currentDigitMagnitude = -1;
    }

    // Value,
    int32_t value=0;
    int32_t lastValue=0;
    char currentChar;
    while (integerLength--) {
        currentChar = string[offset + integerLength] - 48;
        if ((currentChar<0) || (currentChar>9)) {
            NERROR("NCString.parseInteger()", "Only digits from 0 to 9 are allowed. Found: %s%s", NTCOLOR(HIGHLIGHT), string);
            return 0;
        }
        value += currentDigitMagnitude * (int32_t) currentChar;
        currentDigitMagnitude *= 10;

        // Check overflow,
        if (offset) { // Negative.
            if (lastValue < value) {
                NERROR("NCString.parseInteger()", "Value too small to fit in a 32 bit integer: %s%s", NTCOLOR(HIGHLIGHT), string);
                return 0;
            }
        } else {
            if (lastValue > value) {
                NERROR("NCString.parseInteger()", "Value too large to fit in a 32 bit integer: %s%s", NTCOLOR(HIGHLIGHT), string);
                return 0;
            }
        }
        lastValue = value;
    }

    return value;
}

static int64_t parse64BitInteger(const char* string) {

    // Max: 9,223,372,036,854,775,807
    int32_t integerLength = stringLength(string);
    if (integerLength > 20) {
        NERROR("NCString.parse64BitInteger()", "64bit Integer length can't exceed 19 digits and a sign. Found: %s%s", NTCOLOR(HIGHLIGHT), string);
        return 0;
    }

    // Parse,
    // Sign,
    int64_t currentDigitMagnitude = 1;
    int32_t offset=0;
    if (string[0] == '-') {
        offset = 1;
        integerLength--;
        currentDigitMagnitude = -1;
    }

    // Value,
    int64_t value=0;
    int64_t lastValue=0;
    char currentChar;
    while (integerLength--) {
        currentChar = string[offset + integerLength] - 48;
        if ((currentChar<0) || (currentChar>9)) {
            NERROR("NCString.parse64BitInteger()", "Only digits from 0 to 9 are allowed. Found: %s%s", NTCOLOR(HIGHLIGHT), string);
            return 0;
        }
        value += currentDigitMagnitude * (int64_t) currentChar;
        currentDigitMagnitude *= 10;

        // Check overflow,
        if (offset) { // Negative.
            if (lastValue < value) {
                NERROR("NCString.parse64BitInteger()", "Value too small to fit in a 64 bit integer: %s%s", NTCOLOR(HIGHLIGHT), string);
                return 0;
            }
        } else {
            if (lastValue > value) {
                NERROR("NCString.parse64BitInteger()", "Value too large to fit in a 64 bit integer: %s%s", NTCOLOR(HIGHLIGHT), string);
                return 0;
            }
        }
        lastValue = value;
    }

    return value;
}

const struct NCString_Interface NCString = {
    .length = stringLength,
    .startsWith = startsWith,
    .lastIndexOf = lastIndexOf,
    .equals = equals,
    .copy = copy,
    .clone = clone,
    .parseInteger = parseInteger,
    .parse64BitInteger = parse64BitInteger
};
