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


#include <QtDebug>
#include <QIcon>
#include <QDBusArgument>
#include <QVariantMap>
#include <QMapIterator>
#include "ofonodbustypes.h"
#include "stkofonoutils.h"
#include "stkofonoproperties.h"


StkOfonoProperties::StkOfonoProperties(StkIf *stkIf, QObject *parent) :
    QObject(parent)
{
    // org.ofono.SimToolkit interface GetProperties method
    QDBusPendingReply<QVariantMap> stkPropsCall = stkIf->GetProperties();
    stkPropsCall.waitForFinished();
    if (stkPropsCall.isError()) {
        QDBusError error = stkPropsCall.error();
        qDebug() << "Error:" << error.name() << ":" << error.message();
    }
    else
    {   // get properties map
        mProperties = stkPropsCall.value();
    }
}


QList<QListWidgetItem *> StkOfonoProperties::mainMenuItems()
{
    QList<QListWidgetItem *> items;
    // "MainMenu" property
    QVariant varMenu = mProperties.value("MainMenu");
    QDBusArgument arg = varMenu.value<QDBusArgument>();
    // demarshall a{sy} QDBusArgument to a QMap<QString,uchar> registered as OfonoMenuList
    OfonoMenuList menuList;
    arg >> menuList;
    foreach (const OfonoMenuEntry entry, menuList)
        items.append(new QListWidgetItem(QIcon(StkOfonoUtils::findIcon(entry.icon)), entry.label));
    return items;
}


QString StkOfonoProperties::mainMenuTitle()
{
    return mProperties.value("MainMenuTitle").toString();
}


QPixmap StkOfonoProperties::mainMenuIcon()
{
    return StkOfonoUtils::findIcon((uchar)mProperties.value("MainMenuIcon").toChar().toAscii());
}
