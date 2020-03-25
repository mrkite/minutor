#ifndef CHUNKID_H
#define CHUNKID_H

// ChunkID is the key used to identify entries in the Cache
// Chunks are identified by their coordinates (CX,CZ) but a single key is needed to access a map like structure
class ChunkID {
 public:
  ChunkID(int cx, int cz);
  bool operator==(const ChunkID &) const;
  friend unsigned int qHash(const ChunkID &);
 protected:
  int cx, cz;
};

inline ChunkID::ChunkID(int cx, int cz) : cx(cx), cz(cz) {
}

inline bool ChunkID::operator==(const ChunkID &other) const {
  return (other.cx == cx) && (other.cz == cz);
}

inline unsigned int qHash(const ChunkID &c) {
  return (c.cx << 16) ^ (c.cz & 0xffff);  // safe way to hash a pair of integers
}

#endif // CHUNKID_H
