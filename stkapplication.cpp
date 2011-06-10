/*
 * satk - SIM application toolkit
 * Copyright © 2011, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Written by - Luc Yriarte <luc.yriarte@linux.intel.com>
 */


#include "stkapplication.h"
#include "stkofonoutils.h"


StkApplication::StkApplication(int &argc, char **argv, int version) :
    QApplication(argc, argv, version)
{
    mAgentServiceRegistered = mAgentMode = false;
    mStkAgentIfAdaptor = NULL;
    mStkAgentService = NULL;
    mMgrIf = NULL;
    mModemIf = NULL;
    mSimIf = NULL;
    mStkIf = NULL;
}

StkApplication::~StkApplication()
{
    deleteInterfaces();
}

void StkApplication::resetInterfaces()
{
    // Unregister StkAgentService if we registered it
    if (mAgentServiceRegistered)
        StkOfonoUtils::unRegisterSimToolkitAgent(mStkIf);
    // Disconnect all signals sent from oFono interfaces
    if (mMgrIf != NULL) {
        disconnect(mMgrIf, SIGNAL(ModemAdded(QDBusObjectPath, QVariantMap)), this, SLOT(mgrModemAdded(QDBusObjectPath, QVariantMap)));
        disconnect(mMgrIf, SIGNAL(ModemRemoved(QDBusObjectPath)), this, SLOT(mgrModemRemoved(QDBusObjectPath)));
    }
    if (mModemIf != NULL)
        disconnect(mModemIf, SIGNAL(PropertyChanged(QString, QDBusVariant)), this, SLOT(modemPropertyChanged(QString, QDBusVariant)));
    if (mSimIf != NULL)
        disconnect(mSimIf, SIGNAL(PropertyChanged(QString, QDBusVariant)), this, SLOT(simPropertyChanged(QString, QDBusVariant)));
    mModemIf = NULL; // delete with mModemIfs
    mSimIf = NULL; // delete with mSimIfs
    mStkIf = NULL; // delete with mStkIfs
    mStkAgentService = NULL; // delete with mStkAgentIfAdaptor
    mAgentServiceRegistered = mAgentMode = false;
}

void StkApplication::deleteInterfaces()
{
    resetInterfaces();
    // delete all org.ofono.SimToolkit interfaces
    while (!mStkIfs.isEmpty()) {
        delete mStkIfs.first();
        mStkIfs.removeFirst();
    }
    // delete all org.ofono.SimManager interfaces
    while (!mSimIfs.isEmpty()) {
        delete mSimIfs.first();
        mSimIfs.removeFirst();
    }
    // delete all org.ofono.Modem interfaces
    while (!mModemIfs.isEmpty()) {
        delete mModemIfs.first();
        mModemIfs.removeFirst();
    }
    // StkAgentService is deleted by StkAgentIfAdaptor destructor
    if (mStkAgentIfAdaptor != NULL) {
        delete mStkAgentIfAdaptor;
        mStkAgentIfAdaptor = NULL;
    }
    // disconnected by resetInterfaces
    if (mMgrIf != NULL) {
        delete mMgrIf;
        mMgrIf = NULL;
    }
}

bool StkApplication::initOfonoConnection(bool agentMode)
{
    deleteInterfaces();
    mAgentMode = agentMode;
    // DBus Connection systemBus
    QDBusConnection connection = QDBusConnection::systemBus();
    if( !connection.isConnected() ) {
        QDBusError dbusError = connection.lastError();
        qDebug() << "Error:" << dbusError.name() << ":" << dbusError.message();
        return false;
    }
    // Instanciate proxy for org.ofono.Manager interface
    mMgrIf = new MgrIf("org.ofono","/",connection,NULL);
    // find all modems
    OfonoModemList modems = StkOfonoUtils::findModems(mMgrIf);
    if (modems.isEmpty()) {
        qDebug() << "No modem found, registering for modem add / removed";
        registerModemMgrChanges();
        return false;
    }
    // find org.ofono.Modem interfaces
    mModemIfs = StkOfonoUtils::findModemInterfaces(connection, mMgrIf);
    if (mModemIfs.isEmpty()) {
        qDebug() << "No modem interface found, registering for modem add / removed";
        registerModemMgrChanges();
        return false;
    }
    // Use the first Modem available
    mModemIf = mModemIfs.first();
    // find org.ofono.SimManager interfaces for all modems
    mSimIfs = StkOfonoUtils::findSimInterfaces(connection, mMgrIf);
    if (mSimIfs.isEmpty()) {
        qDebug() << "No SIM interface found, registering for Modem property changes";
        registerModemPropertyChanged();
        return false;
    }
    // Use the first SimManager available
    mSimIf = mSimIfs.first();
    // find org.ofono.SimToolkit interfaces for all modems
    mStkIfs = StkOfonoUtils::findSimToolkitInterfaces(connection, mMgrIf);
    if (mStkIfs.isEmpty()) {
        qDebug() << "No SIM Toolkit interface found, registering for SIM property changes";
        registerSimPropertyChanged();
        return false;
    }
    // Use the first SimToolkit available
    mStkIf = mStkIfs.first();
    // Hook StkAgentIfAdaptor and StkAgentService together
    mStkAgentService = new StkAgentService(mSimIf);
    mStkAgentIfAdaptor = new StkAgentIfAdaptor(mStkAgentService);
    // Register agent service
    mAgentServiceRegistered = registerStkAgentService();
    return true;
}

bool StkApplication::registerModemMgrChanges()
{
    // Connect mgrIf modem added / removed signals
    bool reg1 = connect(mMgrIf, SIGNAL(ModemAdded(QDBusObjectPath, QVariantMap)), this, SLOT(mgrModemAdded(QDBusObjectPath, QVariantMap)));
    bool reg2 = connect(mMgrIf, SIGNAL(ModemRemoved(QDBusObjectPath)), this, SLOT(mgrModemRemoved(QDBusObjectPath)));
    return reg1 && reg2;
}

bool StkApplication::registerModemPropertyChanged()
{
    // Connect simIf signals
    return connect(mModemIf, SIGNAL(PropertyChanged(QString, QDBusVariant)), this, SLOT(modemPropertyChanged(QString, QDBusVariant)));
}

bool StkApplication::registerSimPropertyChanged()
{
    // Connect simIf signals
    return connect(mSimIf, SIGNAL(PropertyChanged(QString, QDBusVariant)), this, SLOT(simPropertyChanged(QString, QDBusVariant)));
}

bool StkApplication::registerStkAgentService()
{
    if (mStkAgentService == NULL || mStkIf == NULL)
        return false;
    QDBusConnection connection = QDBusConnection::systemBus();
    if (StkOfonoUtils::registerSimToolkitAgent(connection, mStkAgentService, mStkIf) != 0)
        return false;
    return true;
}

void StkApplication::unRegisterStkAgentService()
{
    StkOfonoUtils::unRegisterSimToolkitAgent(mStkIf);
}

void StkApplication::mgrModemAdded(const QDBusObjectPath &in0, const QVariantMap &in1)
{
    qDebug() << "mgrModemAdded: " << in0.path() << " variant map: " << in1;
}

void StkApplication::mgrModemRemoved(const QDBusObjectPath &in0)
{
    qDebug() << "mgrModemRemoved: " << in0.path();
}

void StkApplication::modemPropertyChanged(const QString &property, const QDBusVariant &value)
{
    qDebug() << "modemPropertyChanged: " << property << " variant string : " << value.variant().toString();
}

void StkApplication::simPropertyChanged(const QString &property, const QDBusVariant &value)
{
    qDebug() << "simPropertyChanged: " << property << " variant string : " << value.variant().toString();
}
