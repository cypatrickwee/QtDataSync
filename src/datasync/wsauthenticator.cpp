#include "defaults.h"
#include "wsauthenticator.h"
#include "wsauthenticator_p.h"
#include "wsremoteconnector_p.h"

#include <QtCore/QJsonDocument>

using namespace QtDataSync;

WsAuthenticator::WsAuthenticator(WsRemoteConnector *connector, Defaults *defaults, QObject *parent) :
	Authenticator(parent),
	d(new WsAuthenticatorPrivate(connector))
{
	d->settings = defaults->createSettings(this);

	connect(d->connector, &WsRemoteConnector::remoteStateChanged,
			this, [this](RemoteConnector::RemoteState s){updateConnected(s);},
			Qt::QueuedConnection);
}

WsAuthenticator::~WsAuthenticator() {}

void WsAuthenticator::exportUserDataImpl(QIODevice *device) const
{
	QJsonObject data;
	data[QStringLiteral("key")] = QString::fromUtf8(d->connector->cryptor()->key().toBase64());
	data[QStringLiteral("identity")] = QString::fromUtf8(userIdentity());

	device->write(QJsonDocument(data).toJson(QJsonDocument::Indented));
}

GenericTask<void> WsAuthenticator::importUserDataImpl(QIODevice *device)
{
	QJsonParseError error;
	auto data = QJsonDocument::fromJson(device->readAll(), &error);
	if(error.error != QJsonParseError::NoError) {
		QFutureInterface<QVariant> futureInterface;
		futureInterface.reportStarted();
		futureInterface.reportException(InvalidDataException(error.errorString()));
		futureInterface.reportFinished();
		return futureInterface;
	}

	auto obj = data.object();
	auto key = QByteArray::fromBase64(obj[QStringLiteral("key")].toString().toUtf8());
	QMetaObject::invokeMethod(d->connector->cryptor(), "setKey", Qt::QueuedConnection,
							  Q_ARG(QByteArray, key));
	return setUserIdentity(obj[QStringLiteral("identity")].toString().toUtf8());
}

QUrl WsAuthenticator::remoteUrl() const
{
	return d->settings->value(WsRemoteConnector::keyRemoteUrl).toUrl();
}

QHash<QByteArray, QByteArray> WsAuthenticator::customHeaders() const
{
	d->settings->beginGroup(WsRemoteConnector::keyHeadersGroup);
	auto keys = d->settings->childKeys();
	QHash<QByteArray, QByteArray> headers;
	foreach(auto key, keys)
		headers.insert(key.toUtf8(), d->settings->value(key).toByteArray());
	d->settings->endGroup();
	return headers;
}

bool WsAuthenticator::validateServerCertificate() const
{
	return d->settings->value(WsRemoteConnector::keyVerifyPeer).toBool();
}

QByteArray WsAuthenticator::userIdentity() const
{
	return d->settings->value(WsRemoteConnector::keyUserIdentity).toByteArray();
}

QString WsAuthenticator::serverSecret() const
{
	return d->settings->value(WsRemoteConnector::keySharedSecret).toString();
}

bool WsAuthenticator::isConnected() const
{
	return d->connected;
}

void WsAuthenticator::reconnect()
{
	QMetaObject::invokeMethod(d->connector, "reconnect");
}

void WsAuthenticator::setRemoteUrl(QUrl remoteUrl)
{
	d->settings->setValue(WsRemoteConnector::keyRemoteUrl, remoteUrl);
	d->settings->sync();
}

void WsAuthenticator::setCustomHeaders(QHash<QByteArray, QByteArray> customHeaders)
{
	d->settings->remove(WsRemoteConnector::keyHeadersGroup);
	d->settings->beginGroup(WsRemoteConnector::keyHeadersGroup);
	for(auto it = customHeaders.constBegin(); it != customHeaders.constEnd(); it++)
		d->settings->setValue(QString::fromUtf8(it.key()), it.value());
	d->settings->endGroup();
	d->settings->sync();
}

void WsAuthenticator::setValidateServerCertificate(bool validateServerCertificate)
{
	d->settings->setValue(WsRemoteConnector::keyVerifyPeer, validateServerCertificate);
	d->settings->sync();
}

GenericTask<void> WsAuthenticator::setUserIdentity(QByteArray userIdentity, bool clearLocalStore)
{
	return resetIdentity(userIdentity, clearLocalStore);
}

GenericTask<void> WsAuthenticator::resetUserIdentity(bool clearLocalStore)
{
	return resetIdentity(QVariant(), clearLocalStore);
}

void WsAuthenticator::setServerSecret(QString serverSecret)
{
	d->settings->setValue(WsRemoteConnector::keySharedSecret, serverSecret);
	d->settings->sync();
}

RemoteConnector *WsAuthenticator::connector()
{
	return d->connector;
}

void WsAuthenticator::updateConnected(int connected)
{
	auto cOld = d->connected;
	switch (connected) {
	case RemoteConnector::RemoteDisconnected:
	case RemoteConnector::RemoteConnecting:
		d->connected = false;
		break;
	case RemoteConnector::RemoteLoadingState:
	case RemoteConnector::RemoteReady:
		d->connected = true;
		break;
	default:
		Q_UNREACHABLE();
		break;
	}
	if(d->connected != cOld)
		emit connectedChanged(connected);
}

WsAuthenticatorPrivate::WsAuthenticatorPrivate(WsRemoteConnector *connector) :
	connector(connector),
	settings(nullptr),
	connected(false)
{}
