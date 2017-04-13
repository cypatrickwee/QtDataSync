#ifndef QTDATASYNC_ENCRYPTOR_H
#define QTDATASYNC_ENCRYPTOR_H

#include "QtDataSync/qtdatasync_global.h"
#include "QtDataSync/defaults.h"

#include <QObject>

namespace QtDataSync {

class Encryptor : public QObject
{
	Q_OBJECT

public:
	//! Constructor
	explicit Encryptor(QObject *parent = nullptr);

	//! Called from the engine to initialize the encryptor
	virtual void initialize(Defaults *defaults);
	//! Called from the engine to finalize the encryptor
	virtual void finalize();

	//! Encrypts the given dataset
	virtual QJsonValue encrypt(const ObjectKey &key, const QJsonObject &object, const QByteArray &keyProperty) const = 0;
	//! Encrypts the given dataset
	virtual QJsonObject decrypt(const ObjectKey &key, const QJsonValue &data, const QByteArray &keyProperty) const = 0;
};

}

#endif // QTDATASYNC_ENCRYPTOR_H