/*
  Q Light Controller Plus
  encryptworkspacedialog.cpp

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

#include "encryptworkspacedialog.h"
#include "ui_encryptworkspacedialog.h"
#include "../../qmlui/tardis/simplecrypt.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

EncryptWorkspaceDialog::EncryptWorkspaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EncryptWorkspaceDialog)
{
    ui->setupUi(this);
    connect(ui->m_browseButton, SIGNAL(clicked()), this, SLOT(slotBrowseSource()));
    connect(ui->m_encryptButton, SIGNAL(clicked()), this, SLOT(slotEncryptAndSave()));
}

EncryptWorkspaceDialog::~EncryptWorkspaceDialog()
{
    delete ui;
}

void EncryptWorkspaceDialog::slotBrowseSource()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Workspace"),
        QString(),
        tr("QLC+ Workspace (*.qxw)")
    );

    if (!fileName.isEmpty())
    {
        m_sourceFile = fileName;
        ui->m_sourceLabel->setText(fileName);
        ui->m_statusLabel->clear();
    }
}

void EncryptWorkspaceDialog::slotEncryptAndSave()
{
    if (m_sourceFile.isEmpty())
    {
        ui->m_statusLabel->setText(tr("No source file selected."));
        return;
    }

    QFile srcFile(m_sourceFile);
    if (!srcFile.open(QIODevice::ReadOnly))
    {
        ui->m_statusLabel->setText(tr("Cannot open source file."));
        return;
    }
    QByteArray data = srcFile.readAll();
    srcFile.close();

    QString destFile = QFileDialog::getSaveFileName(
        this,
        tr("Save Encrypted Workspace"),
        QString(),
        tr("Encrypted Workspace (*.igm)")
    );

    if (destFile.isEmpty())
        return;

    if (!destFile.endsWith(".igm", Qt::CaseInsensitive))
        destFile += ".igm";

    SimpleCrypt crypto(Q_UINT64_C(0x6C74697665727365));
    QByteArray encrypted = crypto.encryptToByteArray(data);

    QFile dstFile(destFile);
    if (!dstFile.open(QIODevice::WriteOnly))
    {
        ui->m_statusLabel->setText(tr("Cannot write destination file."));
        return;
    }
    dstFile.write(encrypted);
    dstFile.close();

    ui->m_statusLabel->setText(tr("File encrypted successfully: %1").arg(destFile));
}
