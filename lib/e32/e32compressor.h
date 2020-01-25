#ifndef E32COMPRESSOR_H_INCLUDED
#define E32COMPRESSOR_H_INCLUDED

//! set input buffer as nullptr to decompress remained data
uint32_t DecompressBPE(const char* src, char* dst);
uint32_t CompressBPE(const char* src, uint32_t srcSize, char* dst, uint32_t dstSize);

#endif // E32COMPRESSOR_H_INCLUDED
