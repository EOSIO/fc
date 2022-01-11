#pragma once

#define ENABLE_MODULE_RECOVERY 1

#define ECMULT_GEN_PREC_BITS 4
#define ECMULT_WINDOW_SIZE 15

//use internal field & num impls
#define USE_FIELD_INV_BUILTIN 1
#define USE_SCALAR_INV_BUILTIN 1
#define USE_NUM_NONE 1

//enable asm
#ifdef __x86_64__
  #define USE_ASM_X86_64 1
#endif
