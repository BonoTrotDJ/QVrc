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
#include <QTimer>
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
    m_websiteLabel->setText(tr("Website: %1").arg("<A HREF=\"https://www.imedia.it/\">https://www.imedia.it/</A>"));
    connect(m_websiteLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotWebsiteClicked()));

    connect(m_contributors, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(slotItemClicked()));
    m_contributors->clear();
    m_contributors->addItem("Imedia Group - Multiverse Controller");

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
    m_row = -1;
    m_increment = 1;
    m_timer->start(500);
}

AboutBox::~AboutBox()
{
}

void AboutBox::slotTimeout()
{
    if (m_row <= 0)
        m_increment = 1;
    else if (m_row >= m_contributors->count())
        m_increment = -1;

    m_row += m_increment;
    m_contributors->scrollToItem(m_contributors->item(m_row));
}

void AboutBox::slotItemClicked()
{
    if (m_timer != NULL)
    {
        m_timer->stop();
        delete m_timer;
        m_timer = NULL;
    }
}

void AboutBox::slotWebsiteClicked()
{
    QDesktopServices::openUrl(QUrl("https://www.imedia.it/"));
}
