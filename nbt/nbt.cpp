/** Copyright (c) 2013, Sean Kasun */

#include <zlib.h>
#include <QFile>

#include "nbt/nbt.h"
#include "lz4/lz4.h"

#define XXH_INLINE_ALL
#include "lz4/xxhash.h"


// this handles decoding the gzipped level.dat
NBT::NBT(const QString level)
  : root(&NBT::Null)  // just in case we die, init with empty data
{
  QFile f(level);
  f.open(QIODevice::ReadOnly);
  QByteArray data = f.readAll();
  f.close();

  // level.dat is typically gzip format, but autodetect here
  // +32 to autodetect gzip/zlib compression
  unpack_zlib(reinterpret_cast<Bytef *>(data.data()), data.size(), 15 + 32);
}

// this handles decoding a compressed Chunk of a region file
NBT::NBT(const uchar *chunk)
  : root(&NBT::Null)  // just in case we die, init with empty data
{
  // find chunk size in first 4 bytes, format is fifth byte
  int length = (chunk[0] << 24) | (chunk[1] << 16) | (chunk[2] << 8) | chunk[3];
  length -= 1; // -1 byte for compression format
  const unsigned char * data = reinterpret_cast<const Bytef *>(chunk) + 5;
  // supported compression formats
  if (chunk[4] == 1) unpack_zlib(data, length, 15 + 16);  // rfc1952 not used by official Minecraft
  if (chunk[4] == 2) unpack_zlib(data, length, 15 + 0);   // rfc1950 default for all Chunk data
  if (chunk[4] == 3) decode_nbt(data, length);            // copy uncompressed data
  if (chunk[4] == 4) unpack_lz4(data, length);            // LZ4 compression
  // silent return with empty data in case of unsupported format
}

Tag NBT::Null;


static const int CHUNK_SIZE = 8192;
// default window size: 15 bit
// + 0 zlib data (RFC 1950)
// +16 gzip data (RFC 1952)
// +32 autodetect zlib/gzip from header
void NBT::unpack_zlib(const unsigned char * data, unsigned long length, int windowsize) {
  // prepare zlib stream structure
  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree  = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = length;
  stream.next_in  = const_cast<unsigned char *>(data);  // yes we violate "const", but zlib will not change the input data

  // intermediate buffer for decompression
  char buffer[CHUNK_SIZE];

  // final buffer for decompressed NBT data
  QByteArray nbt;

  inflateInit2(&stream, windowsize);
  do {
    stream.avail_out = CHUNK_SIZE;
    stream.next_out = reinterpret_cast<Bytef *>(buffer);
    inflate(&stream, Z_NO_FLUSH);
    nbt.append(buffer, CHUNK_SIZE - stream.avail_out);
  } while (stream.avail_out == 0);
  inflateEnd(&stream);

  decode_nbt(nbt.constData(), nbt.size());
}


// Minecraft uses an incompatible Java implementation of LZ4
// -> so we have to decode magic headers by ourself
// data is compressed in series of blocks, each with a header
// and luckily blocks are independend from each other
static const char LZ4_MAGIC[] = {'L', 'Z', '4', 'B', 'l', 'o', 'c', 'k'};
static const int  LZ4_MAGIC_LENGTH = sizeof(LZ4_MAGIC);

static const int LZ4_COMPRESSION_METHOD_RAW = 0x10;
static const int LZ4_COMPRESSION_METHOD_LZ4 = 0x20;
static const int LZ4_COMPRESSION_LEVEL_BASE = 10;
static const unsigned int LZ4_DEFAULT_SEED = 0x9747b28c;

// read an int from Little Endian byte stream
long readIntLE(const unsigned char * data) {
  return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0]);
}

void NBT::unpack_lz4(const unsigned char * data, unsigned long length) {
  if (length < LZ4_MAGIC_LENGTH+13) return;

  QByteArray nbt;

  const unsigned char * input = data;

  while (static_cast<unsigned long>(input - data) < length) {
    // decode LZ4-Java block header
    for (int m=0; m<LZ4_MAGIC_LENGTH; m++) {
      if (input[m] != LZ4_MAGIC[m]) return;
    }
    const unsigned char token = input[LZ4_MAGIC_LENGTH];
    const unsigned char compression_method = token & 0xF0;
//   unsigned char compression_level  = LZ4_COMPRESSION_LEVEL_BASE + (token & 0x0F);
    if ((compression_method != LZ4_COMPRESSION_METHOD_RAW) && (compression_method != LZ4_COMPRESSION_METHOD_LZ4)) return;
    const unsigned long length_compressed  = readIntLE(input + LZ4_MAGIC_LENGTH + 1);
    const unsigned long length_original    = readIntLE(input + LZ4_MAGIC_LENGTH + 5);
    const XXH32_hash_t checksum   = readIntLE(input + LZ4_MAGIC_LENGTH + 9);
    input += LZ4_MAGIC_LENGTH + 13;

    // special block indicating "no more data"
    if ((length_compressed == 0) && (length_original == 0)) break;
    // error checks
    if (length_compressed < 0) return;
    if (length_original < 0) return;
    if ((length_compressed == 0) && (length_original != 0)) return;
    if ((length_original == 0) && (length_compressed != 0)) return;
    if ((compression_method == LZ4_COMPRESSION_METHOD_RAW) && (length_original != length_compressed)) return;

    // input buffer overflow check
    if ((static_cast<unsigned long>(input - data) + length_compressed) > length) return;

    XXH32_hash_t checksum1;

    if (compression_method == LZ4_COMPRESSION_METHOD_RAW) {
      // copy RAW block
      nbt.append(reinterpret_cast<const char *>(input), length_original);
      checksum1 = XXH32(input, length_original, LZ4_DEFAULT_SEED);
    } else {
      // decompress one block
      char * buffer = reinterpret_cast<char *>(malloc(length_original));
      int len = LZ4_decompress_safe(reinterpret_cast<const char *>(input), buffer,
                                    length_compressed, length_original);
      checksum1 = XXH32(buffer, length_original, LZ4_DEFAULT_SEED);
      nbt.append(buffer, len);
      free(buffer);
    }
    // advance input data pointer
    input += length_compressed;
    // check for matching checksum
    checksum1 &= 0x0fffffff;  // why the hell we have to remove the uppermost 4 bits ?!?
    if (checksum != checksum1) return;
  }

  decode_nbt(nbt.constData(), nbt.size());
}

void NBT::decode_nbt(const unsigned char * data, unsigned long length) {
  decode_nbt(reinterpret_cast<const char *>(data),  length);
}

void NBT::decode_nbt(const char * data, unsigned long length) {
  TagDataStream s(data, length);

  if (s.r8() == Tag::TAG_COMPOUND) {  // outer compound is expected
    s.skip(s.r16());  // skip name (should be empty anyways)
    root = new Tag_Compound(&s);
  }
}

bool NBT::has(const QString key) const {
  return root->has(key);
}

const Tag *NBT::at(const QString key) const {
  return root->at(key);
}

NBT::~NBT() {
  if (root != &NBT::Null)
    delete root;
}
