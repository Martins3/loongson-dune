#ifndef AUX_H_K25YP6NF
#define AUX_H_K25YP6NF

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

void pr_warn(const char *err, ...);
void pr_info(const char *info, ...);
void die(const char *err, ...);

#endif /* end of include guard: AUX_H_K25YP6NF */
