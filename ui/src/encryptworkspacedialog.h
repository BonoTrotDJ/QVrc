/*
  Q Light Controller Plus
  encryptworkspacedialog.h

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

#ifndef ENCRYPTWORKSPACEDIALOG_H
#define ENCRYPTWORKSPACEDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class EncryptWorkspaceDialog;
}

class EncryptWorkspaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EncryptWorkspaceDialog(QWidget *parent = nullptr);
    ~EncryptWorkspaceDialog();

public slots:
    void slotBrowseSource();
    void slotEncryptAndSave();

private:
    Ui::EncryptWorkspaceDialog *ui;
    QString m_sourceFile;
};

#endif // ENCRYPTWORKSPACEDIALOG_H
