/* 	ya-dconvert.c
	=============
	
   Support functions for double->string (used by ya_sprintf) and string->double (used by fast_strtod()).
   They offer fast "round the loop" exact conversions, and share a common "powers of 10" array of constants to minimise the "object code size".
   Where native unsigned int128's don't exist then u2_64 is leveraged to provide 128 bit unsigned integers using a pair of uint64_t's
   
   Written by Peter Miller 11-3-2026
   
   This version uses the fpfmt algorithms as described in https://research.swtch.com/fp and https://research.swtch.com/fp-proof 
   
   April 2026 - four directly callable functions added (previoulsy only provided support functions for ya_sprintf() :
 	 void ya_dconvert_efmt(char *dst, double f, uint32_t prec) ; // prec is required number of digits after decimal point - this is equivalent to (but faster than) sprintf(dst,"%.*e",prec,f)
	 void ya_dconvert_gfmt(char *dst, double f, int32_t prec);  // prec is required number of digits - this is equivalent to (but faster than) sprintf(dst,"%.*g",prec,f)   
     void ya_shortd(char *dst, double f) ;// gives the shortest (smallest number of characters) representation of double f that is round trip exact
     void ya_shortf(char *dst, float f) ;// gives the shortest (smallest number of characters) representation of float f that is round trip exact
*/
/*----------------------------------------------------------------------------
 * MIT License:
 *
 * Copyright (c) 2026 Peter Miller
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *--------------------------------------------------------------------------*/


#include <stdlib.h> 
#include <assert.h>   // assert
#include <float.h>    // DBL_MANT_DIG
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <stdint.h>   // uint64_t
#include <string.h>   // memcpy
#include <stdalign.h> // for aligns() - this has been available since C11

#include "../u2_64-128bits-with-two-u64/u2_64.h"
#include "ya-dconvert.h"

#ifndef DBL_DECIMAL_DIG // fill in missing definition of DBL_DECIMAL_DIG
 #define DBL_DECIMAL_DIG __DBL_DECIMAL_DIG__
#endif

#if defined(_M_AMD64) || defined(_M_IX86) // "Intel" 32 or 64 bits - note "64" or "128" functions need a 64 bit processor
 #include <intrin.h>  // __lzcnt64/_umul128/__umulh
#endif

/* if compiler supports an unsigned 128 integer type then we use that for the 128*64 bit multiply required below, otherwise we use u2_64 to give a "software" unsigned 128 bit type. */ 
#if defined(__SIZEOF_INT128__)
// Computes high 64 bits of multiplication of x and y,
// lsb of result is effectively a "sticky bit", its 0 only if lower 65 bits of (128 bit) [apart from lsb  of 128] result are all 0
static inline uint64_t umulhi_stickybit(uint64_t x_hi, uint64_t x_lo, uint64_t y) 
{
  unsigned __int128 p = (unsigned __int128)x_hi* y;
  uint64_t lo = (uint64_t)(p) + (uint64_t)(((unsigned __int128)x_lo* y)>>64);
  uint64_t hi=(uint64_t)(p>>64)+(lo<(uint64_t)p);// lo<(uint64_t)(p) is carry from addition in line above
  return hi| ((lo >> 1) != 0);
}
// top 64 bits of 64*64 bit multiply - used in div1e8(), div10() and udiv10() below
static inline uint64_t umul128_hi64(uint64_t x, uint64_t y) 
{
  return ((unsigned __int128)x*y)>>64;
}
#else
/* 128 bit unsigned type NOT available within the compiler, so use u2_64 */
// Computes high 64 bits of multiplication of x and y,
// lsb of result is effectively a "sticky bit", its 0 only if lower 65 bits of (128 bit) [apart from lsb  of 128] result are all 0
static inline uint64_t umulhi_stickybit(uint64_t x_hi, uint64_t x_lo, uint64_t y) 
{
  u2_64 p = u2_64_mult_u64_u64(x_hi, y);
  uint64_t lo = p.lo + u2_64_mult_u64_u64(x_lo, y).hi;
  uint64_t hi = p.hi + (lo < p.lo);// lo<p.lo is carry from addition in line above
  return hi | ((lo >> 1) != 0);
}
// top 64 bits of 64*64 bit multiply - used in div1e8(), div10() and udiv10() below
static inline uint64_t umul128_hi64(uint64_t x, uint64_t y) 
{
  return u2_64_mult_u64_u64(x, y).hi;
}

#endif // defined(__SIZEOF_INT128__)

// characteristics of ieee format doubles
#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11
#define DOUBLE_EXPONENT_BIAS 1023
#define DOUBLE_EXPONENT_OFFSET (DOUBLE_EXPONENT_BIAS+DOUBLE_MANTISSA_BITS)
#define DOUBLE_IMPLICIT_BIT ((uint64_t)1<< DOUBLE_MANTISSA_BITS) 

// characteristics of ieee format floats
#define FLOAT_MANTISSA_BITS 23
#define FLOAT_EXPONENT_BITS 8
#define FLOAT_EXPONENT_BIAS 127
#define FLOAT_EXPONENT_OFFSET (FLOAT_EXPONENT_BIAS+FLOAT_MANTISSA_BITS)
#define FLOAT_IMPLICIT_BIT ((uint32_t)1<< FLOAT_MANTISSA_BITS) 

// 128-bit significands of powers of 10 - normalised so the most significant bit is 1 and truncated at 128 bits . Covers -350 to +350 which is adequate for both double->string and string->double conversion
#define dec_exp_min  (-350)
#define dec_exp_max  (350)
alignas(64) const u2_64 u2_64_pow10_table[] = {
	{0xa05c0dd70f6e1619, 0xa8726bc8d55cbb16}, // -350
	{0xc873114cd3499ba0, 0x128f06bb0ab3e9dc}, // -349
	{0xfa8fd5a0081c0288, 0x1732c869cd60e453}, // -348
	{0x9c99e58405118195, 0x0e7fbd42205c8eb4}, // -347
	{0xc3c05ee50655e1fa, 0x521fac92a873b261}, // -346
	{0xf4b0769e47eb5a78, 0xe6a797b752909ef9}, // -345
	{0x98ee4a22ecf3188b, 0x9028bed2939a635c}, // -344
	{0xbf29dcaba82fdeae, 0x7432ee873880fc33}, // -343
	{0xeef453d6923bd65a, 0x113faa2906a13b3f}, // -342
	{0x9558b4661b6565f8, 0x4ac7ca59a424c507}, // -341
	{0xbaaee17fa23ebf76, 0x5d79bcf00d2df649}, // -340
	{0xe95a99df8ace6f53, 0xf4d82c2c107973dc}, // -339
	{0x91d8a02bb6c10594, 0x79071b9b8a4be869}, // -338
	{0xb64ec836a47146f9, 0x9748e2826cdee284}, // -337
	{0xe3e27a444d8d98b7, 0xfd1b1b2308169b25}, // -336
	{0x8e6d8c6ab0787f72, 0xfe30f0f5e50e20f7}, // -335
	{0xb208ef855c969f4f, 0xbdbd2d335e51a935}, // -334
	{0xde8b2b66b3bc4723, 0xad2c788035e61382}, // -333
	{0x8b16fb203055ac76, 0x4c3bcb5021afcc31}, // -332
	{0xaddcb9e83c6b1793, 0xdf4abe242a1bbf3d}, // -331
	{0xd953e8624b85dd78, 0xd71d6dad34a2af0d}, // -330
	{0x87d4713d6f33aa6b, 0x8672648c40e5ad68}, // -329
	{0xa9c98d8ccb009506, 0x680efdaf511f18c2}, // -328
	{0xd43bf0effdc0ba48, 0x0212bd1b2566def2}, // -327
	{0x84a57695fe98746d, 0x014bb630f7604b57}, // -326
	{0xa5ced43b7e3e9188, 0x419ea3bd35385e2d}, // -325
	{0xcf42894a5dce35ea, 0x52064cac828675b9}, // -324
	{0x818995ce7aa0e1b2, 0x7343efebd1940993}, // -323
	{0xa1ebfb4219491a1f, 0x1014ebe6c5f90bf8}, // -322
	{0xca66fa129f9b60a6, 0xd41a26e077774ef6}, // -321
	{0xfd00b897478238d0, 0x8920b098955522b4}, // -320
	{0x9e20735e8cb16382, 0x55b46e5f5d5535b0}, // -319
	{0xc5a890362fddbc62, 0xeb2189f734aa831d}, // -318
	{0xf712b443bbd52b7b, 0xa5e9ec7501d523e4}, // -317
	{0x9a6bb0aa55653b2d, 0x47b233c92125366e}, // -316
	{0xc1069cd4eabe89f8, 0x999ec0bb696e840a}, // -315
	{0xf148440a256e2c76, 0xc00670ea43ca250d}, // -314
	{0x96cd2a865764dbca, 0x380406926a5e5728}, // -313
	{0xbc807527ed3e12bc, 0xc605083704f5ecf2}, // -312
	{0xeba09271e88d976b, 0xf7864a44c633682e}, // -311
	{0x93445b8731587ea3, 0x7ab3ee6afbe0211d}, // -310
	{0xb8157268fdae9e4c, 0x5960ea05bad82964}, // -309
	{0xe61acf033d1a45df, 0x6fb92487298e33bd}, // -308
	{0x8fd0c16206306bab, 0xa5d3b6d479f8e056}, // -307
	{0xb3c4f1ba87bc8696, 0x8f48a4899877186c}, // -306
	{0xe0b62e2929aba83c, 0x331acdabfe94de87}, // -305
	{0x8c71dcd9ba0b4925, 0x9ff0c08b7f1d0b14}, // -304
	{0xaf8e5410288e1b6f, 0x07ecf0ae5ee44dd9}, // -303
	{0xdb71e91432b1a24a, 0xc9e82cd9f69d6150}, // -302
	{0x892731ac9faf056e, 0xbe311c083a225cd2}, // -301
	{0xab70fe17c79ac6ca, 0x6dbd630a48aaf406}, // -300
	{0xd64d3d9db981787d, 0x092cbbccdad5b108}, // -299
	{0x85f0468293f0eb4e, 0x25bbf56008c58ea5}, // -298
	{0xa76c582338ed2621, 0xaf2af2b80af6f24e}, // -297
	{0xd1476e2c07286faa, 0x1af5af660db4aee1}, // -296
	{0x82cca4db847945ca, 0x50d98d9fc890ed4d}, // -295
	{0xa37fce126597973c, 0xe50ff107bab528a0}, // -294
	{0xcc5fc196fefd7d0c, 0x1e53ed49a96272c8}, // -293
	{0xff77b1fcbebcdc4f, 0x25e8e89c13bb0f7a}, // -292
	{0x9faacf3df73609b1, 0x77b191618c54e9ac}, // -291
	{0xc795830d75038c1d, 0xd59df5b9ef6a2417}, // -290
	{0xf97ae3d0d2446f25, 0x4b0573286b44ad1d}, // -289
	{0x9becce62836ac577, 0x4ee367f9430aec32}, // -288
	{0xc2e801fb244576d5, 0x229c41f793cda73f}, // -287
	{0xf3a20279ed56d48a, 0x6b43527578c1110f}, // -286
	{0x9845418c345644d6, 0x830a13896b78aaa9}, // -285
	{0xbe5691ef416bd60c, 0x23cc986bc656d553}, // -284
	{0xedec366b11c6cb8f, 0x2cbfbe86b7ec8aa8}, // -283
	{0x94b3a202eb1c3f39, 0x7bf7d71432f3d6a9}, // -282
	{0xb9e08a83a5e34f07, 0xdaf5ccd93fb0cc53}, // -281
	{0xe858ad248f5c22c9, 0xd1b3400f8f9cff68}, // -280
	{0x91376c36d99995be, 0x23100809b9c21fa1}, // -279
	{0xb58547448ffffb2d, 0xabd40a0c2832a78a}, // -278
	{0xe2e69915b3fff9f9, 0x16c90c8f323f516c}, // -277
	{0x8dd01fad907ffc3b, 0xae3da7d97f6792e3}, // -276
	{0xb1442798f49ffb4a, 0x99cd11cfdf41779c}, // -275
	{0xdd95317f31c7fa1d, 0x40405643d711d583}, // -274
	{0x8a7d3eef7f1cfc52, 0x482835ea666b2572}, // -273
	{0xad1c8eab5ee43b66, 0xda3243650005eecf}, // -272
	{0xd863b256369d4a40, 0x90bed43e40076a82}, // -271
	{0x873e4f75e2224e68, 0x5a7744a6e804a291}, // -270
	{0xa90de3535aaae202, 0x711515d0a205cb36}, // -269
	{0xd3515c2831559a83, 0x0d5a5b44ca873e03}, // -268
	{0x8412d9991ed58091, 0xe858790afe9486c2}, // -267
	{0xa5178fff668ae0b6, 0x626e974dbe39a872}, // -266
	{0xce5d73ff402d98e3, 0xfb0a3d212dc8128f}, // -265
	{0x80fa687f881c7f8e, 0x7ce66634bc9d0b99}, // -264
	{0xa139029f6a239f72, 0x1c1fffc1ebc44e80}, // -263
	{0xc987434744ac874e, 0xa327ffb266b56220}, // -262
	{0xfbe9141915d7a922, 0x4bf1ff9f0062baa8}, // -261
	{0x9d71ac8fada6c9b5, 0x6f773fc3603db4a9}, // -260
	{0xc4ce17b399107c22, 0xcb550fb4384d21d3}, // -259
	{0xf6019da07f549b2b, 0x7e2a53a146606a48}, // -258
	{0x99c102844f94e0fb, 0x2eda7444cbfc426d}, // -257
	{0xc0314325637a1939, 0xfa911155fefb5308}, // -256
	{0xf03d93eebc589f88, 0x793555ab7eba27ca}, // -255
	{0x96267c7535b763b5, 0x4bc1558b2f3458de}, // -254
	{0xbbb01b9283253ca2, 0x9eb1aaedfb016f16}, // -253
	{0xea9c227723ee8bcb, 0x465e15a979c1cadc}, // -252
	{0x92a1958a7675175f, 0x0bfacd89ec191ec9}, // -251
	{0xb749faed14125d36, 0xcef980ec671f667b}, // -250
	{0xe51c79a85916f484, 0x82b7e12780e7401a}, // -249
	{0x8f31cc0937ae58d2, 0xd1b2ecb8b0908810}, // -248
	{0xb2fe3f0b8599ef07, 0x861fa7e6dcb4aa15}, // -247
	{0xdfbdcece67006ac9, 0x67a791e093e1d49a}, // -246
	{0x8bd6a141006042bd, 0xe0c8bb2c5c6d24e0}, // -245
	{0xaecc49914078536d, 0x58fae9f773886e18}, // -244
	{0xda7f5bf590966848, 0xaf39a475506a899e}, // -243
	{0x888f99797a5e012d, 0x6d8406c952429603}, // -242
	{0xaab37fd7d8f58178, 0xc8e5087ba6d33b83}, // -241
	{0xd5605fcdcf32e1d6, 0xfb1e4a9a90880a64}, // -240
	{0x855c3be0a17fcd26, 0x5cf2eea09a55067f}, // -239
	{0xa6b34ad8c9dfc06f, 0xf42faa48c0ea481e}, // -238
	{0xd0601d8efc57b08b, 0xf13b94daf124da26}, // -237
	{0x823c12795db6ce57, 0x76c53d08d6b70858}, // -236
	{0xa2cb1717b52481ed, 0x54768c4b0c64ca6e}, // -235
	{0xcb7ddcdda26da268, 0xa9942f5dcf7dfd09}, // -234
	{0xfe5d54150b090b02, 0xd3f93b35435d7c4c}, // -233
	{0x9efa548d26e5a6e1, 0xc47bc5014a1a6daf}, // -232
	{0xc6b8e9b0709f109a, 0x359ab6419ca1091b}, // -231
	{0xf867241c8cc6d4c0, 0xc30163d203c94b62}, // -230
	{0x9b407691d7fc44f8, 0x79e0de63425dcf1d}, // -229
	{0xc21094364dfb5636, 0x985915fc12f542e4}, // -228
	{0xf294b943e17a2bc4, 0x3e6f5b7b17b2939d}, // -227
	{0x979cf3ca6cec5b5a, 0xa705992ceecf9c42}, // -226
	{0xbd8430bd08277231, 0x50c6ff782a838353}, // -225
	{0xece53cec4a314ebd, 0xa4f8bf5635246428}, // -224
	{0x940f4613ae5ed136, 0x871b7795e136be99}, // -223
	{0xb913179899f68584, 0x28e2557b59846e3f}, // -222
	{0xe757dd7ec07426e5, 0x331aeada2fe589cf}, // -221
	{0x9096ea6f3848984f, 0x3ff0d2c85def7621}, // -220
	{0xb4bca50b065abe63, 0x0fed077a756b53a9}, // -219
	{0xe1ebce4dc7f16dfb, 0xd3e8495912c62894}, // -218
	{0x8d3360f09cf6e4bd, 0x64712dd7abbbd95c}, // -217
	{0xb080392cc4349dec, 0xbd8d794d96aacfb3}, // -216
	{0xdca04777f541c567, 0xecf0d7a0fc5583a0}, // -215
	{0x89e42caaf9491b60, 0xf41686c49db57244}, // -214
	{0xac5d37d5b79b6239, 0x311c2875c522ced5}, // -213
	{0xd77485cb25823ac7, 0x7d633293366b828b}, // -212
	{0x86a8d39ef77164bc, 0xae5dff9c02033197}, // -211
	{0xa8530886b54dbdeb, 0xd9f57f830283fdfc}, // -210
	{0xd267caa862a12d66, 0xd072df63c324fd7b}, // -209
	{0x8380dea93da4bc60, 0x4247cb9e59f71e6d}, // -208
	{0xa46116538d0deb78, 0x52d9be85f074e608}, // -207
	{0xcd795be870516656, 0x67902e276c921f8b}, // -206
	{0x806bd9714632dff6, 0x00ba1cd8a3db53b6}, // -205
	{0xa086cfcd97bf97f3, 0x80e8a40eccd228a4}, // -204
	{0xc8a883c0fdaf7df0, 0x6122cd128006b2cd}, // -203
	{0xfad2a4b13d1b5d6c, 0x796b805720085f81}, // -202
	{0x9cc3a6eec6311a63, 0xcbe3303674053bb0}, // -201
	{0xc3f490aa77bd60fc, 0xbedbfc4411068a9c}, // -200
	{0xf4f1b4d515acb93b, 0xee92fb5515482d44}, // -199
	{0x991711052d8bf3c5, 0x751bdd152d4d1c4a}, // -198
	{0xbf5cd54678eef0b6, 0xd262d45a78a0635d}, // -197
	{0xef340a98172aace4, 0x86fb897116c87c34}, // -196
	{0x9580869f0e7aac0e, 0xd45d35e6ae3d4da0}, // -195
	{0xbae0a846d2195712, 0x8974836059cca109}, // -194
	{0xe998d258869facd7, 0x2bd1a438703fc94b}, // -193
	{0x91ff83775423cc06, 0x7b6306a34627ddcf}, // -192
	{0xb67f6455292cbf08, 0x1a3bc84c17b1d542}, // -191
	{0xe41f3d6a7377eeca, 0x20caba5f1d9e4a93}, // -190
	{0x8e938662882af53e, 0x547eb47b7282ee9c}, // -189
	{0xb23867fb2a35b28d, 0xe99e619a4f23aa43}, // -188
	{0xdec681f9f4c31f31, 0x6405fa00e2ec94d4}, // -187
	{0x8b3c113c38f9f37e, 0xde83bc408dd3dd04}, // -186
	{0xae0b158b4738705e, 0x9624ab50b148d445}, // -185
	{0xd98ddaee19068c76, 0x3badd624dd9b0957}, // -184
	{0x87f8a8d4cfa417c9, 0xe54ca5d70a80e5d6}, // -183
	{0xa9f6d30a038d1dbc, 0x5e9fcf4ccd211f4c}, // -182
	{0xd47487cc8470652b, 0x7647c3200069671f}, // -181
	{0x84c8d4dfd2c63f3b, 0x29ecd9f40041e073}, // -180
	{0xa5fb0a17c777cf09, 0xf468107100525890}, // -179
	{0xcf79cc9db955c2cc, 0x7182148d4066eeb4}, // -178
	{0x81ac1fe293d599bf, 0xc6f14cd848405530}, // -177
	{0xa21727db38cb002f, 0xb8ada00e5a506a7c}, // -176
	{0xca9cf1d206fdc03b, 0xa6d90811f0e4851c}, // -175
	{0xfd442e4688bd304a, 0x908f4a166d1da663}, // -174
	{0x9e4a9cec15763e2e, 0x9a598e4e043287fe}, // -173
	{0xc5dd44271ad3cdba, 0x40eff1e1853f29fd}, // -172
	{0xf7549530e188c128, 0xd12bee59e68ef47c}, // -171
	{0x9a94dd3e8cf578b9, 0x82bb74f8301958ce}, // -170
	{0xc13a148e3032d6e7, 0xe36a52363c1faf01}, // -169
	{0xf18899b1bc3f8ca1, 0xdc44e6c3cb279ac1}, // -168
	{0x96f5600f15a7b7e5, 0x29ab103a5ef8c0b9}, // -167
	{0xbcb2b812db11a5de, 0x7415d448f6b6f0e7}, // -166
	{0xebdf661791d60f56, 0x111b495b3464ad21}, // -165
	{0x936b9fcebb25c995, 0xcab10dd900beec34}, // -164
	{0xb84687c269ef3bfb, 0x3d5d514f40eea742}, // -163
	{0xe65829b3046b0afa, 0x0cb4a5a3112a5112}, // -162
	{0x8ff71a0fe2c2e6dc, 0x47f0e785eaba72ab}, // -161
	{0xb3f4e093db73a093, 0x59ed216765690f56}, // -160
	{0xe0f218b8d25088b8, 0x306869c13ec3532c}, // -159
	{0x8c974f7383725573, 0x1e414218c73a13fb}, // -158
	{0xafbd2350644eeacf, 0xe5d1929ef90898fa}, // -157
	{0xdbac6c247d62a583, 0xdf45f746b74abf39}, // -156
	{0x894bc396ce5da772, 0x6b8bba8c328eb783}, // -155
	{0xab9eb47c81f5114f, 0x066ea92f3f326564}, // -154
	{0xd686619ba27255a2, 0xc80a537b0efefebd}, // -153
	{0x8613fd0145877585, 0xbd06742ce95f5f36}, // -152
	{0xa798fc4196e952e7, 0x2c48113823b73704}, // -151
	{0xd17f3b51fca3a7a0, 0xf75a15862ca504c5}, // -150
	{0x82ef85133de648c4, 0x9a984d73dbe722fb}, // -149
	{0xa3ab66580d5fdaf5, 0xc13e60d0d2e0ebba}, // -148
	{0xcc963fee10b7d1b3, 0x318df905079926a8}, // -147
	{0xffbbcfe994e5c61f, 0xfdf17746497f7052}, // -146
	{0x9fd561f1fd0f9bd3, 0xfeb6ea8bedefa633}, // -145
	{0xc7caba6e7c5382c8, 0xfe64a52ee96b8fc0}, // -144
	{0xf9bd690a1b68637b, 0x3dfdce7aa3c673b0}, // -143
	{0x9c1661a651213e2d, 0x06bea10ca65c084e}, // -142
	{0xc31bfa0fe5698db8, 0x486e494fcff30a62}, // -141
	{0xf3e2f893dec3f126, 0x5a89dba3c3efccfa}, // -140
	{0x986ddb5c6b3a76b7, 0xf89629465a75e01c}, // -139
	{0xbe89523386091465, 0xf6bbb397f1135823}, // -138
	{0xee2ba6c0678b597f, 0x746aa07ded582e2c}, // -137
	{0x94db483840b717ef, 0xa8c2a44eb4571cdc}, // -136
	{0xba121a4650e4ddeb, 0x92f34d62616ce413}, // -135
	{0xe896a0d7e51e1566, 0x77b020baf9c81d17}, // -134
	{0x915e2486ef32cd60, 0x0ace1474dc1d122e}, // -133
	{0xb5b5ada8aaff80b8, 0x0d819992132456ba}, // -132
	{0xe3231912d5bf60e6, 0x10e1fff697ed6c69}, // -131
	{0x8df5efabc5979c8f, 0xca8d3ffa1ef463c1}, // -130
	{0xb1736b96b6fd83b3, 0xbd308ff8a6b17cb2}, // -129
	{0xddd0467c64bce4a0, 0xac7cb3f6d05ddbde}, // -128
	{0x8aa22c0dbef60ee4, 0x6bcdf07a423aa96b}, // -127
	{0xad4ab7112eb3929d, 0x86c16c98d2c953c6}, // -126
	{0xd89d64d57a607744, 0xe871c7bf077ba8b7}, // -125
	{0x87625f056c7c4a8b, 0x11471cd764ad4972}, // -124
	{0xa93af6c6c79b5d2d, 0xd598e40d3dd89bcf}, // -123
	{0xd389b47879823479, 0x4aff1d108d4ec2c3}, // -122
	{0x843610cb4bf160cb, 0xcedf722a585139ba}, // -121
	{0xa54394fe1eedb8fe, 0xc2974eb4ee658828}, // -120
	{0xce947a3da6a9273e, 0x733d226229feea32}, // -119
	{0x811ccc668829b887, 0x0806357d5a3f525f}, // -118
	{0xa163ff802a3426a8, 0xca07c2dcb0cf26f7}, // -117
	{0xc9bcff6034c13052, 0xfc89b393dd02f0b5}, // -116
	{0xfc2c3f3841f17c67, 0xbbac2078d443ace2}, // -115
	{0x9d9ba7832936edc0, 0xd54b944b84aa4c0d}, // -114
	{0xc5029163f384a931, 0x0a9e795e65d4df11}, // -113
	{0xf64335bcf065d37d, 0x4d4617b5ff4a16d5}, // -112
	{0x99ea0196163fa42e, 0x504bced1bf8e4e45}, // -111
	{0xc06481fb9bcf8d39, 0xe45ec2862f71e1d6}, // -110
	{0xf07da27a82c37088, 0x5d767327bb4e5a4c}, // -109
	{0x964e858c91ba2655, 0x3a6a07f8d510f86f}, // -108
	{0xbbe226efb628afea, 0x890489f70a55368b}, // -107
	{0xeadab0aba3b2dbe5, 0x2b45ac74ccea842e}, // -106
	{0x92c8ae6b464fc96f, 0x3b0b8bc90012929d}, // -105
	{0xb77ada0617e3bbcb, 0x09ce6ebb40173744}, // -104
	{0xe55990879ddcaabd, 0xcc420a6a101d0515}, // -103
	{0x8f57fa54c2a9eab6, 0x9fa946824a12232d}, // -102
	{0xb32df8e9f3546564, 0x47939822dc96abf9}, // -101
	{0xdff9772470297ebd, 0x59787e2b93bc56f7}, // -100
	{0x8bfbea76c619ef36, 0x57eb4edb3c55b65a}, //  -99
	{0xaefae51477a06b03, 0xede622920b6b23f1}, //  -98
	{0xdab99e59958885c4, 0xe95fab368e45eced}, //  -97
	{0x88b402f7fd75539b, 0x11dbcb0218ebb414}, //  -96
	{0xaae103b5fcd2a881, 0xd652bdc29f26a119}, //  -95
	{0xd59944a37c0752a2, 0x4be76d3346f0495f}, //  -94
	{0x857fcae62d8493a5, 0x6f70a4400c562ddb}, //  -93
	{0xa6dfbd9fb8e5b88e, 0xcb4ccd500f6bb952}, //  -92
	{0xd097ad07a71f26b2, 0x7e2000a41346a7a7}, //  -91
	{0x825ecc24c873782f, 0x8ed400668c0c28c8}, //  -90
	{0xa2f67f2dfa90563b, 0x728900802f0f32fa}, //  -89
	{0xcbb41ef979346bca, 0x4f2b40a03ad2ffb9}, //  -88
	{0xfea126b7d78186bc, 0xe2f610c84987bfa8}, //  -87
	{0x9f24b832e6b0f436, 0x0dd9ca7d2df4d7c9}, //  -86
	{0xc6ede63fa05d3143, 0x91503d1c79720dbb}, //  -85
	{0xf8a95fcf88747d94, 0x75a44c6397ce912a}, //  -84
	{0x9b69dbe1b548ce7c, 0xc986afbe3ee11aba}, //  -83
	{0xc24452da229b021b, 0xfbe85badce996168}, //  -82
	{0xf2d56790ab41c2a2, 0xfae27299423fb9c3}, //  -81
	{0x97c560ba6b0919a5, 0xdccd879fc967d41a}, //  -80
	{0xbdb6b8e905cb600f, 0x5400e987bbc1c920}, //  -79
	{0xed246723473e3813, 0x290123e9aab23b68}, //  -78
	{0x9436c0760c86e30b, 0xf9a0b6720aaf6521}, //  -77
	{0xb94470938fa89bce, 0xf808e40e8d5b3e69}, //  -76
	{0xe7958cb87392c2c2, 0xb60b1d1230b20e04}, //  -75
	{0x90bd77f3483bb9b9, 0xb1c6f22b5e6f48c2}, //  -74
	{0xb4ecd5f01a4aa828, 0x1e38aeb6360b1af3}, //  -73
	{0xe2280b6c20dd5232, 0x25c6da63c38de1b0}, //  -72
	{0x8d590723948a535f, 0x579c487e5a38ad0e}, //  -71
	{0xb0af48ec79ace837, 0x2d835a9df0c6d851}, //  -70
	{0xdcdb1b2798182244, 0xf8e431456cf88e65}, //  -69
	{0x8a08f0f8bf0f156b, 0x1b8e9ecb641b58ff}, //  -68
	{0xac8b2d36eed2dac5, 0xe272467e3d222f3f}, //  -67
	{0xd7adf884aa879177, 0x5b0ed81dcc6abb0f}, //  -66
	{0x86ccbb52ea94baea, 0x98e947129fc2b4e9}, //  -65
	{0xa87fea27a539e9a5, 0x3f2398d747b36224}, //  -64
	{0xd29fe4b18e88640e, 0x8eec7f0d19a03aad}, //  -63
	{0x83a3eeeef9153e89, 0x1953cf68300424ac}, //  -62
	{0xa48ceaaab75a8e2b, 0x5fa8c3423c052dd7}, //  -61
	{0xcdb02555653131b6, 0x3792f412cb06794d}, //  -60
	{0x808e17555f3ebf11, 0xe2bbd88bbee40bd0}, //  -59
	{0xa0b19d2ab70e6ed6, 0x5b6aceaeae9d0ec4}, //  -58
	{0xc8de047564d20a8b, 0xf245825a5a445275}, //  -57
	{0xfb158592be068d2e, 0xeed6e2f0f0d56712}, //  -56
	{0x9ced737bb6c4183d, 0x55464dd69685606b}, //  -55
	{0xc428d05aa4751e4c, 0xaa97e14c3c26b886}, //  -54
	{0xf53304714d9265df, 0xd53dd99f4b3066a8}, //  -53
	{0x993fe2c6d07b7fab, 0xe546a8038efe4029}, //  -52
	{0xbf8fdb78849a5f96, 0xde98520472bdd033}, //  -51
	{0xef73d256a5c0f77c, 0x963e66858f6d4440}, //  -50
	{0x95a8637627989aad, 0xdde7001379a44aa8}, //  -49
	{0xbb127c53b17ec159, 0x5560c018580d5d52}, //  -48
	{0xe9d71b689dde71af, 0xaab8f01e6e10b4a6}, //  -47
	{0x9226712162ab070d, 0xcab3961304ca70e8}, //  -46
	{0xb6b00d69bb55c8d1, 0x3d607b97c5fd0d22}, //  -45
	{0xe45c10c42a2b3b05, 0x8cb89a7db77c506a}, //  -44
	{0x8eb98a7a9a5b04e3, 0x77f3608e92adb242}, //  -43
	{0xb267ed1940f1c61c, 0x55f038b237591ed3}, //  -42
	{0xdf01e85f912e37a3, 0x6b6c46dec52f6688}, //  -41
	{0x8b61313bbabce2c6, 0x2323ac4b3b3da015}, //  -40
	{0xae397d8aa96c1b77, 0xabec975e0a0d081a}, //  -39
	{0xd9c7dced53c72255, 0x96e7bd358c904a21}, //  -38
	{0x881cea14545c7575, 0x7e50d64177da2e54}, //  -37
	{0xaa242499697392d2, 0xdde50bd1d5d0b9e9}, //  -36
	{0xd4ad2dbfc3d07787, 0x955e4ec64b44e864}, //  -35
	{0x84ec3c97da624ab4, 0xbd5af13bef0b113e}, //  -34
	{0xa6274bbdd0fadd61, 0xecb1ad8aeacdd58e}, //  -33
	{0xcfb11ead453994ba, 0x67de18eda5814af2}, //  -32
	{0x81ceb32c4b43fcf4, 0x80eacf948770ced7}, //  -31
	{0xa2425ff75e14fc31, 0xa1258379a94d028d}, //  -30
	{0xcad2f7f5359a3b3e, 0x096ee45813a04330}, //  -29
	{0xfd87b5f28300ca0d, 0x8bca9d6e188853fc}, //  -28
	{0x9e74d1b791e07e48, 0x775ea264cf55347d}, //  -27
	{0xc612062576589dda, 0x95364afe032a819d}, //  -26
	{0xf79687aed3eec551, 0x3a83ddbd83f52204}, //  -25
	{0x9abe14cd44753b52, 0xc4926a9672793542}, //  -24
	{0xc16d9a0095928a27, 0x75b7053c0f178293}, //  -23
	{0xf1c90080baf72cb1, 0x5324c68b12dd6338}, //  -22
	{0x971da05074da7bee, 0xd3f6fc16ebca5e03}, //  -21
	{0xbce5086492111aea, 0x88f4bb1ca6bcf584}, //  -20
	{0xec1e4a7db69561a5, 0x2b31e9e3d06c32e5}, //  -19
	{0x9392ee8e921d5d07, 0x3aff322e62439fcf}, //  -18
	{0xb877aa3236a4b449, 0x09befeb9fad487c2}, //  -17
	{0xe69594bec44de15b, 0x4c2ebe687989a9b3}, //  -16
	{0x901d7cf73ab0acd9, 0x0f9d37014bf60a10}, //  -15
	{0xb424dc35095cd80f, 0x538484c19ef38c94}, //  -14
	{0xe12e13424bb40e13, 0x2865a5f206b06fb9}, //  -13
	{0x8cbccc096f5088cb, 0xf93f87b7442e45d3}, //  -12
	{0xafebff0bcb24aafe, 0xf78f69a51539d748}, //  -11
	{0xdbe6fecebdedd5be, 0xb573440e5a884d1b}, //  -10
	{0x89705f4136b4a597, 0x31680a88f8953030}, //   -9
	{0xabcc77118461cefc, 0xfdc20d2b36ba7c3d}, //   -8
	{0xd6bf94d5e57a42bc, 0x3d32907604691b4c}, //   -7
	{0x8637bd05af6c69b5, 0xa63f9a49c2c1b10f}, //   -6
	{0xa7c5ac471b478423, 0x0fcf80dc33721d53}, //   -5
	{0xd1b71758e219652b, 0xd3c36113404ea4a8}, //   -4
	{0x83126e978d4fdf3b, 0x645a1cac083126e9}, //   -3
	{0xa3d70a3d70a3d70a, 0x3d70a3d70a3d70a3}, //   -2
	{0xcccccccccccccccc, 0xcccccccccccccccc}, //   -1
	{0x8000000000000000, 0x0000000000000000}, //    0
	{0xa000000000000000, 0x0000000000000000}, //    1
	{0xc800000000000000, 0x0000000000000000}, //    2
	{0xfa00000000000000, 0x0000000000000000}, //    3
	{0x9c40000000000000, 0x0000000000000000}, //    4
	{0xc350000000000000, 0x0000000000000000}, //    5
	{0xf424000000000000, 0x0000000000000000}, //    6
	{0x9896800000000000, 0x0000000000000000}, //    7
	{0xbebc200000000000, 0x0000000000000000}, //    8
	{0xee6b280000000000, 0x0000000000000000}, //    9
	{0x9502f90000000000, 0x0000000000000000}, //   10
	{0xba43b74000000000, 0x0000000000000000}, //   11
	{0xe8d4a51000000000, 0x0000000000000000}, //   12
	{0x9184e72a00000000, 0x0000000000000000}, //   13
	{0xb5e620f480000000, 0x0000000000000000}, //   14
	{0xe35fa931a0000000, 0x0000000000000000}, //   15
	{0x8e1bc9bf04000000, 0x0000000000000000}, //   16
	{0xb1a2bc2ec5000000, 0x0000000000000000}, //   17
	{0xde0b6b3a76400000, 0x0000000000000000}, //   18
	{0x8ac7230489e80000, 0x0000000000000000}, //   19
	{0xad78ebc5ac620000, 0x0000000000000000}, //   20
	{0xd8d726b7177a8000, 0x0000000000000000}, //   21
	{0x878678326eac9000, 0x0000000000000000}, //   22
	{0xa968163f0a57b400, 0x0000000000000000}, //   23
	{0xd3c21bcecceda100, 0x0000000000000000}, //   24
	{0x84595161401484a0, 0x0000000000000000}, //   25
	{0xa56fa5b99019a5c8, 0x0000000000000000}, //   26
	{0xcecb8f27f4200f3a, 0x0000000000000000}, //   27
	{0x813f3978f8940984, 0x4000000000000000}, //   28
	{0xa18f07d736b90be5, 0x5000000000000000}, //   29
	{0xc9f2c9cd04674ede, 0xa400000000000000}, //   30
	{0xfc6f7c4045812296, 0x4d00000000000000}, //   31
	{0x9dc5ada82b70b59d, 0xf020000000000000}, //   32
	{0xc5371912364ce305, 0x6c28000000000000}, //   33
	{0xf684df56c3e01bc6, 0xc732000000000000}, //   34
	{0x9a130b963a6c115c, 0x3c7f400000000000}, //   35
	{0xc097ce7bc90715b3, 0x4b9f100000000000}, //   36
	{0xf0bdc21abb48db20, 0x1e86d40000000000}, //   37
	{0x96769950b50d88f4, 0x1314448000000000}, //   38
	{0xbc143fa4e250eb31, 0x17d955a000000000}, //   39
	{0xeb194f8e1ae525fd, 0x5dcfab0800000000}, //   40
	{0x92efd1b8d0cf37be, 0x5aa1cae500000000}, //   41
	{0xb7abc627050305ad, 0xf14a3d9e40000000}, //   42
	{0xe596b7b0c643c719, 0x6d9ccd05d0000000}, //   43
	{0x8f7e32ce7bea5c6f, 0xe4820023a2000000}, //   44
	{0xb35dbf821ae4f38b, 0xdda2802c8a800000}, //   45
	{0xe0352f62a19e306e, 0xd50b2037ad200000}, //   46
	{0x8c213d9da502de45, 0x4526f422cc340000}, //   47
	{0xaf298d050e4395d6, 0x9670b12b7f410000}, //   48
	{0xdaf3f04651d47b4c, 0x3c0cdd765f114000}, //   49
	{0x88d8762bf324cd0f, 0xa5880a69fb6ac800}, //   50
	{0xab0e93b6efee0053, 0x8eea0d047a457a00}, //   51
	{0xd5d238a4abe98068, 0x72a4904598d6d880}, //   52
	{0x85a36366eb71f041, 0x47a6da2b7f864750}, //   53
	{0xa70c3c40a64e6c51, 0x999090b65f67d924}, //   54
	{0xd0cf4b50cfe20765, 0xfff4b4e3f741cf6d}, //   55
	{0x82818f1281ed449f, 0xbff8f10e7a8921a4}, //   56
	{0xa321f2d7226895c7, 0xaff72d52192b6a0d}, //   57
	{0xcbea6f8ceb02bb39, 0x9bf4f8a69f764490}, //   58
	{0xfee50b7025c36a08, 0x02f236d04753d5b4}, //   59
	{0x9f4f2726179a2245, 0x01d762422c946590}, //   60
	{0xc722f0ef9d80aad6, 0x424d3ad2b7b97ef5}, //   61
	{0xf8ebad2b84e0d58b, 0xd2e0898765a7deb2}, //   62
	{0x9b934c3b330c8577, 0x63cc55f49f88eb2f}, //   63
	{0xc2781f49ffcfa6d5, 0x3cbf6b71c76b25fb}, //   64
	{0xf316271c7fc3908a, 0x8bef464e3945ef7a}, //   65
	{0x97edd871cfda3a56, 0x97758bf0e3cbb5ac}, //   66
	{0xbde94e8e43d0c8ec, 0x3d52eeed1cbea317}, //   67
	{0xed63a231d4c4fb27, 0x4ca7aaa863ee4bdd}, //   68
	{0x945e455f24fb1cf8, 0x8fe8caa93e74ef6a}, //   69
	{0xb975d6b6ee39e436, 0xb3e2fd538e122b44}, //   70
	{0xe7d34c64a9c85d44, 0x60dbbca87196b616}, //   71
	{0x90e40fbeea1d3a4a, 0xbc8955e946fe31cd}, //   72
	{0xb51d13aea4a488dd, 0x6babab6398bdbe41}, //   73
	{0xe264589a4dcdab14, 0xc696963c7eed2dd1}, //   74
	{0x8d7eb76070a08aec, 0xfc1e1de5cf543ca2}, //   75
	{0xb0de65388cc8ada8, 0x3b25a55f43294bcb}, //   76
	{0xdd15fe86affad912, 0x49ef0eb713f39ebe}, //   77
	{0x8a2dbf142dfcc7ab, 0x6e3569326c784337}, //   78
	{0xacb92ed9397bf996, 0x49c2c37f07965404}, //   79
	{0xd7e77a8f87daf7fb, 0xdc33745ec97be906}, //   80
	{0x86f0ac99b4e8dafd, 0x69a028bb3ded71a3}, //   81
	{0xa8acd7c0222311bc, 0xc40832ea0d68ce0c}, //   82
	{0xd2d80db02aabd62b, 0xf50a3fa490c30190}, //   83
	{0x83c7088e1aab65db, 0x792667c6da79e0fa}, //   84
	{0xa4b8cab1a1563f52, 0x577001b891185938}, //   85
	{0xcde6fd5e09abcf26, 0xed4c0226b55e6f86}, //   86
	{0x80b05e5ac60b6178, 0x544f8158315b05b4}, //   87
	{0xa0dc75f1778e39d6, 0x696361ae3db1c721}, //   88
	{0xc913936dd571c84c, 0x03bc3a19cd1e38e9}, //   89
	{0xfb5878494ace3a5f, 0x04ab48a04065c723}, //   90
	{0x9d174b2dcec0e47b, 0x62eb0d64283f9c76}, //   91
	{0xc45d1df942711d9a, 0x3ba5d0bd324f8394}, //   92
	{0xf5746577930d6500, 0xca8f44ec7ee36479}, //   93
	{0x9968bf6abbe85f20, 0x7e998b13cf4e1ecb}, //   94
	{0xbfc2ef456ae276e8, 0x9e3fedd8c321a67e}, //   95
	{0xefb3ab16c59b14a2, 0xc5cfe94ef3ea101e}, //   96
	{0x95d04aee3b80ece5, 0xbba1f1d158724a12}, //   97
	{0xbb445da9ca61281f, 0x2a8a6e45ae8edc97}, //   98
	{0xea1575143cf97226, 0xf52d09d71a3293bd}, //   99
	{0x924d692ca61be758, 0x593c2626705f9c56}, //  100
	{0xb6e0c377cfa2e12e, 0x6f8b2fb00c77836c}, //  101
	{0xe498f455c38b997a, 0x0b6dfb9c0f956447}, //  102
	{0x8edf98b59a373fec, 0x4724bd4189bd5eac}, //  103
	{0xb2977ee300c50fe7, 0x58edec91ec2cb657}, //  104
	{0xdf3d5e9bc0f653e1, 0x2f2967b66737e3ed}, //  105
	{0x8b865b215899f46c, 0xbd79e0d20082ee74}, //  106
	{0xae67f1e9aec07187, 0xecd8590680a3aa11}, //  107
	{0xda01ee641a708de9, 0xe80e6f4820cc9495}, //  108
	{0x884134fe908658b2, 0x3109058d147fdcdd}, //  109
	{0xaa51823e34a7eede, 0xbd4b46f0599fd415}, //  110
	{0xd4e5e2cdc1d1ea96, 0x6c9e18ac7007c91a}, //  111
	{0x850fadc09923329e, 0x03e2cf6bc604ddb0}, //  112
	{0xa6539930bf6bff45, 0x84db8346b786151c}, //  113
	{0xcfe87f7cef46ff16, 0xe612641865679a63}, //  114
	{0x81f14fae158c5f6e, 0x4fcb7e8f3f60c07e}, //  115
	{0xa26da3999aef7749, 0xe3be5e330f38f09d}, //  116
	{0xcb090c8001ab551c, 0x5cadf5bfd3072cc5}, //  117
	{0xfdcb4fa002162a63, 0x73d9732fc7c8f7f6}, //  118
	{0x9e9f11c4014dda7e, 0x2867e7fddcdd9afa}, //  119
	{0xc646d63501a1511d, 0xb281e1fd541501b8}, //  120
	{0xf7d88bc24209a565, 0x1f225a7ca91a4226}, //  121
	{0x9ae757596946075f, 0x3375788de9b06958}, //  122
	{0xc1a12d2fc3978937, 0x0052d6b1641c83ae}, //  123
	{0xf209787bb47d6b84, 0xc0678c5dbd23a49a}, //  124
	{0x9745eb4d50ce6332, 0xf840b7ba963646e0}, //  125
	{0xbd176620a501fbff, 0xb650e5a93bc3d898}, //  126
	{0xec5d3fa8ce427aff, 0xa3e51f138ab4cebe}, //  127
	{0x93ba47c980e98cdf, 0xc66f336c36b10137}, //  128
	{0xb8a8d9bbe123f017, 0xb80b0047445d4184}, //  129
	{0xe6d3102ad96cec1d, 0xa60dc059157491e5}, //  130
	{0x9043ea1ac7e41392, 0x87c89837ad68db2f}, //  131
	{0xb454e4a179dd1877, 0x29babe4598c311fb}, //  132
	{0xe16a1dc9d8545e94, 0xf4296dd6fef3d67a}, //  133
	{0x8ce2529e2734bb1d, 0x1899e4a65f58660c}, //  134
	{0xb01ae745b101e9e4, 0x5ec05dcff72e7f8f}, //  135
	{0xdc21a1171d42645d, 0x76707543f4fa1f73}, //  136
	{0x899504ae72497eba, 0x6a06494a791c53a8}, //  137
	{0xabfa45da0edbde69, 0x0487db9d17636892}, //  138
	{0xd6f8d7509292d603, 0x45a9d2845d3c42b6}, //  139
	{0x865b86925b9bc5c2, 0x0b8a2392ba45a9b2}, //  140
	{0xa7f26836f282b732, 0x8e6cac7768d7141e}, //  141
	{0xd1ef0244af2364ff, 0x3207d795430cd926}, //  142
	{0x8335616aed761f1f, 0x7f44e6bd49e807b8}, //  143
	{0xa402b9c5a8d3a6e7, 0x5f16206c9c6209a6}, //  144
	{0xcd036837130890a1, 0x36dba887c37a8c0f}, //  145
	{0x802221226be55a64, 0xc2494954da2c9789}, //  146
	{0xa02aa96b06deb0fd, 0xf2db9baa10b7bd6c}, //  147
	{0xc83553c5c8965d3d, 0x6f92829494e5acc7}, //  148
	{0xfa42a8b73abbf48c, 0xcb772339ba1f17f9}, //  149
	{0x9c69a97284b578d7, 0xff2a760414536efb}, //  150
	{0xc38413cf25e2d70d, 0xfef5138519684aba}, //  151
	{0xf46518c2ef5b8cd1, 0x7eb258665fc25d69}, //  152
	{0x98bf2f79d5993802, 0xef2f773ffbd97a61}, //  153
	{0xbeeefb584aff8603, 0xaafb550ffacfd8fa}, //  154
	{0xeeaaba2e5dbf6784, 0x95ba2a53f983cf38}, //  155
	{0x952ab45cfa97a0b2, 0xdd945a747bf26183}, //  156
	{0xba756174393d88df, 0x94f971119aeef9e4}, //  157
	{0xe912b9d1478ceb17, 0x7a37cd5601aab85d}, //  158
	{0x91abb422ccb812ee, 0xac62e055c10ab33a}, //  159
	{0xb616a12b7fe617aa, 0x577b986b314d6009}, //  160
	{0xe39c49765fdf9d94, 0xed5a7e85fda0b80b}, //  161
	{0x8e41ade9fbebc27d, 0x14588f13be847307}, //  162
	{0xb1d219647ae6b31c, 0x596eb2d8ae258fc8}, //  163
	{0xde469fbd99a05fe3, 0x6fca5f8ed9aef3bb}, //  164
	{0x8aec23d680043bee, 0x25de7bb9480d5854}, //  165
	{0xada72ccc20054ae9, 0xaf561aa79a10ae6a}, //  166
	{0xd910f7ff28069da4, 0x1b2ba1518094da04}, //  167
	{0x87aa9aff79042286, 0x90fb44d2f05d0842}, //  168
	{0xa99541bf57452b28, 0x353a1607ac744a53}, //  169
	{0xd3fa922f2d1675f2, 0x42889b8997915ce8}, //  170
	{0x847c9b5d7c2e09b7, 0x69956135febada11}, //  171
	{0xa59bc234db398c25, 0x43fab9837e699095}, //  172
	{0xcf02b2c21207ef2e, 0x94f967e45e03f4bb}, //  173
	{0x8161afb94b44f57d, 0x1d1be0eebac278f5}, //  174
	{0xa1ba1ba79e1632dc, 0x6462d92a69731732}, //  175
	{0xca28a291859bbf93, 0x7d7b8f7503cfdcfe}, //  176
	{0xfcb2cb35e702af78, 0x5cda735244c3d43e}, //  177
	{0x9defbf01b061adab, 0x3a0888136afa64a7}, //  178
	{0xc56baec21c7a1916, 0x088aaa1845b8fdd0}, //  179
	{0xf6c69a72a3989f5b, 0x8aad549e57273d45}, //  180
	{0x9a3c2087a63f6399, 0x36ac54e2f678864b}, //  181
	{0xc0cb28a98fcf3c7f, 0x84576a1bb416a7dd}, //  182
	{0xf0fdf2d3f3c30b9f, 0x656d44a2a11c51d5}, //  183
	{0x969eb7c47859e743, 0x9f644ae5a4b1b325}, //  184
	{0xbc4665b596706114, 0x873d5d9f0dde1fee}, //  185
	{0xeb57ff22fc0c7959, 0xa90cb506d155a7ea}, //  186
	{0x9316ff75dd87cbd8, 0x09a7f12442d588f2}, //  187
	{0xb7dcbf5354e9bece, 0x0c11ed6d538aeb2f}, //  188
	{0xe5d3ef282a242e81, 0x8f1668c8a86da5fa}, //  189
	{0x8fa475791a569d10, 0xf96e017d694487bc}, //  190
	{0xb38d92d760ec4455, 0x37c981dcc395a9ac}, //  191
	{0xe070f78d3927556a, 0x85bbe253f47b1417}, //  192
	{0x8c469ab843b89562, 0x93956d7478ccec8e}, //  193
	{0xaf58416654a6babb, 0x387ac8d1970027b2}, //  194
	{0xdb2e51bfe9d0696a, 0x06997b05fcc0319e}, //  195
	{0x88fcf317f22241e2, 0x441fece3bdf81f03}, //  196
	{0xab3c2fddeeaad25a, 0xd527e81cad7626c3}, //  197
	{0xd60b3bd56a5586f1, 0x8a71e223d8d3b074}, //  198
	{0x85c7056562757456, 0xf6872d5667844e49}, //  199
	{0xa738c6bebb12d16c, 0xb428f8ac016561db}, //  200
	{0xd106f86e69d785c7, 0xe13336d701beba52}, //  201
	{0x82a45b450226b39c, 0xecc0024661173473}, //  202
	{0xa34d721642b06084, 0x27f002d7f95d0190}, //  203
	{0xcc20ce9bd35c78a5, 0x31ec038df7b441f4}, //  204
	{0xff290242c83396ce, 0x7e67047175a15271}, //  205
	{0x9f79a169bd203e41, 0x0f0062c6e984d386}, //  206
	{0xc75809c42c684dd1, 0x52c07b78a3e60868}, //  207
	{0xf92e0c3537826145, 0xa7709a56ccdf8a82}, //  208
	{0x9bbcc7a142b17ccb, 0x88a66076400bb691}, //  209
	{0xc2abf989935ddbfe, 0x6acff893d00ea435}, //  210
	{0xf356f7ebf83552fe, 0x0583f6b8c4124d43}, //  211
	{0x98165af37b2153de, 0xc3727a337a8b704a}, //  212
	{0xbe1bf1b059e9a8d6, 0x744f18c0592e4c5c}, //  213
	{0xeda2ee1c7064130c, 0x1162def06f79df73}, //  214
	{0x9485d4d1c63e8be7, 0x8addcb5645ac2ba8}, //  215
	{0xb9a74a0637ce2ee1, 0x6d953e2bd7173692}, //  216
	{0xe8111c87c5c1ba99, 0xc8fa8db6ccdd0437}, //  217
	{0x910ab1d4db9914a0, 0x1d9c9892400a22a2}, //  218
	{0xb54d5e4a127f59c8, 0x2503beb6d00cab4b}, //  219
	{0xe2a0b5dc971f303a, 0x2e44ae64840fd61d}, //  220
	{0x8da471a9de737e24, 0x5ceaecfed289e5d2}, //  221
	{0xb10d8e1456105dad, 0x7425a83e872c5f47}, //  222
	{0xdd50f1996b947518, 0xd12f124e28f77719}, //  223
	{0x8a5296ffe33cc92f, 0x82bd6b70d99aaa6f}, //  224
	{0xace73cbfdc0bfb7b, 0x636cc64d1001550b}, //  225
	{0xd8210befd30efa5a, 0x3c47f7e05401aa4e}, //  226
	{0x8714a775e3e95c78, 0x65acfaec34810a71}, //  227
	{0xa8d9d1535ce3b396, 0x7f1839a741a14d0d}, //  228
	{0xd31045a8341ca07c, 0x1ede48111209a050}, //  229
	{0x83ea2b892091e44d, 0x934aed0aab460432}, //  230
	{0xa4e4b66b68b65d60, 0xf81da84d5617853f}, //  231
	{0xce1de40642e3f4b9, 0x36251260ab9d668e}, //  232
	{0x80d2ae83e9ce78f3, 0xc1d72b7c6b426019}, //  233
	{0xa1075a24e4421730, 0xb24cf65b8612f81f}, //  234
	{0xc94930ae1d529cfc, 0xdee033f26797b627}, //  235
	{0xfb9b7cd9a4a7443c, 0x169840ef017da3b1}, //  236
	{0x9d412e0806e88aa5, 0x8e1f289560ee864e}, //  237
	{0xc491798a08a2ad4e, 0xf1a6f2bab92a27e2}, //  238
	{0xf5b5d7ec8acb58a2, 0xae10af696774b1db}, //  239
	{0x9991a6f3d6bf1765, 0xacca6da1e0a8ef29}, //  240
	{0xbff610b0cc6edd3f, 0x17fd090a58d32af3}, //  241
	{0xeff394dcff8a948e, 0xddfc4b4cef07f5b0}, //  242
	{0x95f83d0a1fb69cd9, 0x4abdaf101564f98e}, //  243
	{0xbb764c4ca7a4440f, 0x9d6d1ad41abe37f1}, //  244
	{0xea53df5fd18d5513, 0x84c86189216dc5ed}, //  245
	{0x92746b9be2f8552c, 0x32fd3cf5b4e49bb4}, //  246
	{0xb7118682dbb66a77, 0x3fbc8c33221dc2a1}, //  247
	{0xe4d5e82392a40515, 0x0fabaf3feaa5334a}, //  248
	{0x8f05b1163ba6832d, 0x29cb4d87f2a7400e}, //  249
	{0xb2c71d5bca9023f8, 0x743e20e9ef511012}, //  250
	{0xdf78e4b2bd342cf6, 0x914da9246b255416}, //  251
	{0x8bab8eefb6409c1a, 0x1ad089b6c2f7548e}, //  252
	{0xae9672aba3d0c320, 0xa184ac2473b529b1}, //  253
	{0xda3c0f568cc4f3e8, 0xc9e5d72d90a2741e}, //  254
	{0x8865899617fb1871, 0x7e2fa67c7a658892}, //  255
	{0xaa7eebfb9df9de8d, 0xddbb901b98feeab7}, //  256
	{0xd51ea6fa85785631, 0x552a74227f3ea565}, //  257
	{0x8533285c936b35de, 0xd53a88958f87275f}, //  258
	{0xa67ff273b8460356, 0x8a892abaf368f137}, //  259
	{0xd01fef10a657842c, 0x2d2b7569b0432d85}, //  260
	{0x8213f56a67f6b29b, 0x9c3b29620e29fc73}, //  261
	{0xa298f2c501f45f42, 0x8349f3ba91b47b8f}, //  262
	{0xcb3f2f7642717713, 0x241c70a936219a73}, //  263
	{0xfe0efb53d30dd4d7, 0xed238cd383aa0110}, //  264
	{0x9ec95d1463e8a506, 0xf4363804324a40aa}, //  265
	{0xc67bb4597ce2ce48, 0xb143c6053edcd0d5}, //  266
	{0xf81aa16fdc1b81da, 0xdd94b7868e94050a}, //  267
	{0x9b10a4e5e9913128, 0xca7cf2b4191c8326}, //  268
	{0xc1d4ce1f63f57d72, 0xfd1c2f611f63a3f0}, //  269
	{0xf24a01a73cf2dccf, 0xbc633b39673c8cec}, //  270
	{0x976e41088617ca01, 0xd5be0503e085d813}, //  271
	{0xbd49d14aa79dbc82, 0x4b2d8644d8a74e18}, //  272
	{0xec9c459d51852ba2, 0xddf8e7d60ed1219e}, //  273
	{0x93e1ab8252f33b45, 0xcabb90e5c942b503}, //  274
	{0xb8da1662e7b00a17, 0x3d6a751f3b936243}, //  275
	{0xe7109bfba19c0c9d, 0x0cc512670a783ad4}, //  276
	{0x906a617d450187e2, 0x27fb2b80668b24c5}, //  277
	{0xb484f9dc9641e9da, 0xb1f9f660802dedf6}, //  278
	{0xe1a63853bbd26451, 0x5e7873f8a0396973}, //  279
	{0x8d07e33455637eb2, 0xdb0b487b6423e1e8}, //  280
	{0xb049dc016abc5e5f, 0x91ce1a9a3d2cda62}, //  281
	{0xdc5c5301c56b75f7, 0x7641a140cc7810fb}, //  282
	{0x89b9b3e11b6329ba, 0xa9e904c87fcb0a9d}, //  283
	{0xac2820d9623bf429, 0x546345fa9fbdcd44}, //  284
	{0xd732290fbacaf133, 0xa97c177947ad4095}, //  285
	{0x867f59a9d4bed6c0, 0x49ed8eabcccc485d}, //  286
	{0xa81f301449ee8c70, 0x5c68f256bfff5a74}, //  287
	{0xd226fc195c6a2f8c, 0x73832eec6fff3111}, //  288
	{0x83585d8fd9c25db7, 0xc831fd53c5ff7eab}, //  289
	{0xa42e74f3d032f525, 0xba3e7ca8b77f5e55}, //  290
	{0xcd3a1230c43fb26f, 0x28ce1bd2e55f35eb}, //  291
	{0x80444b5e7aa7cf85, 0x7980d163cf5b81b3}, //  292
	{0xa0555e361951c366, 0xd7e105bcc332621f}, //  293
	{0xc86ab5c39fa63440, 0x8dd9472bf3fefaa7}, //  294
	{0xfa856334878fc150, 0xb14f98f6f0feb951}, //  295
	{0x9c935e00d4b9d8d2, 0x6ed1bf9a569f33d3}, //  296
	{0xc3b8358109e84f07, 0x0a862f80ec4700c8}, //  297
	{0xf4a642e14c6262c8, 0xcd27bb612758c0fa}, //  298
	{0x98e7e9cccfbd7dbd, 0x8038d51cb897789c}, //  299
	{0xbf21e44003acdd2c, 0xe0470a63e6bd56c3}, //  300
	{0xeeea5d5004981478, 0x1858ccfce06cac74}, //  301
	{0x95527a5202df0ccb, 0x0f37801e0c43ebc8}, //  302
	{0xbaa718e68396cffd, 0xd30560258f54e6ba}, //  303
	{0xe950df20247c83fd, 0x47c6b82ef32a2069}, //  304
	{0x91d28b7416cdd27e, 0x4cdc331d57fa5441}, //  305
	{0xb6472e511c81471d, 0xe0133fe4adf8e952}, //  306
	{0xe3d8f9e563a198e5, 0x58180fddd97723a6}, //  307
	{0x8e679c2f5e44ff8f, 0x570f09eaa7ea7648}, //  308
	{0xb201833b35d63f73, 0x2cd2cc6551e513da}, //  309
	{0xde81e40a034bcf4f, 0xf8077f7ea65e58d1}, //  310
	{0x8b112e86420f6191, 0xfb04afaf27faf782}, //  311
	{0xadd57a27d29339f6, 0x79c5db9af1f9b563}, //  312
	{0xd94ad8b1c7380874, 0x18375281ae7822bc}, //  313
	{0x87cec76f1c830548, 0x8f2293910d0b15b5}, //  314
	{0xa9c2794ae3a3c69a, 0xb2eb3875504ddb22}, //  315
	{0xd433179d9c8cb841, 0x5fa60692a46151eb}, //  316
	{0x849feec281d7f328, 0xdbc7c41ba6bcd333}, //  317
	{0xa5c7ea73224deff3, 0x12b9b522906c0800}, //  318
	{0xcf39e50feae16bef, 0xd768226b34870a00}, //  319
	{0x81842f29f2cce375, 0xe6a1158300d46640}, //  320
	{0xa1e53af46f801c53, 0x60495ae3c1097fd0}, //  321
	{0xca5e89b18b602368, 0x385bb19cb14bdfc4}, //  322
	{0xfcf62c1dee382c42, 0x46729e03dd9ed7b5}, //  323
	{0x9e19db92b4e31ba9, 0x6c07a2c26a8346d1}, //  324
	{0xc5a05277621be293, 0xc7098b7305241885}, //  325
	{0xf70867153aa2db38, 0xb8cbee4fc66d1ea7}, //  326
	{0x9a65406d44a5c903, 0x737f74f1dc043328}, //  327
	{0xc0fe908895cf3b44, 0x505f522e53053ff2}, //  328
	{0xf13e34aabb430a15, 0x647726b9e7c68fef}, //  329
	{0x96c6e0eab509e64d, 0x5eca783430dc19f5}, //  330
	{0xbc789925624c5fe0, 0xb67d16413d132072}, //  331
	{0xeb96bf6ebadf77d8, 0xe41c5bd18c57e88f}, //  332
	{0x933e37a534cbaae7, 0x8e91b962f7b6f159}, //  333
	{0xb80dc58e81fe95a1, 0x723627bbb5a4adb0}, //  334
	{0xe61136f2227e3b09, 0xcec3b1aaa30dd91c}, //  335
	{0x8fcac257558ee4e6, 0x213a4f0aa5e8a7b1}, //  336
	{0xb3bd72ed2af29e1f, 0xa988e2cd4f62d19d}, //  337
	{0xe0accfa875af45a7, 0x93eb1b80a33b8605}, //  338
	{0x8c6c01c9498d8b88, 0xbc72f130660533c3}, //  339
	{0xaf87023b9bf0ee6a, 0xeb8fad7c7f8680b4}, //  340
	{0xdb68c2ca82ed2a05, 0xa67398db9f6820e1}, //  341
	{0x892179be91d43a43, 0x88083f8943a1148c}, //  342
	{0xab69d82e364948d4, 0x6a0a4f6b948959b0}, //  343
	{0xd6444e39c3db9b09, 0x848ce34679abb01c}, //  344
	{0x85eab0e41a6940e5, 0xf2d80e0c0c0b4e11}, //  345
	{0xa7655d1d2103911f, 0x6f8e118f0f0e2195}, //  346
	{0xd13eb46469447567, 0x4b7195f2d2d1a9fb}, //  347
	{0x82c730bec1cac960, 0x8f26fdb7c3c30a3d}, //  348
	{0xa378fcee723d7bb8, 0xb2f0bd25b4b3cccc}, //  349
	{0xcc573c2a0eccdaa6, 0xdfacec6f21e0bfff} //  350
};

// Interface routine for the above table
static inline u2_64 get_pow10_significand(int32_t dec_exp) 
{ 
  if(dec_exp - dec_exp_min<0 || dec_exp - dec_exp_min > sizeof(u2_64_pow10_table)/sizeof(u2_64_pow10_table[0]))
  	{u2_64 result = {0, 0};// return something thats easy to check in caller [ all valid entries in table have msbit=1 and .hi!=0 ]
  	 return result;
	}
  return u2_64_pow10_table[dec_exp - dec_exp_min];
}

// fpfmt algorithm 

// The algorithm for this is given in the "Unrounded Numbers" section of the fpfmt paper (pp 6), and implements "round, half to even".
// the input is a fixed point number with 2 bits after the decimal point, the 2nd of these bits is the "sticky bit" (the bit immediately after the decimal point represents "0.5").
static inline uint64_t uround(uint64_t u) 
{ return (u + 1 + ((u>>2)&1)) >> 2;
}
static inline uint32_t uround32(uint32_t u) 
{ return (u + 1 + ((u>>2)&1)) >> 2;
}

// The algorithm for this is given in the "Unrounded Numbers" section of the fpfmt paper (pp 6), and implements "floor".
// the input is a fixed point number with 2 bits after the decimal point, the 2nd of these bits is the "sticky bit" (the bit immediately after the decimal point represents "0.5").
static inline uint64_t ufloor(uint64_t u) 
{ return (u >> 2);
}
static inline uint32_t ufloor32(uint32_t u) 
{ return (u >> 2);
}

// The algorithm for this is given in the "Unrounded Numbers" section of the fpfmt paper (pp 6), and implements "ceiling".
// the input is a fixed point number with 2 bits after the decimal point, the 2nd of these bits is the "sticky bit" (the bit immediately after the decimal point represents "0.5").
static inline uint64_t uceil(uint64_t u) 
{ return ((u + 3) >> 2);
}
static inline uint32_t uceil32(uint32_t u) 
{ return ((u + 3) >> 2);
}

// The algorithm for this is given in the "Unrounded Numbers" section of the fpfmt paper (pp 6), and implements "nudge".
// the input is a fixed point number with 2 bits after the decimal point, the 2nd of these bits is the "sticky bit" (the bit immediately after the decimal point represents "0.5").
static inline uint64_t unudge(uint64_t u,int delta) 
{ return (u + delta);
}
static inline uint32_t unudge32(uint32_t u,int delta) 
{ return (u + delta);
}
// note gcc uses a divide function for u64/u64 when compiled -m32, for u32/u32 it uses a multiply & shift. For -m64 it always uses a multiply & shift.
 static inline uint64_t div1e8(const uint64_t x) 
 {// equivalent to x/100,000,000 =1e8
  return umul128_hi64(x, 0xABCC77118461CEFDu) >> 26; // this is faster for m32 and no different for m64
 }
 
// constant calculated by maficgu-python3.py
 static inline uint64_t div1e10(const uint64_t x) 
 {// equivalent to x/10,000,000,000 =1e10, note returns uint64_t (could return u32)
  return (uint64_t)(umul128_hi64(x, 0xDBE6FECEBDEDD5BFu) >> (97-64)); // this is faster for m32 and no different for m64 [ shift right 33 ]
 }
 
 static inline uint64_t div10(const uint64_t x) 
 {// equivalent to x/10 
  return umul128_hi64(x,14757395258967641293ull)>>3 ; // this is faster for m32 and no different for m64 constant is 2^64*8/10+1, there is an implicit >>64 as we only take upper 64 bits of the multiply, so the >>3 is needed (divide by 8)
 }
  
// The algorithm for division with a sticky bit is given the the fpfmt paper scetion "Unrounded Numbers", with the generic division algorithm given on pp 7.
static inline uint64_t udiv10(uint64_t u) // returns u/10 with sticky bit set on the result
{ 
 uint64_t u_div_10=div10(u); 
 uint64_t rem=u-u_div_10*10;
 return u_div_10 | (u&1) | (rem != 0);
}

// Computes the decimal exponent as floor(log10(2**bin_exp))
// the fpfmt paper (page 9 log10Pow2), converted to C.
// It uses the approximation (78913/2^18) which is slightly below log10(2), given we want the floor of the result this would seem to be good (its also the value used in ya_sprintf which means its well proven).
// 78913>>18  =0.3010 292 053 2227
// 2620*78913=2.1e8 , 2^31=2.14e9 so result does fit into an int32_t 
static inline int32_t compute_dec_exp(int32_t bin_exp) 
{
 assert(bin_exp >= -1334 && bin_exp <= 2620);
 return (bin_exp * 78913 ) >> 18;
}

// from fpfmt paper, page 9 - log2Pow10 function, converted to C.
// 108853 / 2^15 = 3.3219 29931 , log2(10)=3.3219 2809 
// 350*108853= 3.81e7, 2^31=2.1e9 so the result of dec_exp*108853 easily fits into an int32_t 
static inline int32_t log2Pow10(int32_t x) 
{
 return (x * 108853) >> 15;
}

// from fpfmt paper, page 14 - skewed function, converted to C.
// computes  [log₁₀ 3/4 * 2**e] = [e*(log₁₀ 2)-(log₁₀ 4/3)].
//  2620*631305 = 1.65e9, 2^31=2.14e9 so result does fit into an int32_t 
static inline int32_t skewed(int32_t bin_exp) 
{
 return (bin_exp*631305 - 261663) >> 21;
}

// This structure is the C implementation of that shown in the fpfmt paper "Fast, Accurate Scaling", pp 18
typedef struct Scalers Scalers;
struct Scalers {
	u2_64 pm;// unsigned 128 bit variable
	int s;
};

// prescale() is from the "Fast, Acurate Scaling" section of the fpfmt paper, page 18. The algorithm here has been translated directly into C.
static inline Scalers prescale(int e, int p, int lp) {
	assert(p>=dec_exp_min && p<=dec_exp_max);
	int s = -(e + lp + 3);
	Scalers pre;
	p -= dec_exp_min;
	u2_64 p10=u2_64_pow10_table[p];
	p10.lo+=(p10.lo>0);// change rounding to match that expected by the standard fpfmt algorithm (bounded from above, rather than truncated which is what the table is)	
	pre.pm = p10;
	pre.s = s;
	return pre;
}

// uscale() is from the "Fast, Accurate Scaling" section of the fpfmt paper, page 19, and the algorithm is exactly as described in the paper for the #if 0 case.
// For the #if 1 case below, the algorithm is different to that described in the "Omit Needless Multiplications" section of the fpfmt paper (page 20) in that it uses the same table format as the basic algorithm - but it does give the same result.
// This new algorithm is described by the comments below.
// if compiler supports an unsigned 128 integer type then we use that for the 128*64 bit multiply required below, otherwise we use u2_64 to give a "software" unsigned 128 bit type. 
#if 1 /* if 1  attempts to avoid 2nd 64*64 bit multiply, this makes very little difference nto the execution time on full test program  with 64 bit compiler and ~ 9ns/conversion faster for 32 bit compiler - Winlibs gcc 15.2.0 Intel i3-10100 */

// base algorithm is:
// 	unsigned __int128 full = (unsigned __int128)x * c.pm.hi;
//	uint64_t hi = full>>64;
//	uint64_t mid = full;
//*	uint64_t mid2 = ((unsigned __int128)x * c.pm.lo) >> 64;
//	mid+=mid2;// mid, carry := bits.Add64(mid, mid2, 0)
//	hi+=(mid<mid2); // hi += carry	
//	bool sticky=(mid != 0 || (hi&(((uint64_t)1<<c.s)-1)) != 0);	
//	return (hi>>c.s) | sticky;
//
// To avoid the 2nd multiply (marked with a //* at the start of the line above) mid2 must not change the result ((hi>>c.s) | sticky).
// It might change the result in 3 ways:
//  1)  by generating a carry from mid, carry := bits.Add64(mid, mid2, 0) , with the carry propagating into the final result (hi>>c.s)
//		For this to happen the c.s low order bits of hi must all be ones (these bits will become the highest bits of the result that will be "Ignored" in the return value)
//		This can be detected by the condition (hi&(((uint64_t)1<<c.s)-1)) != 0
//  2)	by changing the value of the sticky bit.
//		The sticky bit is only false if both the lowest c.s bits of hi are all 0 and all the low order bits are also zero.
//		This gives 2 cases to consider:
//		 A) the low order bits of hi are all zero and mid and mid2 are both zero
//		 B) the low order bits of hi are all 1's and mid+mid2 gives zero+carry
//		Case A means the calculation of mid2 will not change the result (as it has to be zero). 
//		Case B is a subset of case 1) so does not need a separate test
// 	3)	If c.s <1 (its an integer so signed) - in this case no bits will be shifted right out of hi, and we must therefore always do the 2nd multiply to calculate the full value of mid=(mid+mid2)
//		 If c.s==0 "mask" will be 0 (see code below) and so (hi & mask)==mask becomes (hi & 0)==0 => 0==0 which is always true, so the 2nd multiply will be correctly done in this case
//		 c.s <0  is trapped by the assert as it should never happen [ c.s is used in shifts where negative shifts are not defined by the C standard ]
//		 So nothing special (apart from assert) is needed for case 3).
//
// Russ Cox, the creator of the fpfmt algorithm has confirmed by email that this approach is correct.
//
// The 64 (if defined(__SIZEOF_INT128__)) and 32 bit C code is given below:
//
 #if defined(__SIZEOF_INT128__) // code for 64 bit compiler
 static inline uint64_t uscale(uint64_t x, Scalers c) 
  { assert(c.s>=0 && c.s <64);
	unsigned __int128 full = (unsigned __int128)x * c.pm.hi;
	uint64_t hi = full>>64;
	uint64_t mid = full;
	const uint64_t mask=(((uint64_t)1<<c.s)-1);// mask for lower bits of hi (ones that will be lost on return hi>>c.s)
	if( (hi&mask) == mask )
		{
		 const uint64_t mid2 = ((unsigned __int128)x * c.pm.lo) >> 64;
		 mid+=mid2;// mid, carry := bits.Add64(mid, mid2, 0)
		 hi+=(mid<mid2); // hi += carry
		}
	bool sticky=(mid != 0 || (hi&mask) != 0);	 
	return (hi>>c.s) | sticky;
  }
 #else // code for 32 bit compiler
 static inline uint64_t uscale(uint64_t x, Scalers c) 
  {
	u2_64 full = u2_64_mult_u64_u64(x, c.pm.hi);//unsigned __int128 full = (unsigned __int128)x * c.pmHi;
	uint64_t hi = full.hi;
	uint64_t mid = full.lo;
	const uint64_t mask=(((uint64_t)1<<c.s)-1);
	if( (hi&mask) == mask  )
		{
		 const uint64_t mid2=u2_64_mult_u64_u64(x, c.pm.lo).hi;// const uint64_t mid2 = ((unsigned __int128)x * c.pmLo) >> 64;
		 mid+=mid2;// mid, carry := bits.Add64(mid, mid2, 0)
		 hi+=(mid<mid2); // hi += carry
		}
	bool sticky=(mid != 0 || (hi&mask) != 0);	 
	return (hi>>c.s) | sticky;
  }
 #endif //defined(__SIZEOF_INT128__)
#else /* algorithm in the fpfmt paper */
/* if compiler supports an unsigned 128 integer type then we use that for the 128*64 bit multiply required below, therwise we use u2_64 to give a "software" unsigned 128 bit type. */ 
 #if defined(__SIZEOF_INT128__) // code for 64 bit compiler
 static inline uint64_t uscale(uint64_t x, Scalers c) 
  {
	unsigned __int128 full = (unsigned __int128)x * c.pm.hi;
	uint64_t hi = full>>64;
	uint64_t mid = full;
	uint64_t mid2 = ((unsigned __int128)x * c.pm.lo) >> 64;
	mid+=mid2;// mid, carry := bits.Add64(mid, mid2, 0)
	hi+=(mid<mid2); // hi += carry	
	bool sticky=(mid != 0 || (hi&(((uint64_t)1<<c.s)-1)) != 0);	
	return (hi>>c.s) | sticky;
 }
 #else // code for 32 bit compiler
 static inline uint64_t uscale(uint64_t x, Scalers c) 
  {
	u2_64 full = u2_64_mult_u64_u64(x, c.pm.hi);//unsigned __int128 full = (unsigned __int128)x * c.pmHi;
	uint64_t hi = full.hi;
	uint64_t mid = full.lo;	
	const uint64_t mid2=u2_64_mult_u64_u64(x, c.pm.lo).hi;// const uint64_t mid2 = ((unsigned __int128)x * c.pmLo) >> 64;
	mid+=mid2;// mid, carry := bits.Add64(mid, mid2, 0)
	hi+=(mid<mid2); // hi += carry	
	bool sticky=(mid != 0 || (hi&(((uint64_t)1<<c.s)-1)) != 0);	
	return (hi>>c.s) | sticky;
 } 
 #endif //defined(__SIZEOF_INT128__)
#endif

#include <stdio.h> // only needed for debug
#include <inttypes.h> /* to print uint64_t */

// version of above  for the case where we have a 32 bit mantissa, This only uses upper 64 bits of entry from power of 10's table above (correctly rounded using lower 64 bits), and a single 32*64bit multiply
 #if defined(__SIZEOF_INT128__) // code for 64 bit compiler - this uses a 64*64=>128 bit multiply (actually its 32*64=>96 bits) which is (just) faster than the two 32*32=>64bit multiplies used with the 32 bit compiler
 static inline uint32_t uscale32(uint32_t x, Scalers c) 
  { assert(c.s>=2 && c.s <32);	 
	/* 32*64 bit multiply as per zmij */
	uint64_t pm_hi=c.pm.hi;
	if( c.pm.lo !=0) pm_hi++;// round up upper 64 bits of power10 value
	unsigned __int128 fullx = (unsigned __int128)x * pm_hi;
	uint64_t hi64=fullx>>32;// upper 64 bits of result
	const uint64_t mask=(((uint64_t)1<<(c.s+32))-1);
	bool sticky=(hi64 & mask)!=0 ;	
	return (hi64>>(c.s+32)) | sticky;
  } 
#else // version avoiding 128 bit multiply - uses two 64 bit multiplies, so good for 32 bit compiler
 static inline uint32_t uscale32(uint32_t x, Scalers c) 
  { assert(c.s>=2 && c.s <32);	 
	uint64_t pm_hi=c.pm.hi;
	if( c.pm.lo !=0) pm_hi++;// round up upper 64 bits of power10 value
	uint32_t pm_hi_h=pm_hi>>32;// upper 32 bits
	uint32_t pm_hi_l=pm_hi;// lower 32 bits
	uint64_t hi64=(uint64_t)x*pm_hi_h;
	uint64_t mid64=(uint64_t)x*pm_hi_l;
	hi64+=mid64>>32;
	const uint64_t mask=(((uint64_t)1<<(c.s+32))-1);
	bool sticky=(hi64 & mask)!=0 ;	
	return (hi64>>(c.s+32)) | sticky;
  }    
 #endif //defined(__SIZEOF_INT128__)
 
static const uint64_t u64powersOf10[]=
				{
					UINT64_C(1), 	// 10^ 0
					UINT64_C(10), 	// 10^1
					UINT64_C(100), 	// 10^2
					UINT64_C(1000),	// 10^3
					UINT64_C(10000),// 10^4
					UINT64_C(100000),// 5
					UINT64_C(1000000),// 6
					UINT64_C(10000000),// 7
					UINT64_C(100000000),// 8
					UINT64_C(1000000000),// 9   
					UINT64_C(10000000000),// 10 
					UINT64_C(100000000000),// 11  
					UINT64_C(1000000000000),// 12   
					UINT64_C(10000000000000),// 13
					UINT64_C(100000000000000),// 14
					UINT64_C(1000000000000000), // 15
					UINT64_C(10000000000000000), // 16 
					UINT64_C(100000000000000000), // 17 
					UINT64_C(1000000000000000000), // 18 
					UINT64_C(10000000000000000000)  // 19  2^64=1.8446744073709551616e+19 so 10^19 should fit into a uint64
				};
static const uint32_t u32powersOf10[] =
  {1, 10, 100, 1000, 10000, 100000,
    1000000, 10000000, 100000000, 1000000000};// 2^32=4,294,967,296 so 1,000,000,000 is the largest possible power of 10, = u32powersOf10[9]
    
// This is the algorithm from the fpfmt paper "Fixed-Width Printing" page 9
static inline void FixedWidth(uint64_t m,int e, int n, uint64_t *dp, int *pp) 
{
 assert(n<sizeof(u64powersOf10)/sizeof(u64powersOf10[0])) ;
 int p = n - 1 - compute_dec_exp(e+63);
 uint64_t u = uscale(m, prescale(e, p, log2Pow10(p)));
 uint64_t d = uround(u);
 // compute_dec_exp() may be out by 1 - correct for that now [ we have 1 more digit than we needed, so we just need a divide by 10 with skicky bit to fix this ]
 if(d >= u64powersOf10[n]) 
	{
	 d = uround(udiv10(u));
	 p--;
	}
 *dp = d;
 *pp = -p;
}	

// returns the number of decimal digits in d 
// fpfmt paper page 23
static inline uint16_t Digits(uint64_t d)
{
 int nd = compute_dec_exp(64-ya_clz(d));// log10Pow2(bits.Len64(d))
 return nd + (d >= u64powersOf10[nd]);
}
static inline uint16_t Digits32(uint32_t d)
{
 int nd = compute_dec_exp(32-ya_clz32(d));
 return nd + (d >= u32powersOf10[nd]);
}

// convert 64 mantissa to at most 17 digits, and typically 16 digits
// This is fast for 17 and 16 digits but gets progressively slower as nd reduces (as it always converts at least 8 digits), if lower values of nd are likely mantissa64_to_string() below is faster.
// uses SWAR BCD converter (as above)
// always copies 16 or 17 digits to dst - there should always be this much room 
// It does NOT terminate the resultant string with a \0
static void inline mantissa64_to_stringBCD(char *dst, uint64_t d64, int nd)
{assert(nd>=0 && nd<=17);
 const uint64_t zeros = 0x0101010101010101u * '0';
 // Each digit is denoted by a letter so value is abbccddeeffgghhii.
 uint32_t abbccddee = (uint32_t)div1e8(d64);// 100,000,000 =1e8
 uint32_t ffgghhii =(uint32_t)(d64-(uint64_t)abbccddee*100000000); // may be faster than abbccddee%100000000 - in most (but not all) cases the compiler will do this when given the % operator.
 if(nd==17)
 	{*dst++=(abbccddee /   100000000)+'0';// creates MSDigit, abbccddee is uint32_t, so / & % should be efficient.
 	 abbccddee %= 100000000;
 	 nd=16;
 	}
 u2_64 bcd128;// 16 BCD digits (needed to allow us to remove leading zero's based on nd below);
 bcd128.hi = abbccddee==0?0:ya_to_BCD8(abbccddee);// have already removed "a" if it was present, might be zero for example if nd=8
 bcd128.lo =  ffgghhii==0?0:ya_to_BCD8( ffgghhii);// check for zero - relatively unlikley but a simple check
 // now remove leading zero's based on nd
 bcd128=lshift_u2_64(bcd128,((16-nd)<<3));
 bcd128.hi= is_big_endian() ? bcd128.hi | zeros: bswap64(bcd128.hi |zeros);// convert to ascii chars and if necessary swap order ready for memcpy to dst    
 bcd128.lo= is_big_endian() ? bcd128.lo | zeros: bswap64(bcd128.lo |zeros);// convert to ascii chars and if necessary swap order ready for memcpy to dst 
 memcpy(dst,&bcd128.hi,8);// always write 16/17 bytes
 memcpy(dst+8,&bcd128.lo,8);
}

/* like above but returns nd adjusted to remove trailing zero's */
static int inline mantissa64_to_stringBCD_notrailing0(char *dst, uint64_t d64, int nd)
{assert(nd>0 && nd<=17);
 const uint64_t zeros = 0x0101010101010101u * '0';
 // Each digit is denoted by a letter so value is abbccddeeffgghhii.
 uint32_t abbccddee = (uint32_t)div1e8(d64);// 100,000,000 =1e8
 uint32_t ffgghhii =(uint32_t)(d64-(uint64_t)abbccddee*100000000); // may be faster than abbccddee%100000000 - in most (but not all) cases the compiler will do this when given the % operator.
 if(nd==17)
 	{*dst++=(abbccddee /   100000000)+'0';// creates MSDigit, abbccddee is uint32_t, so / & % should be efficient.
 	 abbccddee %= 100000000;
 	 nd=16;
 	}
 u2_64 bcd128;// 16 BCD digits (needed to allow us to remove leading zero's based on nd below);
 bcd128.hi = abbccddee==0?0:ya_to_BCD8(abbccddee);// have already removed "a" if it was present, might be zero for example if nd=8
 bcd128.lo =  ffgghhii==0?0:ya_to_BCD8( ffgghhii);// check for zero - relatively unlikley but a simple check
 // look for trailing zero's by counting trailing zero bits [8 bits per character]
 int tz=bcd128.lo==0?8+(ya_ctz64(bcd128.hi)>>3) :  ya_ctz64(bcd128.lo)>>3;
 // now remove leading zero's based on nd
 bcd128=lshift_u2_64(bcd128,((16-nd)<<3));
 bcd128.hi= is_big_endian() ? bcd128.hi | zeros: bswap64(bcd128.hi |zeros);// convert to ascii chars and if necessary swap order ready for memcpy to dst    
 memcpy(dst,&bcd128.hi,8);// always write 8 (or 9) bytes
 if(bcd128.lo)
 	{// optionally write another 8 bytes if they are not zero (if they were zero will have been removed by trailing zero code)
 	 bcd128.lo= is_big_endian() ? bcd128.lo | zeros: bswap64(bcd128.lo |zeros);// convert to ascii chars and if necessary swap order ready for memcpy to dst 
	 memcpy(dst+8,&bcd128.lo,8);
	}
 return nd-tz;
}

// Short computes the shortest formatting of f,
// using as few digits as possible that will still round trip
// back to the original double.
// from fpfmt paper page 15, algorithm converted to C by Peter Miller
// This version takes a different approach to the fpfmt paper on trailing zero removal, rather than do this in a seperate function, its done as part of the double->ascii conversion
// The logic for this change is that the trailing zero logic effectively looks at digits being zero which ~ the same effort as binary->decimal conversion (which stil has to be done), so merging them gives simpler, faster code.
// Two binary->decimal convertors are used - one of 16/17 digits (where no trailing zero are present) and one that removes trailing zero's. There is only a small gain in this, but the code naturally splits into these two path's
// Returns the number of digits (after trailing zero removal).
static int Short_d(uint64_t ieeeMantissa,uint32_t ieeeExponent, char *dst, int32_t *p) 
{
 const int32_t minExp = -(DOUBLE_EXPONENT_BIAS+DOUBLE_EXPONENT_BITS+DOUBLE_MANTISSA_BITS-1);// -1085, e > this is "normal"
 uint64_t m;// base 2 mantissa
 int32_t e,pv;// pv will become *p (base 10 exponent), e is base 2 exponent
 int32_t SieeeExponent;
 if (ieeeExponent == 0) 
 	{// subnormals
	 SieeeExponent = 1-(DOUBLE_EXPONENT_OFFSET);// exponent for subnormals is offset by 1
	 int s = ya_clz(ieeeMantissa); // normalise so msb is 1, and adjust exponent to match
	 ieeeMantissa <<= s;
	 SieeeExponent -= s;
	}  
  else // normals
 	{ieeeMantissa= (ieeeMantissa | DOUBLE_IMPLICIT_BIT  )<<DOUBLE_EXPONENT_BITS;// put back "hidden bit" and shift so msbit is set
 	 SieeeExponent=(int32_t)ieeeExponent-(DOUBLE_EXPONENT_OFFSET+DOUBLE_EXPONENT_BITS);  // adjust exponent for bias and shift on line above
	}
 m=ieeeMantissa;
 e=SieeeExponent;
 uint64_t min,max,dmin,dmax,dv;// dv will become *d
 int32_t z = DOUBLE_EXPONENT_BITS; // number of bits in exponent (as we normalised mantissa these are now "spare" at the ls end of the mantissa)
 if(m == (uint64_t)1<<63 && e > minExp )
 	{// "normal" value
 	 pv = -skewed(e + z);
	 min = m - ((uint64_t)1<<(z-2)); // min = m - 1/4 * 2**(e+z)
	} 
 else 
 	{
	 if( e < minExp) 
	 	{// "denormal"
		 z = DOUBLE_EXPONENT_BITS + (minExp - e);
		}	
	 pv = -compute_dec_exp(e + z);   // -log10Pow2(e + z)
	 min = m - ((uint64_t)1<<(z-1)); // min = m - 1/2 * 2**(e+z)
	}
 max = m + ((uint64_t)1<<(z-1)); // max = m + 1/2 * 2**(e+z)
 int odd = (m>>z) & 1;
 Scalers pre = prescale(e, pv, log2Pow10(pv));
 dmin = uceil(unudge(uscale(min, pre),+odd));
 dmax = ufloor(unudge(uscale(max, pre),-odd));
 dv=div10(dmax); // dmax/10;
 int dlen;
 if ( dv*10>= dmin )
 	{
	 dlen=Digits(dv);
 	 pv= -(pv - 1);
 	 int newlen=mantissa64_to_stringBCD_notrailing0(dst, dv, dlen);// convert to ascii and remove trailing zero's
 	 pv+=dlen-newlen; // adjust 10's exponent to compensate for trailing zero's removed
 	 *p=pv;
	 return newlen; 
	}
 dv=dmin;
 if ( dv < dmax )
 	{
 	 dv = uround(uscale(m, pre));
	}
 dlen=Digits(dv); 
 mantissa64_to_stringBCD(dst, dv, dlen);
 *p=-pv;
 return dlen;// no trailing zeros
}



/* convert 32 bit unsigned integer to ascii with nd digits AND delete trailing zero's - returns new value for nd
   nd must be >0 and <=8 if its larger than the actual number of digits then leading zero's will be created.
    The upper limit of 8 comes from the fact that it's only called when a trailing zero gas already been found and removed so that takes us from 9 to 8 digits max.
   It does NOT terminate the resultant string with a \0
   This function uses an 8 digit BCD SWAR conversion, with embedded trailing zero removal
   It always writes 8 characters as this is faster - this is not an issue as buffer should always be large enough for 8 characters.
   Its slightly faster if mantissa32_to_string_notrailing0() is not marked "inline"
*/
static int mantissa32_to_string_notrailing0(char *dst, uint32_t d32, int nd)
{assert(nd>0 && nd<=8);
 // convert to 8 digit BCD using SWAR code 
 const uint64_t zeros = 0x0101010101010101u * '0';
 uint64_t a_b_c_d_e_f_g_h=ya_to_BCD8(d32);  
 // finished conversion to BCD 
 // look for trailing zero's by counting trailing zero bits [8 bits per character]
 int tz=ya_ctz64(a_b_c_d_e_f_g_h)>>3; 
 // now remove leading zero's based on nd
 a_b_c_d_e_f_g_h <<=((8-nd)<<3);
 a_b_c_d_e_f_g_h= is_big_endian() ? a_b_c_d_e_f_g_h |zeros : bswap64(a_b_c_d_e_f_g_h | zeros);// convert to ascii chars and if necessary swap order ready for memcpy to dst
 memcpy(dst,&a_b_c_d_e_f_g_h,8); // could be 8 rather than nd-tz if that's faster 8=>32.2ns nd-tz=>32.5ns
 // printf(" mantissa32_to_string_notrailing0(%u,%d) gives 0X%016"PRIx64" as a string this is \"%.8s\" return length=%d so return string is \"%.*s\"\n",d32,nd,a_b_c_d_e_f_g_h,dst,nd-tz,nd-tz,dst);
 return nd-tz;
}

// convert 32 mantissa to at most 9 digits, and typically 8 digits
// This is fast for 9 and 8 digits but gets progressively slower as nd reduces (as it always converts at least 8 digits), if lower values of nd are likely mantissa32_to_string() below is faster.
// uses SWAR BCD converter (as above)
// always copies 8 or 9 digits to dst - there should always be this much room 
// It does NOT terminate the resultant string with a \0

static void inline mantissa32_to_stringBCD(char *dst, uint32_t d32, int nd)
{assert(nd>=0 && nd<=9);
 if (nd>8) 
	{/* must be 9 , convert 1 here */
	 nd=8;// must be 8 left
	 uint32_t d32_1e8=d32/100000000;
	 d32 -=  d32_1e8*100000000;// x = d32 % 100000000;
	 *dst++=d32_1e8+'0';
	}
 // now have 8 or less, always convert 8 using BCD SWAR
 const uint64_t zeros = 0x0101010101010101u * '0';
 uint64_t a_b_c_d_e_f_g_h = ya_to_BCD8(d32); 
 // now remove leading zero's based on nd
 a_b_c_d_e_f_g_h <<=((8-nd)<<3);
 a_b_c_d_e_f_g_h= is_big_endian() ? a_b_c_d_e_f_g_h | zeros: bswap64(a_b_c_d_e_f_g_h |zeros);// convert to ascii chars and if necessary swap order ready for memcpy to dst    
 memcpy(dst,&a_b_c_d_e_f_g_h,8);  // could just copy nd (adjusted for possible 9th digit) - but always doing 8 is faster on average
}

/* convert 32 bit unsigned integer to ascii with nd digits 
   nd must be >=0 and <=10 if its larger than the actual number of digits then leading zero's will be created.
   It is efficient for all values of nd, but if nd is almost always 8 or 9 then mantissa32_to_stringBCD() above is faster.
   [ it can be called from mantissa64_to_string() with nd=0, but note it does not convert anything in this case ]
   2^32=4,294,967,296 which has 10 digits
   As all the maths is 32 bit the compiler will change / and % to multiplies and shifts (even when compiled for a 32 bit target)
   It does NOT terminate the resultant string with a \0
   This function uses a binary pattern when creating the digits:
   	 convert  8
   	 convert  4
   	 convert  2
   	 convert  1
*/
static void inline mantissa32_to_string(char *dst, uint32_t d32, int nd)
{assert(nd>=0 && nd<=10);
 if (nd>=8) 
	{/* convert 8 digits  */
	 uint32_t d32_1e8=d32/100000000;
	 uint32_t x = d32 - d32_1e8*100000000;// x = d32 % 100000000;
	 d32=d32_1e8;// d32 /= 100000000;	 
	 nd-=8;
	 // convert 8 using BCD SWAR - this is ~ 1.5ns/test faster at w64 and same speed as ya_uitoa8() for w32 
	 const uint64_t zeros = 0x0101010101010101u * '0';
	 uint64_t a_b_c_d_e_f_g_h = ya_to_BCD8(x); 
	 a_b_c_d_e_f_g_h= is_big_endian() ? a_b_c_d_e_f_g_h | zeros: bswap64(a_b_c_d_e_f_g_h |zeros);// convert to ascii chars and if necessary swap order ready for memcpy to dst    
	 memcpy(dst+nd,&a_b_c_d_e_f_g_h,8);   
	}
 if(nd>=4) 
	{/* convert 4 digits */
	 uint32_t x = d32 % 10000;// note the compiler will convert these to * and shift (and give us d/=10000 at the same time). 
	 d32 /= 10000;
	 nd-=4;
	 ya_uitoa4(dst+nd, x);
	}		
// here we have 3 digits (at most) left	
 if(nd>=2) // after this step we have 1 or 0 digits left
	{
	 uint32_t x = d32 % 100;// note the compiler will convert these to * and shift (and give us d/=100 at the same time). 
	 d32 /= 100;
	 nd-=2;
	 ya_uitoa2(dst+nd, x);
	}	
 if (nd==1)  *dst = '0' + d32;// final digit 0-9 (msd) - its trivial to convert this.	
}

/* Short_f a version of Short_d that works on float's rather than doubles
   Created by Peter Miller 31-3-2026 */
// This version returns an that specifies the length of the returned string (string in dst).
static int Short_f(uint32_t ieeeMantissa,uint32_t ieeeExponent, char *dst, int32_t *p) 
{
 const int32_t minExp = -(FLOAT_EXPONENT_BIAS+FLOAT_EXPONENT_BITS+FLOAT_MANTISSA_BITS-1);//  e > this is "normal"
 uint32_t m;// base 2 mantissa
 int32_t e,pv;// pv will become *p (base 10 exponent), e is base 2 exponent
 int32_t SieeeExponent;
 if (ieeeExponent == 0) 
 	{// subnormals
	 SieeeExponent = 1-(FLOAT_EXPONENT_OFFSET);// exponent for subnormals is offset by 1
	 int s = ya_clz32(ieeeMantissa); // normalise so msb is 1, and adjust exponent to match 
	 ieeeMantissa <<= s;
	 SieeeExponent -= s;
	}  
  else // normals
 	{ieeeMantissa= (ieeeMantissa | FLOAT_IMPLICIT_BIT  )<<FLOAT_EXPONENT_BITS;// put back "hidden bit" and shift so msbit is set
 	 SieeeExponent=(int32_t)ieeeExponent-(FLOAT_EXPONENT_OFFSET+FLOAT_EXPONENT_BITS);  // adjust exponent for bias and shift on line above
	}
 m=ieeeMantissa;
 e=SieeeExponent;
 uint32_t min,max,dmin,dmax,dv;// dv will become *d
 int32_t z = FLOAT_EXPONENT_BITS; // number of bits in exponent (as we normalised mantissa these are now "spare" at the ls end of the mantissa)
 if(m == (uint32_t)1<<31 && e > minExp )
 	{// "normal" value
 	 pv = -skewed(e + z);
	 min = m - ((uint64_t)1<<(z-2)); // min = m - 1/4 * 2**(e+z)
	} 
 else 
 	{
	 if( e < minExp) 
	 	{// "denormal"
		 z = FLOAT_EXPONENT_BITS + (minExp - e);
		}	
	 pv = -compute_dec_exp(e + z);   // -log10Pow2(e + z)
	 min = m - ((uint32_t)1<<(z-1)); // min = m - 1/2 * 2**(e+z)
	}
 max = m + ((uint32_t)1<<(z-1)); // max = m + 1/2 * 2**(e+z)
 int odd = (m>>z) & 1;
 Scalers pre = prescale(e, pv, log2Pow10(pv));
 dmin = uceil32(unudge32(uscale32(min, pre),+odd));
 dmax = ufloor32(unudge32(uscale32(max, pre),-odd));
 dv=dmax/10; // dmax/10;
 int dlen;
 if ( dv*10>= dmin )
 	{
 	 dlen=Digits32(dv);
 	 pv= -(pv - 1);
 	 int newlen=mantissa32_to_string_notrailing0(dst, dv, dlen);// convert to ascii and remove trailing zero's
 	 pv+=dlen-newlen; // adjust 10's exponent to compensate for trailing zero's removed
 	 *p=pv;
	 return newlen; 
	}
 dv=dmin;
 if ( dv  < dmax )
 	{
 	 dv = uround32(uscale32(m, pre));
	}
 dlen=Digits32(dv); 
 mantissa32_to_stringBCD(dst, dv, dlen);	//  could be mantissa32_to_string() or mantissa32_to_stringBCD(). "BCD" version is on average ~ 1ns/test faster for both w32 & w64
 *p=-pv;
 return dlen;// no trailing zeros
}





/* 	Convert a uint64_t to ascii with nd digits
    nd must be >=1 and <=18 if its larger than the actual number of digits then leading zero's will be created.
   	Extracts the 1st 10 digits then uses mantissa32_to_string() [ above] to convert the remaining digits
	It does NOT terminate the resultant string with a \0
	2^64=18,446,744,073,709,551,616 which has 20 digits 
	The 1st step divides by 1,000,000,000 giving a max of  18,446,744,073 for mantissa32_to_string() which takes a 32 bit argument which can be at most 4,294,967,296 => meaning this is not a generic 64 bit conversion 
	being limited to 4,294,967,295,000,000,000 which is just over 18 digits. 
	In its use in ya-dconvert 18 digits is the maximum its called with so this is not an issue.
	If this is ever a problem the initial "if" could be changed to a while()
	It's slightly faster if this function is not marked "inline".
*/
static void mantissa64_to_string(char *dst, uint64_t d64, int nd) 
{assert(nd>0 && nd<=18);
 if (d64>= 1000000000) 
	{/* quickly convert 1st 10 digits */
	 uint64_t d64_1e10=div1e10(d64); // d64/10000000000 implemented using a multiply & shift as gcc 15.2.0 does not do this automatically if compiled for a 32 bit target. This Has no speed impact for -m64 and saves 2ns for -m32	 
	 uint64_t x = d64 - d64_1e10*10000000000;// x = d64 % 10000000000;
	 d64=d64_1e10;// d64 /= 10000000000;	 
	 nd-=10;
	 ya_uitoa10(dst+nd,x);// (quickly) convert 10 digits to ascii	
	}
 // now we can swap to a uint32_t which is faster as %,/ get converted to * and shifts in both 32 and 64 bit compilers.  2^32 is 4,294,967,296 while we compare with 1e9=1,000,000,000 in the line above ensuring a u32 is OK.
 mantissa32_to_string(dst,(uint32_t)d64,nd); // note mantissa32_to_string() is marked "inline" so there is no over head in making this function call. Note we cannot call mantissa32_to_stringBCD() here as that might overwrite result from ya_uitoa10().	
}



// #define DEBUG_DCONV

// Interface function to ya_sprintf() 
//
// int32_t *decimal_pos is set to the (decimal) exponent
// result in *buffer, has just numbers (no decimal point, but its assumed to be after the 1st digit) and no exponent (that's returned in *decimal_pos)
// no checks for special case (nan, inf, etc) required here as they are dealt with by ya_sprintf() before calling this function
// sign is also not dealt with here - its assumed input is positive 
// params must previously have been calculated as:
//	uint64_t ieeeMantissa = bits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
//	uint32_t ieeeExponent = (uint32_t) ((bits >> DOUBLE_MANTISSA_BITS) & ((1u << DOUBLE_EXPONENT_BITS) - 1));
// returns number of digits in mantissa (can be less than requested by precision if the rest are all 0)
int ya_d2exp_buffered_n_ya_sprintf(uint64_t ieeeMantissa,uint32_t ieeeExponent, uint32_t precision, char* buffer,int32_t *decimal_pos) 
{
#ifdef DEBUG_DCONV
  printf("d2exp_buffered_n_ya_sprintf(mant=0X%llx,exp=%u,prec=%u) called\n",ieeeMantissa,ieeeExponent,precision);
#endif
  int32_t SieeeExponent;
  if (ieeeExponent == 0) 
 	{// subnormals
	 SieeeExponent = 1-(DOUBLE_EXPONENT_OFFSET);// exponent for subnormals is offset by 1
	 int s = ya_clz(ieeeMantissa); // normalise so msb is 1, and adjust exponent to match
	 ieeeMantissa <<= s;
	 SieeeExponent -= s;
	}  
  else // normals
 	{ieeeMantissa= (ieeeMantissa | DOUBLE_IMPLICIT_BIT  )<<DOUBLE_EXPONENT_BITS;// put back "hidden bit" and shift so msbit is set
 	 SieeeExponent=(int32_t)ieeeExponent-(DOUBLE_EXPONENT_OFFSET+DOUBLE_EXPONENT_BITS);  // adjust exponent for bias and shift on line above
	}	   	
#ifdef DEBUG_DCONV
  printf(" after normalisation mant=0X%llx exp=%d\n",ieeeMantissa,SieeeExponent);
#endif  	
 // static inline void FixedWidth(uint64_t m,int e, int n, uint64_t *dp, int *pp) 
 uint64_t d;// decimal mantissa
 int p;	 // decimal exponent
 int n=precision+1<=18?precision+1:18; // n is total number of digits required , precision is unsigned so n>=1, n is also limited to 18 which is also enforced here	
 FixedWidth(ieeeMantissa,SieeeExponent, n, &d, &p) ;// convert to base 10 mantissa d and exponent p
#ifdef DEBUG_DCONV
  printf(" after FixedWidth mant10=%llu exp10=%d\n",d,p);
#endif   
 mantissa64_to_string(buffer, d, n);// need to limit nos digits to <=18
 p += n - 1;// 1 digit before decimal point
 *decimal_pos=p;
 return n ;// nos digits created
}	


/* user function that can be directly called to convert double -> string, , returns the same result as sprintf(dst,"%.*e",prec,f) 
*/
char * ya_dconvert_efmt(char *dst, double f, uint32_t prec)  /* prec is required number of digits after decimal point , returns pointer to trailing 0 in string */
{
 const union {
	double real_d;
	uint64_t bits_d;
  } u = { f };
	// Decode bits into sign, mantissa, and exponent.
 const bool vsign = (u.bits_d  & ((uint64_t)1<<63)) != 0;// true if v is negative 1<<63 as sign bit is the msb 
 uint64_t mantissa = u.bits_d & 0xFFFFFFFFFFFFFULL;
 int expo=(int)((u.bits_d>>52) & 0x7ff) ; 
 if(expo==0x7ff)
	{// nan or inf
	 if(vsign) *dst++='-';
	 if(mantissa) // nan
		{ if(u.bits_d==0xFFF8000000000000ULL)
			{//strcpy(dst,"nan(ind)");
			 memcpy(dst,"nan(ind)",9); // 9 includes trailing 0
			 dst+=8;
			}
		  else if( (mantissa & (1ull << 51)) == 0)
			 {//strcpy(dst,"nan(snan)");
			  memcpy(dst,"nan(snan)",10);// 10 includes trailing 0
			  dst+=9;
			 }
		 else
		 	{// strcpy(dst,"nan");
		 	 memcpy(dst,"nan",4);// 4 includes trailing 0
			 dst+=3;
		 	}
		}
	 else 
	 	{//strcpy(dst,"inf");
		 memcpy(dst,"inf",4);// 4 includes trailing 0
		 dst+=3;	 	
	 	}
	 return dst;
	}	 	
 if(expo==0 && mantissa==0)
	{// zero is special as we cannot scale it into range : return 0.00000e00
	 //printf("v (%g) == zero!\n",v);
	 if(vsign) *dst++='-';
	 *dst++='0';
	 if(prec>0) *dst++='.';
	 for(int i=1;i<=prec;++i)
		*dst++='0';
	 //strcpy(dst,"e+00");
	 memcpy(dst,"e+00",5);// 5 includes trailing 0
	 dst+=4;	 
	 return dst;
	}	

 if(vsign) 
	{*dst++='-';
	}	
 // int ya_d2exp_buffered_n_ya_sprintf(uint64_t ieeeMantissa,uint32_t ieeeExponent, uint32_t precision, char* buffer,int32_t *decimal_pos) 
 int32_t tens; // decimal exponent
 int dlen=ya_d2exp_buffered_n_ya_sprintf(mantissa,expo,prec,dst+1,&tens);// This function actually does all the hard work.
 *dst=dst[1];// make space for decimal point
 dst[1]='.'; // add in decimal point (this is wasted if prec==0, but avoids any conditional code)
 dst+=dlen+(prec>0);
 // add extra trailing zero's if required
 while(dlen<prec+1) {*dst++='0';dlen++;}
 // have written mantissa, now deal with exponent
 // 1st output sign of exponent (+/-) - I tried the "branchless" approach from zmij and it was slightly slower when used here
 *dst++='e';
 if(tens<0)
 	{*dst++='-';
 	 tens= -tens;
 	}
 else *dst++='+';
 // now print exp - this is the fastest of many methods I tried
// 3 digits of exponent for double - max exponent is +308, min -324 - always print at least 2 digits 
 if(tens>=100)
 	{*dst++='0'+tens/100;// 3rd digit if required
 	 tens%=100;
 	}
 ya_uitoa2(dst,tens);// other 2 digits of exponent
 dst+=2;
 *dst=0; // terminate string
 return dst;
}

/* user function that can be directly called to convert double -> string, returns the same result as sprintf(dst,"%.*g",prec,f) 
   C23 standard says for %g:
	   A double argument representing a floating-point number is converted in style f or e, depending on the value converted
	and the precision. Let P equal the precision if nonzero, 6 if the precision is omitted, or 1 if
	the precision is zero. Then, if a conversion with style E would have an exponent of X:
	if P > X ≥ −4, the conversion is with style f  and precision P − (X + 1).
	otherwise, the conversion is with style e  and precision P − 1.
	Finally, any trailing zeros are removed from the fractional portion
	of the result and the decimal-point character is removed if there is no fractional portion
	remaining.
	A double argument representing an infinity or NaN is converted in the style of an f or F
	conversion specifier.

 In this function prec is signed and a negative number is taken as meaning the default (6) is required
*/
char * ya_dconvert_gfmt(char *dst, double f, int32_t prec)  /* prec is required number of digits , returns pointer to trailing 0 in string   */
{
 const union {
	double real_d;
	uint64_t bits_d;
  } u = { f };
	// Decode bits into sign, mantissa, and exponent.
 const bool vsign = (u.bits_d  & ((uint64_t)1<<63)) != 0;// true if v is negative 1<<63 as sign bit is the msb 
 uint64_t mantissa = u.bits_d & 0xFFFFFFFFFFFFFULL;
 int expo=(int)((u.bits_d>>52) & 0x7ff) ; 
 if(expo==0x7ff)
	{// nan or inf
	 if(vsign) *dst++='-';
	 if(mantissa) // nan
		{ if(u.bits_d==0xFFF8000000000000ULL)
			{//strcpy(dst,"nan(ind)");
			 memcpy(dst,"nan(ind)",9); // 9 includes trailing 0
			 dst+=8;
			}
		  else if( (mantissa & (1ull << 51)) == 0)
			 {//strcpy(dst,"nan(snan)");
			  memcpy(dst,"nan(snan)",10);// 10 includes trailing 0
			  dst+=9;
			 }
		 else
		 	{// strcpy(dst,"nan");
		 	 memcpy(dst,"nan",4);// 4 includes trailing 0
			 dst+=3;
		 	}
		}
	 else 
	 	{//strcpy(dst,"inf");
		 memcpy(dst,"inf",4);// 4 includes trailing 0
		 dst+=3;	 	
	 	}
	 return dst;
	}	 	
 if(expo==0 && mantissa==0)
	{// zero is special as we cannot scale it into range : return [-]0
	 //printf("v (%g) == zero!\n",v);
	 if(vsign) memcpy(dst,"-0",3);// 3 to include null
	 else memcpy(dst,"0",2);
	 return dst+1+vsign;
	}	

 if(vsign) 
	{*dst++='-';
	}	
 // int ya_d2exp_buffered_n_ya_sprintf(uint64_t ieeeMantissa,uint32_t ieeeExponent, uint32_t precision, char* buffer,int32_t *decimal_pos) 
 int32_t tens; // decimal exponent
 if(prec==0) prec=1; //  %g has a default precision of 6, and 0=>1
 else if(prec<0) prec=6;
 int dlen=ya_d2exp_buffered_n_ya_sprintf(mantissa,expo,prec-1,dst+1,&tens);// This function actually does all the hard work. Prec-1 as prec for %g is number of digits, whereas for %e format prec is number of digits after decimal point
 if(prec>tens && tens>=-4 ) // C standard says prec>tens>=-4 see whole text from standard above 
 	{// need to output in fixed point format as sprintf("%.*f",prec-(tens+1),f)
 	*dst='X';// guarantee remove trailing zero loop below terminates (1st digit should always be <>0 as zero is trapped elsewhere, so this should not be necessary). Mantissa is at dst+1 onwards
 	// remove trailing zero's [ in some cases below we add back zero's which we may have removed here, but trapping that specific case would make the code more complex ]
 	while(dst[dlen]=='0') dlen--; // will eventually terminate on a non-zero digit (or 'X'); Using X as a sentinel is 0.2ns/conversion faster than using while(dlen>0 && dst[dlen]=='0')
 	if(tens==0)
 		{// simple case x.xxx with no exponent
		*dst=dst[1];// make space for decimal point
 		 dst[1]='.'; // add in decimal point (this is wasted if prec==0, but avoids any conditional code)
 		 dst+=dlen+(dlen>1);// need dp if more than 1 digit 
	 	 *dst=0; return dst;
	 	}
	 else if(tens>0)
	 	{ // don't need exponent, but need to put dp in correct place - will create something like "10.12"
	 	 int minl=tens+1>dlen?dlen:tens+1;
	 	 memmove(dst,dst+1,minl); // memmove allows overlapping areas
	 	 dst[minl]='.';
	 	 if(dlen<=tens) 
		  	{memset(dst+minl,'0',tens+1-dlen);// 1e1 =>10 so needs extra zero's adding
		  	 dst+=tens+1;
		  	}
		 else dst+=dlen+(dlen>1+tens);// need dp if a digit after dp
	 	 *dst=0; return dst;
	 	}
	 // tens < 0, again don't need exponent but need leading zero's after dp.
	 memmove(dst+(-tens)+1,dst+1,dlen);// move right 
	 *dst='0';
	 dst[1]='.';
	 memset(dst+2,'0',-tens-1);
	 dst+=dlen+(-tens)+1; // +1 for dp
	 *dst=0; return dst;
	}
 // if we get here result wil be in %e format
 *dst=dst[1];// make space for decimal point
 dst[1]='.'; // add in decimal point (this is wasted if we end up with just 1 digit, but avoids any conditional code)
 // remove trailing zero's
 while(dst[dlen]=='0') dlen--; // will eventually terminate on decimal point or a non-zero digit
 dst+=dlen+(dlen>1);// dlen>1 adds space for decimal point 
 // have written mantissa, now deal with exponent
 // 1st output sign of exponent (+/-) - I tried the "branchless" approach from zmij and it was slightly slower when used here
 *dst++='e';
 if(tens<0)
 	{*dst++='-';
 	 tens= -tens;
 	}
 else *dst++='+';
 // now print exp - this is the fastest of many methods I tried
// 3 digits of exponent for double - max exponent is +308, min -324 - always print at least 2 digits 
 if(tens>=100)
 	{*dst++='0'+tens/100;// 3rd digit if required
 	 tens%=100;
 	}
 ya_uitoa2(dst,tens);// other 2 digits of exponent
 dst+=2;
 *dst=0; // terminate string
 return dst;
}

#define REAL_SHORTEST /* if defined print shortest string that round-loops correctly using fixed point or exponential format whichever is shorter. if not defined always print as x.xxe+/-EE but with as few as possible characters in the mantissa */
	/* Note if  REAL_SHORTEST is defined it gives results similar to print's %g format, however the exponent will never have a plus sign and 1 digit exponents are used to save characters */
	/* this define only effects ya_shortd() and ya_shortf() */
	
/* This function gives double->"shortest string"  */
char * ya_shortd(char *dst, double f) /* returns pointer to trailing 0 in string */
{
 union {
	double real_d;
	uint64_t bits_d;
  } u = { f };
	// Decode bits into sign, mantissa, and exponent.
 const bool vsign = (u.bits_d  & ((uint64_t)1<<63)) != 0;// true if v is negative 1<<63 as sign bit is the msb 
 uint64_t mantissa = u.bits_d & 0xFFFFFFFFFFFFFULL;
 int expo=(int)((u.bits_d>>52) & 0x7ff) ; 
 if(expo==0x7ff)
	{// nan or inf
	 if(vsign) *dst++='-';
	 if(mantissa) // nan
		{ if(u.bits_d==0xFFF8000000000000ULL)
			{//strcpy(dst,"nan(ind)");
			 memcpy(dst,"nan(ind)",9); // 9 includes trailing 0
			 dst+=8;
			}
		  else if( (mantissa & (1ull << 51)) == 0)
			 {//strcpy(dst,"nan(snan)");
			  memcpy(dst,"nan(snan)",10);// 10 includes trailing 0
			  dst+=9;
			 }
		 else
		 	{// strcpy(dst,"nan");
		 	 memcpy(dst,"nan",4);// 4 includes trailing 0
			 dst+=3;
		 	}
		}
	 else 
	 	{//strcpy(dst,"inf");
		 memcpy(dst,"inf",4);// 4 includes trailing 0
		 dst+=3;	 	
	 	}
	 return dst;
	}	 	
 if(expo==0 && mantissa==0)
	{// zero is special as we cannot scale it into range : return 0
	 //printf("v (%g) == zero!\n",v);
	 if(vsign) memcpy(dst,"-0",3);// 3 to include null
	 else memcpy(dst,"0",2);
	 return dst+1+vsign;
	}	

 if(vsign) 
	{*dst++='-';
	}	
 // end of handling for nan etc	
 int32_t tens;// base 10 exponent
 int dlen=Short_d(mantissa,expo, dst+1, &tens);// short_d gives us a string containing the mantissa at dst+1

 // have written mantissa, now deal with exponent
 #ifdef REAL_SHORTEST
  // This code does create the actual shortest length representation using either fixed point of exponential format - exponential format does not use a + sign for exponent and can print only 1 exponent digit if thats all that's required.
 // At this point mantissa is at dst+1 and has dlen digits
 // This is close to %g format, but omits the exponent sign if its positive and can use a single digit for the exponent it that's all that's required (true %g would print a plus sign for the exponent and print at least 2 digits for the exponent)
 tens+=dlen-1;// only 1 digit before dp - gives us equivalent exponent for %e format
 if((tens>-3 && tens<=0) || (tens>0 && tens<=dlen+1+(tens>9)) ) // for Short_f last item is tens<=dlen+1, but for double there may be a 2 digit exponent at this point so need extra +(tens>9) to reflect this
 	{// conditions above taken from C printf %g 
 	if(tens==0)
 		{// simple case x.xxx with no exponent
		*dst=dst[1];// make space for decimal point
 		 dst[1]='.'; // add in decimal point (this is wasted if prec==0, but avoids any conditional code)
 		 dst+=dlen+(dlen>1);// need dp if more than 1 digit 
	 	 *dst=0; return dst;
	 	}
	 else if(tens>0)
	 	{ // don't need exponent, but need to put dp in correct place - will create something like "10.12"
	 	 int minl=tens+1>dlen?dlen:tens+1;
	 	 memmove(dst,dst+1,minl); // memmove allows overlapping areas
	 	 dst[minl]='.';
	 	 if(dlen<=tens) 
		  	{memset(dst+minl,'0',tens+1-dlen);// 1e1 =>10 so needs extra zero's adding
		  	 dst+=tens+1;// was tens+1
		  	}
		 else dst+=dlen+(dlen>1+tens);// need dp if a digit after dp
	 	 *dst=0; return dst;
	 	}
	 // tens < 0, again don't need exponent but need leading zero's before dp.
	 memmove(dst+(-tens)+1,dst+1,dlen);// move right 
	 *dst='0';
	 dst[1]='.';
	 memset(dst+2,'0',-tens-1);
	 dst+=dlen+(-tens)+1; // +1 for dp
	 *dst=0; return dst;
	}
 // print as %e format x.xxxEee	
 *dst=dst[1];// make space for decimal point
 dst[1]='.'; // add in decimal point (this is wasted if prec==0, but avoids any conditional code)
 dst+=dlen+(dlen>1);// need dp if more than 1 digit 
 *dst++='e';
 if(tens<0)
 	{*dst++='-';
 	 tens= -tens;
 	}
 // don't print "+" for exponent
 // now need to print 1,2 or 3 digits for exponent (max +308, min -324 )
 if(tens>=100) 
 	{// 3 digit exponent
	 *dst='0'+tens/100;// 3rd digit
  	 ya_uitoa2(dst+1,tens%100);// other 2 digits of exponent
  	 dst+=3;	 
 	} 
 else if(tens<10)
 	{// only 1 digit exponent 
 	 *dst++='0'+tens;
 	}
 else
 	{// 2 digit exponent
  	 ya_uitoa2(dst,tens);// other 2 digits of exponent
	 dst+=2;
	}
 *dst=0; return dst;	
}  		
 #else /* simpler solution - always print in exponential format, and always print e+/- then 2 or 3 digit exponent. This is slightly faster, but #if 1 version cretaes less characters so is probably faster overall in almost all use cases */
 *dst=dst[1];// make space for decimal point
 dst[1]='.'; // add in decimal point (this is wasted if prec==0, but avoids any conditional code)
 dst+=dlen+(dlen>1);// need dp if more than 1 digit
 // 1st output sign of exponent (+/-) - I tried the "branchless" approach from zmij and it was slightly slower when used here
 *dst++='e';
 tens+=dlen-1;// only 1 digit before dp
 if(tens<0)
 	{*dst++='-';
 	 tens= -tens;
 	}
 else *dst++='+';
 // now print exp - this is the fastest of many methods I tried
 // 3 digits of exponent for double - max exponent is +308, min -324 - always print at least 2 digits 
 if(tens>=100) 
 	{// 3 digit exponent
	 *dst='0'+tens/100;// 3rd digit
  	 ya_uitoa2(dst+1,tens%100);// other 2 digits of exponent
	 dst+=3;	 
 	} 
 else
 	{// 2 digit exponent
  	 ya_uitoa2(dst,tens);//  2 digits of exponent
	 dst+=2;
	}
 *dst=0; return dst;	
} 
#endif 	


/* This function gives float->"shortest string" */
char * ya_shortf(char *dst, float f) /* returns pointer to trailing 0 in string */
{
 union {
	float real_d;
	uint32_t bits_d;
  } u = { f };
	// Decode bits into sign, mantissa, and exponent.
 const bool vsign = (u.bits_d  & ((uint32_t)1<<31)) != 0;// true if v is negative 1<<31 as sign bit is the msb 
 uint32_t ieeeMantissa = u.bits_d & 0x007FFFFF;
 int ieeeExponent=(int)((u.bits_d>>23) & 0xff) ; 
 if(ieeeExponent==0xff)
	{// nan or inf
	 if(vsign) *dst++='-';
	 if(ieeeMantissa) // nan
		{ if(u.bits_d==0xFFC00000)
			{//strcpy(dst,"nan(ind)");
			 memcpy(dst,"nan(ind)",9); // 9 includes trailing 0
			 dst+=8;
			}
		  else if( (ieeeMantissa & (1ull << 22)) == 0)
			 {//strcpy(dst,"nan(snan)");
			  memcpy(dst,"nan(snan)",10);// 10 includes trailing 0
			  dst+=9;
			 }
		 else 
		 	{// strcpy(dst,"nan");
		 	 memcpy(dst,"nan",4);// 4 includes trailing 0
			 dst+=3;
		 	}
		}
	 else 
	 	{//strcpy(dst,"inf");
		 memcpy(dst,"inf",4);// 4 includes trailing 0
		 dst+=3;	 	
	 	}	 
	 return dst;
	}	 	
 if(ieeeExponent==0 && ieeeMantissa==0)
	{// zero is special as we cannot scale it into range : return 0
	 //printf("v (%g) == zero!\n",v);
	 if(vsign) memcpy(dst,"-0",3);// 3 to include null
	 else memcpy(dst,"0",2);
	 return dst+1+vsign;
	}	
 if(vsign) 
	{*dst++='-';
	}	
 // end of handling for nan etc	
 int32_t tens;// base 10 exponent
 int dlen=Short_f(ieeeMantissa,ieeeExponent, dst+1, &tens);// short_f gives us a string containing the mantissa at dst+1

 // have written mantissa, now deal with exponent
 #ifdef REAL_SHORTEST
 // This code does create the actual shortest length representation using either fixed point of exponential format - exponential format does not use a + sign for exponent and can print only 1 exponent digit if thats all that's required.
 // At this point mantissa is at dst+1 and has dlen digits
 tens+=dlen-1;// only 1 digit before dp - gives us equivalent exponent for %e format
 if((tens>-3 && tens<=0) || (tens>0 && tens<=dlen+1) ) 
 	{// conditions above taken from C printf %g 
 	if(tens==0)
 		{// simple case x.xxx with no exponent
		*dst=dst[1];// make space for decimal point
 		 dst[1]='.'; // add in decimal point (this is wasted if prec==0, but avoids any conditional code)
 		 dst+=dlen+(dlen>1);// need dp if more than 1 digit 
	 	 *dst=0; return dst;
	 	}
	 else if(tens>0)
	 	{ // don't need exponent, but need to put dp in correct place - will create something like "10.12"
	 	 int minl=tens+1>dlen?dlen:tens+1;
	 	 memmove(dst,dst+1,minl); // memmove allows overlapping areas
	 	 dst[minl]='.';
	 	 if(dlen<=tens) 
		  	{memset(dst+minl,'0',tens+1-dlen);// 1e1 =>10 so needs extra zero's adding
		  	 dst+=tens+1;// was tens+1
		  	}
		 else dst+=dlen+(dlen>1+tens);// need dp if a digit after dp
	 	 *dst=0; return dst;
	 	}
	 // tens < 0, again don't need exponent but need leading zero's before dp.
	 memmove(dst+(-tens)+1,dst+1,dlen);// move right 
	 *dst='0';
	 dst[1]='.';
	 memset(dst+2,'0',-tens-1);
	 dst+=dlen+(-tens)+1; // +1 for dp
	 *dst=0; return dst;
	}
 // print as %e format x.xxxEee	
 *dst=dst[1];// make space for decimal point
 dst[1]='.'; // add in decimal point (this is wasted if prec==0, but avoids any conditional code)
 dst+=dlen+(dlen>1);// need dp if more than 1 digit 
 *dst++='e';
 if(tens<0)
 	{*dst++='-';
 	 tens= -tens;
 	}
 // don't print "+" for exponent
 if(tens<10)
 	{// only 1 digit exponent 
 	 *dst++='0'+tens;
 	}
 else
 	{// 2 digit exponent
  	 ya_uitoa2(dst,tens);// other 2 digits of exponent
	 dst+=2;	
	}
 *dst=0; return dst;
}  	
 #else /* simpler solution - always print in exponential format, and always print e+/- then 2 digit exponent. This is slightly faster, but #if 1 version cretaes less characters so is probably faster overall in almost all use cases */
 *dst=dst[1];// make space for decimal point
 dst[1]='.'; // add in decimal point (this is wasted if prec==0, but avoids any conditional code)
 dst+=dlen+(dlen>1);// need dp if more than 1 digit 
 // 1st output sign of exponent (+/-) - I tried the "branchless" approach from zmij and it was slightly slower when used here
 *dst++='e';
 tens+=dlen-1;// only 1 digit before dp
 if(tens<0)
 	{*dst++='-';
 	 tens= -tens;
 	}
 else *dst++='+';
 // now print exp - this is the fastest of many methods I tried
 // 2 digit exponent for floats (e-45 to E+38) - always print  2 digits 
 ya_uitoa2(dst,tens);
 dst[2]=0; // terminate string
 return dst+2;
} 	
#endif 



/* function below created by Peter Miller 25-2-2026 to interface with fast_strtod() in atof.c 
   Based on similar interface function for Ryu in s2s_fast_atof.c
*/

// DBL_DENORM_MIN =((double)4.94065645841246544176568792868221372e-324L)
// DBL_MIN =((double)2.22507385850720138309023271733240406e-308L)
// DBL_MAX =((double)1.79769313486231570814527423731704357e+308L)
/*
IEEE 754 Special Values 
	Exponent 		Fraction 	Represents
	e = emin - 1 	f = 0 		±0
	e = emin - 1 	f!=0 		0.f*2^emin
	emin<= e <=emax  -- 		1.f × 2^e
	e = emax + 1 	f = 0 		±infinity
	e = emax + 1 	f !=0 		NaN 
Note the exponent range is emin-1 to emax+1 where for doubles emin-1==0 and emax+1=7ff (so emin=1 and emax=7fe) [ note as the bias is 1023 a denormal is 0.f*2^-1022 , or emin= -1022]
*/
static inline double int64Bits2Double(uint64_t bits) {
 const union {
 	uint64_t bits_d;
	double real_d;
  } u = { bits };
  return u.real_d;  
}

//#define DEBUG_DCONV  /* define for debugging printf's in code below */

#ifdef DEBUG_DCONV 
 #include <inttypes.h> // for PRI  formats for uint64_t
 #include <stdio.h>
#endif

// It converts a sign (bool signedM, true=-ve),
// integer mantissa (uint64_t m10) and an integer (power of 10) exponent (uint32_t e10) to convert to a double

double ya_conv_mant_exp_to_double(bool signedM,uint64_t m10,int32_t dec_exp)
{ 
// constants below from fpfmt paper, page 9 - log2Pow10 function
// 108853 / 2^15 = 3.3219 29931 , log2(10)=3.3219 2809 
// 350*108853= 3.81e7, 2^31=2.1e9 so the result of dec_exp*108853 easily fits into an int32_t 
  const int log2_pow10_sig = 108853, log2_pow10_exp = 15,numb_bits =64-DOUBLE_MANTISSA_BITS;
#ifdef DEBUG_DCONV  
  printf(" z_conv_mant_exp_to_double(%s,0X%016"PRIx64",%d)\n",(signedM?"negative":"positive"),m10,dec_exp);
#endif    
  if(m10==0)
	   	{//  if m10 is zero, result will be zero, need to trap this 1st as next step is to normalise m10 so its msbit is 1 - which is clearly not possible if its 0!
	   	 // return +/-0
	 	 uint64_t ieee = ((uint64_t) signedM) << (DOUBLE_EXPONENT_BITS + DOUBLE_MANTISSA_BITS);
	 	 return int64Bits2Double(ieee);	   	
	   	}   
  // now scale m10 to make it as big as possible 
  const int m10exp=ya_clz(m10); // count leading zeros in m10
  m10<<=m10exp; // shift so msb of m10 is set 
  // pow10_bin_exp = floor(log2(10**-dec_exp))-(numb_bits-1) - m10exp +64   (+64 as we after multiply we will take highest 64 bits of the result)
  int bin_exp = (dec_exp * log2_pow10_sig >> log2_pow10_exp)-(numb_bits-1)-m10exp+64;
#ifdef DEBUG_DCONV  
  printf(" after normalisation of m10 : m10=0X%016"PRIx64": bin_exp=%d\n",m10,bin_exp);
#endif  
  //static u2_64 get_pow10_significand(int dec_exp)
  u2_64 table_val=get_pow10_significand(dec_exp);// access powers of 10 table, note this table is always a u2_64 (ie a pair of uint64_t's)
  if(table_val.hi==0)
  	{// outside of range of table, return something sensible
  	 if(dec_exp>0) 
  	 		{// table goes up to 1e+350, but DBL_MAX=1.8e308, so we are well over that ; return +/-Infinity.
		     uint64_t ieee = (((uint64_t) signedM) << (DOUBLE_EXPONENT_BITS + DOUBLE_MANTISSA_BITS)) | (0x7ffull << DOUBLE_MANTISSA_BITS);
		     return int64Bits2Double(ieee);
			} 
  	 else 
	   	{//  table goes down to 1e-350 and with denormals doubles go down to 4.9e-324, so if we are off the end of the table the result is definately zero.
	   	 // return +/-0
	 	 uint64_t ieee = ((uint64_t) signedM) << (DOUBLE_EXPONENT_BITS + DOUBLE_MANTISSA_BITS);
	 	 return int64Bits2Double(ieee);	   	
	   	}
  	}  
  uint64_t ieee_m2=umulhi_stickybit(table_val.hi, table_val.lo,m10) ; // multiply power of 10 and integer mantissa
  // now normalise this so msbit=1 [needed as 10*10=0100 whereas 11*11=1001 ], should only need to shift by at most 1 bit so no point in optimising it
  if((ieee_m2 & ((uint64_t)1<<63))==0) { bin_exp--; ieee_m2<<=1;} 
  uint64_t ieee_m2_c=ieee_m2; // copy for rounding
#ifdef DEBUG_DCONV  
  printf(" 2's mantissa=0X%016"PRIx64", exponent=%d \n",ieee_m2,bin_exp);
#endif  
  int mant_shift_right_by=DOUBLE_EXPONENT_BITS;// shift required for ieee format mantissa where exponent is in higher bits
  ieee_m2>>=mant_shift_right_by; bin_exp+=mant_shift_right_by; 
  // denormalise if necessary, this might give a mantissa of 0 [if it does it might change after rounding]
  if(bin_exp<-1022)
  	{ // "denormalise" (shift mantissa right and adjust exponent to match)
	 int dshift=(-bin_exp-1022);
	 mant_shift_right_by+=dshift;
	 if(mant_shift_right_by>64)
	 	{// result is definately zero (even allowing for rounding), return +/-0
	 	 uint64_t ieee = ((uint64_t) signedM) << (DOUBLE_EXPONENT_BITS + DOUBLE_MANTISSA_BITS);
	 	 return int64Bits2Double(ieee);	 		 	
	 	}
	 ieee_m2>>=dshift;// shifts of 64 or more are not defined by C, but the if statement above guarantees dshift is <64 here. ieee_m2 might be zero after this, but rounding below might change it to 1
	 bin_exp= -1023;	// special value used to indicate number is a denormal 		 
  	}
#ifdef DEBUG_DCONV  
  printf(" after aligning mantissa, 2's mantissa=0X%016"PRIx64", exponent=%d ,mant_shift_right_by=%d \n",ieee_m2,bin_exp,mant_shift_right_by);
#endif  
  bool ieee_m_zero=ieee_m2==0;// needed as next line removes hidden bit for normals, which may leave ieee_m2=0
  ieee_m2 &= (1ull << DOUBLE_MANTISSA_BITS) - 1;// get rid of leading bit of mantissa (it's "hidden") [ this will not do anything for denormals ]
#ifdef DEBUG_DCONV  
  printf(" after removing hidden bit from mantissa, 2's mantissa=0X%016"PRIx64", exponent=%d \n",ieee_m2,bin_exp);
#endif 
  //now need to round mantissa
  if(mant_shift_right_by<=63) ieee_m2_c &= (1ull<<mant_shift_right_by)-1; // mask out bits involved in rounding (if mant_shift_right_by==64 all bits are used in rounding so no & required)
  if(ieee_m2_c>(1ull<<(mant_shift_right_by-1)) || (ieee_m2_c==(1ull<<(mant_shift_right_by-1)) && ((ieee_m2&1) !=0))) {ieee_m2++;ieee_m_zero=false;}; // round up if >0.5 or =0.5 and odd (round to even) [any "carry" from this is dealt with later]
#ifdef DEBUG_DCONV  
  printf(" after rounding mantissa, 2's mantissa=0X%016"PRIx64", Threshold=0X%016"PRIx64" exponent=%d ,mant_shift_right_by=%d ,ieee_m2_c=0X%016"PRIx64" \n",ieee_m2,(1ull << (DOUBLE_MANTISSA_BITS + 1)),bin_exp,mant_shift_right_by,ieee_m2_c);
#endif  
  int ieee_e2=bin_exp+DOUBLE_EXPONENT_BIAS; 
  if (ieee_e2 > 0x7fe) 
	{
     // Final IEEE exponent is larger than the maximum representable; return +/-Infinity.
     uint64_t ieee = (((uint64_t) signedM) << (DOUBLE_EXPONENT_BITS + DOUBLE_MANTISSA_BITS)) | (0x7ffull << DOUBLE_MANTISSA_BITS);
     return int64Bits2Double(ieee);
	} 
  if(ieee_e2<0 || ieee_m_zero) 
  	{// exponent too low, or mantissa=0, return +/-0
 	 uint64_t ieee = ((uint64_t) signedM) << (DOUBLE_EXPONENT_BITS + DOUBLE_MANTISSA_BITS);
 	 return int64Bits2Double(ieee);	 	
	} 
  // put it all together to make an ieee format double
  uint64_t ieee = (((((uint64_t) signedM) << (DOUBLE_EXPONENT_BITS + DOUBLE_MANTISSA_BITS)) | (uint64_t)(ieee_e2 & 0x7ff) << DOUBLE_MANTISSA_BITS) + ieee_m2); // we must add mantissa because rounding may have caused a carry into exponent
#ifdef DEBUG_DCONV  
  printf(" after adjustments for ieee 2's mantissa=0X%016"PRIx64", exponent=0x%x, so whole ieee double is 0X%016"PRIx64" \n",ieee_m2,ieee_e2,ieee);
#endif  
  double d=int64Bits2Double(ieee);
#ifdef DEBUG_DCONV  
  printf(" making double=%.20g\n",d);
#endif  
  return d;  
}

