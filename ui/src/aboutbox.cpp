/*
  Q Light Controller
  aboutbox.cpp

  Copyright (C) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QUrl>
#include <QDebug>
#include <QLabel>
#include <QAction>
#include <QCoreApplication>
#include <QDesktopServices>

#include "qlcconfig.h"
#include "aboutbox.h"

AboutBox::AboutBox(QWidget* parent) : QDialog (parent)
{
    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_titleLabel->setText(APPNAME);
    QString version = QCoreApplication::applicationVersion();
    if (version.isEmpty())
        version = APPVERSION;
    m_versionLabel->setText(version);
    m_copyrightLabel->setText(QString("Copyright &copy; <B>Imedia Group - Multiverse Controller</B>"));
    m_websiteLabel->setText(tr("Sito Web: %1").arg("<A HREF=\"https://www.aicontroller.it/\">https://www.aicontroller.it/</A>"));
    m_licenseLabel->setText(QString::fromUtf8("Questo sistema utilizza componenti open source tra cui QLC+ distribuito secondo licenza Apache 2.0."));
    connect(m_websiteLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotWebsiteClicked()));
}

AboutBox::~AboutBox()
{
}

void AboutBox::slotWebsiteClicked()
{
    QDesktopServices::openUrl(QUrl("https://www.aicontroller.it/"));
}
