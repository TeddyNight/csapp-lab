/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  //布尔代数：与门和非门，能构造出其他的门
  return ~(~x&~y)&(~(x&y));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  int t=1;
  t=t<<31; 
  return t;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  //Tmax+1=Tmin Tmin=~Tmax 但要排除x=-1的情况，网上学习到的!!的使用
  return !((x+1)^(~x))&!!(x^(~0));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  //利用与运算，1 1得1,提出选定位
  //利用异或运算，判断选定位是否等于想要的数值，因为此时选定1,非选定0,与1 xor,那么如果相等全为0
  int t=170+(170<<8)+(170<<16)+(170<<24);
  return !(x&t^t);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  //运用！符号变为0与1，&选取需要的位，^检查想要的位
  //110***这种类型是满足条件的，因而直接右移三位剩下110，再与111按位与，与110按位或，就能排除111的情况
  int a=(~0)^63;
  return !(x&a)&(!(((x>>3)&7^6))|!(((x>>1)&31^28)));
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  //映射，=0的情况映射到1111，不等于0的情况映射到0000
  return (~(!x<<31>>31)&y)|((!x<<31>>31)&z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  //算x-y，但可能溢出
  //x<0 y>=0 是全部满足的，规避x是tmin,y是tmax溢出情况
  //x>=0 y<0 全不满足，规避x是tmax，y是tmin溢出情况，除了这种情况其他都是条件满足，一个逻辑否排除这种情况
  //x>=0 y>=0 不会溢出
  //x<0 y<0 不会溢出
  int t=1<<31;
  int m=x+~y+1;
  int xn=!(x&t^t);
  int yn=!(y&t^t);
  return !m|xn&!yn|!(!xn&yn)&!(m&t^t);
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  //从二进制补码的特征考虑，符号位区分成两类，负号肯定不含0
  //0的相反数是0，其他的正数都有相反数，区分一下就可以了
  int t=~0;
  return ((x>>31^t)&1)&((~x+1)>>31^t);
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
    //负数变正数方便处理
    int sign=(x&(1<<31))>>31;
    int t=(sign&(~x))|((~sign)&x);
    //查阅了网上的答案，和老鼠吃毒药有点相似
    //计导课老鼠的启发
    //5位二进制数表示结果
    //二进制和二分法？
    int b16,b8,b4,b2;
    int b1,b0; //检查最后两位
    b16=(!!(t>>16))<<4;
    t=t>>b16;
    b8=(!!(t>>8))<<3;
    t=t>>b8;
    b4=(!!(t>>4))<<2;
    t=t>>b4;
    b2=(!!(t>>2))<<1;
    t=t>>b2;
    //两位的处理比较特殊，先看有没有第二位
    //有这时候只加1
    //因为有的话检查第1位的时候实际是右移得到的第二位
    //如果直接&2和&1的话，11这种情况不好处理
    b1=(!!(t>>1));
    t=t>>b1;
    b0=t&1;
    return b16+b8+b4+b2+b1+b0+1;
    
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  int sign=uf&(1<<31);
  int exp=uf&2139095040;
  int frac=uf&8388607;
  //检查exp全为1，即NaN和INF的情况，直接返回
  if(!(exp^2139095040)) return uf;
  //normalize情况，是1+1，尾数是00001，不需要*2
  if(exp) exp=((exp>>23)+1)<<23;
  //denormalize情况，需要*2
  else frac=frac*2;
  return sign+exp+frac;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  //float表示的数字 (-1)^s*2^E*frac，就是科学计数法1.00000*2^E
  int sign=1;
  int exp=uf&2139095040;
  int e=(exp>>23)-127;
  int frac=uf&8388607;
  if(uf&(1<<31)) sign=-1;
  //检查exp全为1，即NaN和INF的情况，直接返回
  if(!(exp^2139095040)) return 0x80000000u;
  //denormalize情况
  if(!exp) return 0;
  //normalize 情况
  else{
	  //指数小于0，不会大于1，直接舍去，并不是1+小数部分
	  if(e<0) return 0;
	  else if(e==0) return sign;
	  else if(e<=7) return (frac+(1<<24))<<e;
	  //溢出的情况，Intel系列的处理器指定位模式[10..0]即Tmin为整数不确定值
	  //else return (frac+(1<<23))<<8;
	  else return 1<<31;
  }
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    int exp=127;
    if(x==0) return 0x3f800000;
    if(x>127) return 255<<23;
    if(x<-127) return 0;
    exp=(exp+x)<<23;
    //不是1+exp，还是没理解，它是隐含一个1，[1...]这样的，frac代表0....
    return exp;
}
