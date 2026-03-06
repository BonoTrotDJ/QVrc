/*
  Q Light Controller Plus
  decryptworkspacedialog.cpp

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

#include "decryptworkspacedialog.h"
#include "ui_decryptworkspacedialog.h"
#include "../../qmlui/tardis/simplecrypt.h"

#include <QFileDialog>
#include <QFile>

DecryptWorkspaceDialog::DecryptWorkspaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DecryptWorkspaceDialog)
{
    ui->setupUi(this);
    connect(ui->m_browseButton, SIGNAL(clicked()), this, SLOT(slotBrowseSource()));
    connect(ui->m_decryptButton, SIGNAL(clicked()), this, SLOT(slotDecryptAndSave()));
}

DecryptWorkspaceDialog::~DecryptWorkspaceDialog()
{
    delete ui;
}

void DecryptWorkspaceDialog::slotBrowseSource()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Encrypted Workspace"),
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

void DecryptWorkspaceDialog::slotDecryptAndSave()
{
    if (m_sourceFile.isEmpty())
    {
        ui->m_statusLabel->setText(tr("Nessun file selezionato."));
        return;
    }

    QFile srcFile(m_sourceFile);
    if (!srcFile.open(QIODevice::ReadOnly))
    {
        ui->m_statusLabel->setText(tr("Impossibile aprire il file sorgente."));
        return;
    }
    QByteArray data = srcFile.readAll();
    srcFile.close();

    QString destFile = QFileDialog::getSaveFileName(
        this,
        tr("Salva Workspace Decifrato"),
        QString(),
        tr("QLC+ Workspace (*.qxw)")
    );

    if (destFile.isEmpty())
        return;

    if (!destFile.endsWith(".qxw", Qt::CaseInsensitive))
        destFile += ".qxw";

    SimpleCrypt crypto(Q_UINT64_C(0x6C74697665727365));
    QByteArray decrypted = crypto.decryptToByteArray(data);

    if (crypto.lastError() != SimpleCrypt::ErrorNoError)
    {
        ui->m_statusLabel->setText(tr("Errore di decifratura. File non valido o chiave errata."));
        return;
    }

    QFile dstFile(destFile);
    if (!dstFile.open(QIODevice::WriteOnly))
    {
        ui->m_statusLabel->setText(tr("Impossibile scrivere il file di destinazione."));
        return;
    }
    dstFile.write(decrypted);
    dstFile.close();

    ui->m_statusLabel->setText(tr("File decifrato con successo: %1").arg(destFile));
}
