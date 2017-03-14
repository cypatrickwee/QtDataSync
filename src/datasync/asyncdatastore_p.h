#ifndef ASYNCDATASTORE_P_H
#define ASYNCDATASTORE_P_H

#include "qdatasync_global.h"
#include "asyncdatastore.h"
#include "storageengine_p.h"

namespace QtDataSync {

class Q_DATASYNC_EXPORT AsyncDataStorePrivate
{
public:
	StorageEngine *engine;
};

}

#endif // ASYNCDATASTORE_P_H