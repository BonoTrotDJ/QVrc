/*
  Q Light Controller
  aboutbox_test.cpp

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

#include <QtTest>

#include "qlcconfig.h"
#include "aboutbox_test.h"
#define private public
#include "aboutbox.h"
#undef protected

void AboutBox_Test::initial()
{
    QWidget w;
    AboutBox ab(&w);
    QCOMPARE(ab.parentWidget(), &w);

    QVERIFY(ab.m_titleLabel != NULL);
    QCOMPARE(ab.m_titleLabel->text(), QString(APPNAME));

    QVERIFY(ab.m_versionLabel != NULL);
    QCOMPARE(ab.m_versionLabel->text(), QString(APPVERSION));

    QVERIFY(ab.m_copyrightLabel != NULL);
    QVERIFY(ab.m_copyrightLabel->text().contains("Imedia Group - Multiverse Controller"));

    QVERIFY(ab.m_websiteLabel != NULL);
    QVERIFY(ab.m_websiteLabel->text().contains("<A HREF=\"https://www.aicontroller.it/\">https://www.aicontroller.it/</A>"));

    QVERIFY(ab.m_licenseLabel != NULL);
    QVERIFY(ab.m_licenseLabel->text().contains("Questo sistema utilizza componenti open source"));
    QVERIFY(ab.m_licenseLabel->text().contains("Apache 2.0"));
}

QTEST_MAIN(AboutBox_Test)
