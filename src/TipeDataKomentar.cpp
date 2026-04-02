#include "TipeDataKomentar.hpp"

InputBuffer::InputBuffer(std::istream &input)
    : in(input), hasBuffer(false), buffer(0) {}

char InputBuffer::get() {
    if (hasBuffer) {
        hasBuffer = false;
        return buffer;
    }
    return in.get();
}

void InputBuffer::put(char c) {
    hasBuffer = true;
    buffer = c;
}

std::string NumberHandler::process(InputBuffer &in, char firstChar) {
    std::string buffer;
    buffer += firstChar;

    bool isReal = false;
    bool validReal = false;

    char c;

    while (true) {
        c = in.get();

        if (std::isdigit(c)) {
            buffer += c;
            if (isReal) validReal = true;
        }
        else if (c == '.' && !isReal) {
            isReal = true;
            buffer += c;
        }
        else {
            in.put(c);
            break;
        }
    }

    if (isReal && !validReal) {
        in.put('.');
        buffer.pop_back();
        return "intcon(" + buffer + ")";
    }

    if (isReal) {
        return "realcon(" + buffer + ")";
    }

    return "intcon(" + buffer + ")";
}

std::string StringHandler::process(InputBuffer &in, char firstChar) {
    std::string buffer;
    buffer += firstChar;

    std::string content;

    char c;

    while (true) {
        c = in.get();

        if (c == EOF || c == '\n') {
            return "unknown(" + buffer + ")";
        }

        if (c == '\'') {
            char next = in.get();

            if (next == '\'') {
                content += '\'';
                buffer += "''";
            } else {
                in.put(next);
                buffer += '\'';
                break;
            }
        } else {
            content += c;
            buffer += c;
        }
    }

    if (content.length() == 1) {
        return "charcon(" + buffer + ")";
    }

    return "string(" + buffer + ")";
}

std::string CommentHandler::process(InputBuffer &in, char firstChar) {

    std::string content = "";

    if (firstChar == '{') {
        char c;

        while (true) {
            c = in.get();

            if (c == EOF) {
                return "unknown({" + content + ")";
            }

            if (c == '}') {
                return "comment(" + content + ")";
            }

            content += c;
        }
    }

    if (firstChar == '(') {
        char c = in.get();

        if (c != '*') {
            in.put(c);
            return "unknown(" + std::string(1, firstChar) + ")";
        }

        char prev = 0;
        char curr;

        while (true) {
            curr = in.get();

            if (curr == EOF) {
                return "unknown((*" + content + ")";
            }

            if (prev == '*' && curr == ')') {
                if (!content.empty()) content.pop_back();
                return "comment(" + content + ")";
            }

            content += curr;
            prev = curr;
        }
    }

    return "unknown(" + std::string(1, firstChar) + ")";
}