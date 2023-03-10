/**
 * @file test_va.c
 * @author IotaHydrae (writeforever@foxmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-03-10
 * 
 * MIT License
 * 
 * Copyright 2022 IotaHydrae(writeforever@foxmail.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * 
 */

#include <stdio.h>
#include <stdarg.h>


#define NUMARGS(...)  (sizeof((int[]){__VA_ARGS__}) / sizeof(int))

static unsigned char buf[128];

int foo(char *head, int len, ...)
{
    va_list args;
    
    va_start(args, len);
    for (int i =0; i< len; i++) {
        buf[i] = va_arg(args, unsigned int);
        printf("0x%02x\n", buf[i]);
    }

    va_end(args);

    return 0;
}
#define test_foo(head, ...) \
        foo(head, NUMARGS(__VA_ARGS__), __VA_ARGS__)

int main(int argc, char **argv)
{
    test_foo("hello", 0x12, 0x34, 0x56, 0x78);

    return 0;
}