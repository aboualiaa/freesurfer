#ifndef _BYTESWAP_H_
#define _BYTESWAP_H_

#define SWAP_2(x) ((((x)&0xff) << 8) | ((unsigned short)(x) >> 8))
#define SWAP_4(x)                                                              \
  (((x) << 24) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) |       \
   ((x) >> 24))

inline void SWAP_SHORT(short &x) {
  (*reinterpret_cast<unsigned short *>(&(x)) = SWAP_2(*(unsigned short *)&(x)));
}
inline void SWAP_INT(int &x) {
  (*reinterpret_cast<unsigned int *>(&(x)) = SWAP_4(*(unsigned int *)&(x)));
}
inline void SWAP_FLOAT(float &x) { SWAP_INT((*reinterpret_cast<int *>(&(x)))); }
inline void SWAP_DOUBLE(double &x) {
  double retVal;
  char *p = reinterpret_cast<char *>(&retVal);
  char *i = reinterpret_cast<char *>(&x);
  p[0] = i[7];
  p[1] = i[6];
  p[2] = i[5];
  p[3] = i[4];

  p[4] = i[3];
  p[5] = i[2];
  p[6] = i[1];
  p[7] = i[0];

  x = retVal;
}

inline void SWAP_SHORT(short *x, long nSize) {
  for (int i = 0; i < nSize; i++) {
    SWAP_SHORT(x[i]);
}
}

inline void SWAP_INT(int *x, long nSize) {
  for (int i = 0; i < nSize; i++) {
    SWAP_INT(x[i]);
}
}

inline void SWAP_FLOAT(float *x, long nSize) {
  for (int i = 0; i < nSize; i++) {
    SWAP_FLOAT(x[i]);
}
}

inline void SWAP_DOUBLE(double *x, long nSize) {
  for (int i = 0; i < nSize; i++) {
    SWAP_DOUBLE(x[i]);
}
}

inline bool IS_BIG_ENDIAN() {
  unsigned char ch[2] = {0, 1};
  auto *a = reinterpret_cast<unsigned short *>(ch);

  return *a == 1;
}

#endif // _BYTESWAP_H_
