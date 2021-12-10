#pragma once

#define ENABLE_MODULE_RECOVERY 1

#define ECMULT_GEN_PREC_BITS 4
#define ECMULT_WINDOW_SIZE 15

//use GMP for bignum
#define HAVE_LIBGMP 1
#define USE_NUM_GMP 1
#define USE_FIELD_INV_NUM 1
#define USE_SCALAR_INV_NUM 1

//enable asm
#ifdef __x86_64__
  #define USE_ASM_X86_64 1
#endif
