#include "../shared/buffer.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

static int passed = 0;
static int failed = 0;

#define RUN_TEST(name) \
    do { \
        try { \
            name(); \
            std::cout << "[PASS] " #name "\n"; \
            ++passed; \
        } catch (const std::exception& e) { \
            std::cout << "[FAIL] " #name ": " << e.what() << "\n"; \
            ++failed; \
        } catch (...) { \
            std::cout << "[FAIL] " #name ": unknown exception\n"; \
            ++failed; \
        } \
    } while (0)

// ---- constructors ----

void test_default_constructor() {
    Buffer b;
    assert(b.size() == 0);
    assert(b.offset == 0);
}

void test_vector_constructor() {
    std::vector<uint8_t> v = {0x01, 0x02, 0x03};
    Buffer b(v);
    assert(b.size() == 3);
    assert(b[0] == 0x01);
    assert(b[1] == 0x02);
    assert(b[2] == 0x03);
}

void test_copy_constructor() {
    Buffer a;
    a.addByte(0xAB);
    a.offset = 1;
    Buffer b(a);
    assert(b.size() == 1);
    assert(b[0] == 0xAB);
    assert(b.offset == 1);
}

// ---- addByte / readByte ----

void test_byte_roundtrip() {
    Buffer b;
    b.addByte(0x42);
    assert(b.size() == 1);
    assert(b.readByte() == 0x42);
    assert(b.offset == 1);
}

void test_byte_zero() {
    Buffer b;
    b.addByte(0x00);
    assert(b.readByte() == 0x00);
}

void test_byte_max() {
    Buffer b;
    b.addByte(0xFF);
    assert(b.readByte() == 0xFF);
}

// ---- addTwoByte / readTwoByte ----

void test_two_byte_roundtrip() {
    Buffer b;
    b.addTwoByte(0x1234);
    assert(b.size() == 2);
    // little-endian: [0x34, 0x12]
    assert(b[0] == 0x34);
    assert(b[1] == 0x12);
    assert(b.readTwoByte() == 0x1234);
    assert(b.offset == 2);
}

void test_two_byte_zero() {
    Buffer b;
    b.addTwoByte(0x0000);
    assert(b.readTwoByte() == 0x0000);
}

void test_two_byte_max() {
    Buffer b;
    b.addTwoByte(0xFFFF);
    assert(b.readTwoByte() == 0xFFFF);
}

// ---- addFourByte / readFourByte ----

void test_four_byte_roundtrip() {
    Buffer b;
    b.addFourByte(0xDEADBEEF);
    assert(b.size() == 4);
    // little-endian: [0xEF, 0xBE, 0xAD, 0xDE]
    assert(b[0] == 0xEF);
    assert(b[1] == 0xBE);
    assert(b[2] == 0xAD);
    assert(b[3] == 0xDE);
    assert(b.readFourByte() == 0xDEADBEEF);
    assert(b.offset == 4);
}

void test_four_byte_zero() {
    Buffer b;
    b.addFourByte(0x00000000);
    assert(b.readFourByte() == 0x00000000);
}

void test_four_byte_max() {
    Buffer b;
    b.addFourByte(0xFFFFFFFF);
    assert(b.readFourByte() == 0xFFFFFFFF);
}

// ---- addEightByte / readEightByte ----

void test_eight_byte_roundtrip() {
    Buffer b;
    b.addEightByte(0xCAFEBABEDEADBEEF);
    assert(b.size() == 8);
    // little-endian: LSB first
    assert(b[0] == 0xEF);
    assert(b[7] == 0xCA);
    assert(b.readEightByte() == 0xCAFEBABEDEADBEEF);
    assert(b.offset == 8);
}

void test_eight_byte_zero() {
    Buffer b;
    b.addEightByte(0x0000000000000000ULL);
    assert(b.readEightByte() == 0x0000000000000000ULL);
}

void test_eight_byte_max() {
    Buffer b;
    b.addEightByte(0xFFFFFFFFFFFFFFFFULL);
    assert(b.readEightByte() == 0xFFFFFFFFFFFFFFFFULL);
}

// ---- addString / readString ----

void test_string_roundtrip() {
    Buffer b;
    b.addString("hello");
    // 4-byte length prefix + 5 chars
    assert(b.size() == 9);
    assert(b.readString() == "hello");
    assert(b.offset == 9);
}

void test_string_empty() {
    Buffer b;
    b.addString("");
    assert(b.size() == 4);
    assert(b.readString() == "");
}

void test_string_with_spaces() {
    Buffer b;
    b.addString("hello world");
    assert(b.readString() == "hello world");
}

// ---- getData ----

void test_get_data() {
    Buffer b;
    b.addByte(0x01);
    b.addByte(0x02);
    auto& d = b.getData();
    assert(d.size() == 2);
    assert(d[0] == 0x01);
    assert(d[1] == 0x02);
}

// ---- operator[] ----

void test_index_operator_valid() {
    Buffer b;
    b.addByte(0x55);
    assert(b[0] == 0x55);
}

void test_index_operator_out_of_bounds() {
    Buffer b;
    b.addByte(0x55);
    bool threw = false;
    try {
        (void)b[1];
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
}

// ---- sequential reads (offset tracking) ----

void test_sequential_reads() {
    Buffer b;
    b.addByte(0x01);
    b.addTwoByte(0x0203);
    b.addFourByte(0x04050607);
    b.addEightByte(0x08090A0B0C0D0E0FULL);

    assert(b.readByte()      == 0x01);
    assert(b.readTwoByte()   == 0x0203);
    assert(b.readFourByte()  == 0x04050607);
    assert(b.readEightByte() == 0x08090A0B0C0D0E0FULL);
    assert(b.offset == 15);
}

void test_sequential_string_reads() {
    Buffer b;
    b.addString("foo");
    b.addString("bar");
    assert(b.readString() == "foo");
    assert(b.readString() == "bar");
}

// ---- main ----

int main() {
    RUN_TEST(test_default_constructor);
    RUN_TEST(test_vector_constructor);
    RUN_TEST(test_copy_constructor);

    RUN_TEST(test_byte_roundtrip);
    RUN_TEST(test_byte_zero);
    RUN_TEST(test_byte_max);

    RUN_TEST(test_two_byte_roundtrip);
    RUN_TEST(test_two_byte_zero);
    RUN_TEST(test_two_byte_max);

    RUN_TEST(test_four_byte_roundtrip);
    RUN_TEST(test_four_byte_zero);
    RUN_TEST(test_four_byte_max);

    RUN_TEST(test_eight_byte_roundtrip);
    RUN_TEST(test_eight_byte_zero);
    RUN_TEST(test_eight_byte_max);

    RUN_TEST(test_string_roundtrip);
    RUN_TEST(test_string_empty);
    RUN_TEST(test_string_with_spaces);

    RUN_TEST(test_get_data);

    RUN_TEST(test_index_operator_valid);
    RUN_TEST(test_index_operator_out_of_bounds);

    RUN_TEST(test_sequential_reads);
    RUN_TEST(test_sequential_string_reads);

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
