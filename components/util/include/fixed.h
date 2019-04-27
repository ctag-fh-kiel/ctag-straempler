#pragma once
#define SCALE_3 3
#define SCALE_6 6
#define SCALE_7 7
#define SCALE_8 8
#define SCALE_9 9
#define SCALE_14 14

#define FloatToFixed_3(x) (x*(float)(1 << SCALE_3))
#define FixedToFloat_3(x) ( (float) x/ (float) (1 << SCALE_3))
#define IntToFixed_3(x) (x << SCALE_3)
#define FixedToInt_3(x) (x >> SCALE_3)

#define FloatToFixed_6(x) (x*(float)(1 << SCALE_6))
#define FixedToFloat_6(x) ( (float) x/ (float) (1 << SCALE_6))
#define IntToFixed_6(x) (x << SCALE_6)
#define FixedToInt_6(x) (x >> SCALE_6)

#define FloatToFixed_7(x) (x*(float)(1 << SCALE_7))
#define FixedToFloat_7(x) ( (float) x/ (float) (1 << SCALE_7))
#define IntToFixed_7(x) (x << SCALE_7)
#define FixedToInt_7(x) (x >> SCALE_7)

#define FloatToFixed_8(x) (x*(float)(1 << SCALE_8))
#define FixedToFloat_8(x) ( (float) x/ (float) (1 << SCALE_8))
#define IntToFixed_8(x) (x << SCALE_8)
#define FixedToInt_8(x) (x >> SCALE_8)

#define FloatToFixed_9(x) (x*(float)(1 << SCALE_9))
#define FixedToFloat_9(x) ( (float) x/ (float) (1 << SCALE_9))
#define IntToFixed_9(x) (x << SCALE_9)
#define FixedToInt_9(x) (x >> SCALE_9)


#define FloatToFixed_14(x) (x*(float)(1 << SCALE_14))
#define FixedToFloat_14(x) ( (float) x/ (float) (1 << SCALE_14))
#define IntToFixed_14(x) (x << SCALE_14)
#define FixedToInt_14(x) (x >> SCALE_14)