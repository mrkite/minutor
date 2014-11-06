#ifndef VILLAGE_H
#define VILLAGE_H

#include "generatedstructure.h"

class Village : public GeneratedStructure
{
public:
	static QList<QSharedPointer<GeneratedStructure> > tryParse(Tag* tag, const QString &dimension);

protected:
	Village() {}
};

#endif // VILLAGE_H
