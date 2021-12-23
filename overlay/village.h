/** Copyright 2014 Rian Shelley */
#ifndef VILLAGE_H_
#define VILLAGE_H_

#include "generatedstructure.h"

class Village : public GeneratedStructure {
 public:
  static QList<QSharedPointer<GeneratedStructure>>
      tryParseDatFile(const Tag* tag, const QString &dimension);

 protected:
  Village() {}
};

#endif  // VILLAGE_H_
