#ifndef TIPE_DATA_KOMENTAR_HPP
#define TIPE_DATA_KOMENTAR_HPP

#include <iostream>
#include <string>

struct InputBuffer {
    std::istream &in;
    bool hasBuffer;
    char buffer;

    InputBuffer(std::istream &input);

    char get();
    void put(char c);
};

class NumberHandler {
public:
    std::string process(InputBuffer &in, char firstChar);
};

class StringHandler {
public:
    std::string process(InputBuffer &in, char firstChar);
};

class CommentHandler {
public:
    std::string process(InputBuffer &in, char firstChar);
};

#endif